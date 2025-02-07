/*
 * @Descripttion:
 * @version:
 * @Author: Haochen Ye
 * @Date: 2021-08-24 20:12:49
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2021-12-02 12:27:36
 */
#include <iostream>
#include <string>
#include <vector>
#include "vfnet/vfnet.hpp"
#include <memory>
#include "IEngine.hpp"
#include <string.h>
#include <algorithm>
#include <cmath>

using namespace std;
namespace nce_alg {
typedef struct pos_score
{
    int   index;
    float score;
} pos_score;

vfnet_priv::vfnet_priv()
{
    alg_cfg.threshold  = 0.3;
    alg_cfg.isLog      = false;
    input_tensor_infos = nullptr;

    num_anchors      = 3;
    num_cls          = 1;
    model_image_info = { 0 };
}

vfnet_priv::~vfnet_priv()
{
    printf("vfnet deinit");
}


NCE_S32 vfnet::alg_init(vector<input_tensor_info> &            st_tensor_infos,
                        LinkedHashMap<string, tmp_map_result> &st_result_map,
                        YAML::Node &                           config)
{
    NCE_S32 ret = NCE_FAILED;
    pPriv       = shared_ptr<vfnet_priv>(new vfnet_priv());
    int i       = 0;

    const auto names = config["output_names"];
    for (auto &iter : names)
    {
        st_result_map.insert(make_pair(iter.as<string>(), tmp_map_result{ 0 }));
    }

    input_tensor_info input0{ 0 };
    input0.order     = RGB;
    const auto mean0 = config["mean0"];
    const auto std0  = config["std0"];
    for (i = 0; i < 3; i++)
    {
        input0.mean[i] = mean0[i].as<float>();
        input0.std[i]  = std0[i].as<float>();
    }
    st_tensor_infos.push_back(input0);

    pPriv->num_anchors        = config["num_anchors"].as<int>();
    pPriv->num_cls            = config["num_cls"].as<int>();
    pPriv->alg_cfg.threshold  = config["conf_thresh"].as<float>();
    pPriv->input_tensor_infos = &st_tensor_infos;
    printf("finshed alg init\n!");
    return ret;
}

NCE_S32 vfnet::alg_cfg_set(const task_config_info &st_task_config_info)
{
    NCE_S32 ret = NCE_FAILED;

    pPriv->alg_cfg.threshold                   = st_task_config_info.threshold;
    pPriv->alg_cfg.st_cfg.hd_config.nms_thresh = st_task_config_info.st_cfg.bd_config.nms_thresh;
    pPriv->alg_cfg.isLog                       = st_task_config_info.isLog;
    pPriv->num_anchors                         = st_task_config_info.st_cfg.bd_config.num_anchors;
    pPriv->num_cls                             = st_task_config_info.st_cfg.bd_config.num_cls;
    ret                                        = NCE_SUCCESS;

    printf("alg score thresh: %f\n", pPriv->alg_cfg.threshold);
    printf("alg nms thresh: %f\n", pPriv->alg_cfg.st_cfg.hd_config.nms_thresh);
    printf("alg num anchor: %d\n", pPriv->num_anchors);
    printf("alg num cls: %d\n", pPriv->num_cls);
    return ret;
}

NCE_S32 vfnet::alg_inference(vector<img_t> &pc_img)
{
    NCE_S32 ret = NCE_FAILED;
    ret         = NCE_SUCCESS;
    printf("finish alg_inference\n");
    return ret;
}

NCE_S32 vfnet::alg_get_result(alg_result_info &results, LinkedHashMap<string, tmp_map_result> &st_result_map)
{
    results.num = 0;
    pPriv->detect_results.clear();
    pPriv->tmp_result.clear();

    NCE_S32 ret = NCE_FAILED;
    if (NULL == pPriv)
    {
        printf("Failed to init pPriv of alg");
        return NCE_FAILED;
    }

    static vector<tmp_map_result> all_logits = {
        st_result_map["P3_logits"], st_result_map["P4_logits"], st_result_map["P5_logits"],
        st_result_map["P6_logits"], st_result_map["P7_logits"],
    };

    static vector<tmp_map_result> all_reg = {
        st_result_map["P3_bbox_reg"], st_result_map["P4_bbox_reg"], st_result_map["P5_bbox_reg"],
        st_result_map["P6_bbox_reg"], st_result_map["P7_bbox_reg"],
    };

    for (int i = 0; i < 5; i++)
    {

        NCE_F32 *logits = (NCE_F32 *)all_logits[i].pu32Feat;
        NCE_F32 *bboxes = (NCE_F32 *)all_reg[i].pu32Feat;

        auto feat_width  = all_logits[i].tensor.u32FeatWidth;
        auto feat_height = all_logits[i].tensor.u32FeatHeight;

        auto logist_height_stride  = all_logits[i].tensor.height_stride;
        auto logist_width_stride   = all_logits[i].tensor.width_stride;
        auto logist_channel_stride = all_logits[i].tensor.channel_stride;

        auto reg_height_stride  = all_reg[i].tensor.height_stride;
        auto reg_width_stride   = all_reg[i].tensor.width_stride;
        auto reg_channel_stride = all_reg[i].tensor.channel_stride;

        NCE_S32 feature_size = feat_width * feat_height;
        NCE_S32 cur_stride   = pow(2, i + 3);

        for (NCE_S32 h = 0; h < feat_height; h++)
        {
            for (NCE_S32 w = 0; w < feat_width; w++)
            {
                for (NCE_S32 anchor_index = 0; anchor_index < pPriv->num_anchors; anchor_index++)
                {
                    NCE_U32 logits_index = h * logist_height_stride + w * logist_width_stride
                                           + anchor_index * logist_channel_stride * pPriv->num_cls;
                    NCE_U32 reg_index =
                        h * reg_height_stride + w * reg_width_stride + anchor_index * logist_channel_stride * 4;

                    NCE_F32 score = logits[logits_index];
                    if (score < 0)
                    {
                        continue;
                    };
                    score = sqrt(score);
                    if (score < pPriv->alg_cfg.threshold)
                        continue;

                    NCE_F32 left   = bboxes[reg_index + 0 * reg_channel_stride];
                    NCE_F32 top    = bboxes[reg_index + 1 * reg_channel_stride];
                    NCE_F32 right  = bboxes[reg_index + 2 * reg_channel_stride];
                    NCE_F32 bottom = bboxes[reg_index + 3 * reg_channel_stride];

                    NCE_U32 x = w * cur_stride;
                    NCE_U32 y = h * cur_stride;

                    NCE_U32 x1 = std::max((NCE_F32)0, x - left);
                    NCE_U32 y1 = std::max((NCE_F32)0, y - top);
                    NCE_U32 x2 = std::min((NCE_F32)(*pPriv->input_tensor_infos)[0].width - 1, x + right);
                    NCE_U32 y2 = std::min((NCE_F32)(*pPriv->input_tensor_infos)[0].height - 1, y + bottom);

                    pPriv->detect_results.push_back(detect_result{ x1, y1, x2, y2, score });
                }
            }
        }
    }
    // body_nms(pPriv->detect_results, pPriv->detect_results, pPriv->alg_cfg.st_cfg.hd_config.nms_thresh);
    for (auto &item : pPriv->detect_results)
    {
        pPriv->tmp_result.push_back(alg_result{ VFNET, &item });
    }
    NCE_U32 num = pPriv->detect_results.size();
    results.num = num;
    if (num > 0)
        results.st_alg_results = &pPriv->tmp_result[0];

    return ret;
}
NCE_S32 vfnet::alg_destroy()
{
    NCE_S32 ret = NCE_FAILED;
    ret         = NCE_SUCCESS;
    return ret;
}

} // namespace nce_alg

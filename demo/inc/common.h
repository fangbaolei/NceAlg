#ifndef _COMMON_H_
#define _COMMON_H_
#include "alg_type.h"

NCE_S32 nce_read_img(const char *img_path, img_t &input_img);

NCE_S32 nce_write_img(const char *img_path, img_t &input_img);

NCE_S32 nce_free_img(img_t &input_img);

NCE_S32 nce_draw_bbox(img_t &input_img, Bbox box, NCE_S32 line_width, NCE_S32 color[3]);

NCE_S32 nce_trans(img_t &input_img, ImageProcessParam param);

class nce_base_process
{
public:
    nce_base_process()
    {}
    // nce_base_process* create_instance(ImageProcessParam param){ return nullptr;}
    NCE_S32 virtual forward(img_t &input_img)
    {
        printf("g0udasldjasl\n");
        return NCE_FAILED;
    }
};

template <class P>
nce_base_process *create_aainstance(ImageProcessParam param)
{
    P *parent_ptr;
    parent_ptr = new P(param);
    printf("cnmmmmm!\n");
    return (nce_base_process *)parent_ptr;
}

class nce_planner2package : public nce_base_process
{
public:
    nce_planner2package(ImageProcessParam param);
    ~nce_planner2package();
    static nce_base_process  *create_instance(ImageProcessParam param)
    {
        nce_planner2package *parent_ptr;
        parent_ptr = new nce_planner2package(param);
        printf("cnmmmmm!\n");
        return (nce_base_process *)parent_ptr;
    }
    NCE_S32 forward(img_t &input_img);

private:
    NCE_U8 *tmp_buffer;
};

class nce_package2planner : public nce_base_process
{
public:
    nce_package2planner(ImageProcessParam param);
    static nce_base_process  *create_instance(ImageProcessParam param)
    {
        nce_package2planner *parent_ptr;
        parent_ptr = new nce_package2planner(param);
        printf("cnmmmmm!\n");
        return (nce_base_process *)parent_ptr;
    }
    NCE_S32 forward(img_t &input_img);
    ~nce_package2planner();

private:
    NCE_U8 *tmp_buffer;
};

class nce_gray2bgr : public nce_base_process
{
public:
    nce_gray2bgr(ImageProcessParam param);
    NCE_S32 forward(img_t &input_img);
};
class nce_normalization : public nce_base_process
{
public:
    nce_normalization(ImageProcessParam param);
    static nce_base_process  *create_instance(ImageProcessParam param)
    {
        nce_normalization *parent_ptr;
        parent_ptr = new nce_normalization(param);
        printf("cnmmmmm!\n");
        return (nce_base_process *)parent_ptr;
    }
    NCE_S32 forward(img_t &input_img);

private:
    NCE_F32 mean[3];
    NCE_F32 var[3];
};



class nce_resize : public nce_base_process
{
public:
    nce_resize(ImageProcessParam param);
    static nce_base_process  *create_instance(ImageProcessParam param)
    {
        nce_resize *parent_ptr;
        parent_ptr = new nce_resize(param);
        printf("cnmmmmm!\n");
        return (nce_base_process *)parent_ptr;
    }
    NCE_S32 forward(img_t &input_img);
    ~nce_resize();

private:
    img_t output_img;
};

#endif
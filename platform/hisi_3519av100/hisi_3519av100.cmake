INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/platform/hisi_3519av100/MNN/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/platform/hisi_3519av100/opencv/include/opencv4/)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/alg/engine_manager/engine_interface/MNN/ ENGINE_3519av100_SRC)

#设置hisi3516dv300所需源文件
set(PLATFORM_SRC 
${ENGINE_3519av100_SRC}
) 


link_directories("${PROJECT_SOURCE_DIR}/platform/hisi_3519av100/opencv/lib/")
link_directories("${PROJECT_SOURCE_DIR}/platform/hisi_3519av100/MNN/lib/")


#设置hisi3516dv300所需库文件
set(PLATFORM_LIB
-lgomp
-lMNN
-ldl
-lpthread
-lm
-lrt
)

# link_directories("/home/video/user/yehc/sdk/hisi/arm-himix200-linux/arm-himix200-linux/lib")
# link_directories("/home/video/user/yehc/sdk/hisi/arm-himix200-linux/arm-himix200-linux/bin")
# SET(CMAKE_SYSROOT "/home/video/user/yehc/sdk/hisi/arm-himix200-linux/")




if(OPENCVOPTION MATCHES "ON")
SET(OPENCV_LIB
-lopencv_core
-lopencv_highgui
-lopencv_imgproc
-lopencv_imgcodecs)
elseif(OPENCVOPTION MATCHES "OFF")
SET(OPENCV_LIB "")
endif()


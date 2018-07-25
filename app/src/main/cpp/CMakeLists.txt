
cmake_minimum_required(VERSION 3.4.1)

set(PATH_TO_AV_ENAGINE ${PATH_TO_MEDIACORE}/src/main/cpp)
set(PATH_TO_THIRDPARTY ${PATH_TO_AV_ENAGINE}/thirdparty)
set(PATH_TO_PRE_BUILT ${PATH_TO_THIRDPARTY}/prebuilt/${ANDROID_ABI})

message(${PATH_TO_AV_ENAGINE})
message(${ANDROID_ABI})

add_definitions("
    -DGL_GLEXT_PROTOTYPES
    -DEGL_EGLEXT_PROTOTYPES
")

include_directories(BEFORE ${PATH_TO_THIRDPARTY}/ffmpeg/include)
include_directories(${PATH_TO_AV_ENAGINE}/)
include_directories(${PATH_TO_AV_ENAGINE}/video_player/)
include_directories(${PATH_TO_AV_ENAGINE}/common/)

file(GLOB FILES_LIB_COMMON "${PATH_TO_AV_ENAGINE}/common/*.cpp")
file(GLOB FILES_LIB_COMMON_EGL_CORE "${PATH_TO_AV_ENAGINE}/common/egl_core/*.cpp")
file(GLOB FILES_LIB_COMMON_MESSAGE_Q "${PATH_TO_AV_ENAGINE}/common/message_queue/*.cpp")
file(GLOB FILES_LIB_COMMON_OPENSL_MEDIA "${PATH_TO_AV_ENAGINE}/common/opensl_media/*.cpp")
file(GLOB FILES_LIB_COMMON_OPENGL_MEDIA "${PATH_TO_AV_ENAGINE}/common/opengl_media/*.cpp")

file(GLOB FILES_LIB_COMMON_OPENGL_MEDIA_TEXTRUE_COPY
    "${PATH_TO_AV_ENAGINE}/common/opengl_media/texture_copier/*.cpp")
file(GLOB FILES_LIB_COMMON_OPENGL_MEDIA_TEXTRUE
    "${PATH_TO_AV_ENAGINE}/common/opengl_media/texture/*.cpp")

file(GLOB FILES_LIB_VIDEO_PLAYER "${PATH_TO_AV_ENGINE}/video_player/*.cpp")


add_library(av-engine STATIC
            ${FILES_LIB_COMMON}
            ${FILES_LIB_COMMON_EGL_CORE}
            ${FILES_LIB_COMMON_MESSAGE_Q}
            ${FILES_LIB_COMMON_OPENGL_MEDIA}
            ${FILES_LIB_COMMON_OPENSL_MEDIA}
            ${FILES_LIB_COMMON_OPENGL_MEDIA_TEXTRUE_COPY}
            ${FILES_LIB_COMMON_OPENGL_MEDIA_TEXTRUE}
            )




target_link_libraries(av-engine
                       # 引入系统的动态库
                       log
                       android
                       GLESv2
                       EGL
                       z
                       OpenSLES
                       # 引入ffmpeg相关静态库
                       ${PATH_TO_PRE_BUILT}/libavfilter.a
                       ${PATH_TO_PRE_BUILT}/libavformat.a
                       ${PATH_TO_PRE_BUILT}/libavcodec.a
                       ${PATH_TO_PRE_BUILT}/libpostproc.a
                       ${PATH_TO_PRE_BUILT}/libswresample.a
                       ${PATH_TO_PRE_BUILT}/libswscale.a
                       ${PATH_TO_PRE_BUILT}/libavutil.a
                       ${PATH_TO_PRE_BUILT}/libpostproc.a
                       ${PATH_TO_PRE_BUILT}/libfdk-aac.a
                       ${PATH_TO_PRE_BUILT}/libvo-aacenc.a
                       ${PATH_TO_PRE_BUILT}/libx264.a
                       )
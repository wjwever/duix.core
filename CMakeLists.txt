cmake_minimum_required(VERSION 3.27)
project(duix)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(Git REQUIRED)
find_package(CURL REQUIRED)
option(EXTRACT_ALL_FRAMES "extract all frames or not" OFF)

execute_process (
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    OUTPUT_VARIABLE GIT_COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Git commit ID:" ${GIT_COMMIT_ID})


add_definitions(-DCOMMIT_ID="${GIT_COMMIT_ID}")

# 查找 OpenCV 包
#set(OpenCV_DIR ...)
find_package(OpenCV REQUIRED)

# 打印 OpenCV 版本信息
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


#set(CMAKE_CXX_FLAGS "{CMAKE_CXX_FLAGS} -DSTB_IMAGE_IMPLEMENTATION")


# find_package(JPEG REQUIRED)
# include_directories(${JPEG_INCLUDE_DIRS})
# message(STATUS "JPEG_INCLUDE_DIRS=${JPEG_INCLUDE_DIRS}")
# message(STATUS "JPEG_LIBRARIES=${JPEG_LIBRARIES}")


find_package(OpenMP REQUIRED)




set(FFMPEG /usr/local/ffmpeg/)
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/base
    ${CMAKE_SOURCE_DIR}/ext
    ${CMAKE_SOURCE_DIR}/aisdk
    ${CMAKE_SOURCE_DIR}/third/json/include
    ${CMAKE_SOURCE_DIR}/aes
    ${OpenCV_INCLUDE_DIRS}
    ${FFMPEG}/include

    ${CMAKE_SOURCE_DIR}/third/ncnn/include/ncnn
    ${CMAKE_SOURCE_DIR}/third/onnxruntime/include
    ${CMAKE_SOURCE_DIR}/third/x86/include/turbojpeg

)

link_directories(
    ${FFMPEG}/lib
    ${CMAKE_SOURCE_DIR}/third/ncnn/lib
    ${CMAKE_SOURCE_DIR}/third/onnxruntime/lib
)

file(GLOB AES_SRC ${CMAKE_SOURCE_DIR}/aes/*.c)
file(GLOB DIGIT_CPP ${CMAKE_SOURCE_DIR}/digit/*.cpp)
file(GLOB DIGIT_C ${CMAKE_SOURCE_DIR}/digit/*.c)

file(GLOB BASE_C ${CMAKE_SOURCE_DIR}/base/*.c)
file(GLOB BASE_CPP ${CMAKE_SOURCE_DIR}/base/*.cpp)
file(GLOB AISDK_CPP ${CMAKE_SOURCE_DIR}/aisdk/*.cpp)
file(GLOB EXT_CPP ${CMAKE_SOURCE_DIR}/ext/*.cpp)


if (EXTRACT_ALL_FRAMES) 
    add_definitions(-DEXTRACT_ALL_FRAMES)
endif() 

add_executable(main ${EXT_CPP} ${AES_SRC} ${DIGIT_C} ${DIGIT_CPP} ${BASE_CPP} ${AISDK_CPP} base/cJSON.c base/dh_mem.c)


target_link_libraries(main
    avformat
    avcodec
    avutil
    swscale
    CURL::libcurl
    onnxruntime
    ncnn
    OpenMP::OpenMP_CXX
    ${CMAKE_SOURCE_DIR}/third/x86/lib/libturbojpeg.a
    ${OpenCV_LIBS}
)

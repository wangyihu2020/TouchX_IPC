cmake_minimum_required(VERSION 3.12)
project(Device)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES Device.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} HD)
#target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)



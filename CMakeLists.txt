cmake_minimum_required(VERSION 3.10)

set(SERVICE_NAME main)

project(${SERVICE_NAME})

set (CPP_SERVICE_MAJOR_VERSION 0)
set (CPP_SERVICE_MINOR_VERSION 8)
set (CPP_SERVICE_PATCH_VERSION 1)
set (CPP_SERVICE_VERSION ${CPP_SERVICE_MAJOR_VERSION}.${CPP_SERVICE_MINOR_VERSION}.${CPP_SERVICE_PATCH_VERSION})

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/package")

######################################################################
#全局编译标志
######################################################################

SET (CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,$ORIGIN/../lib" )

SET (CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,$ORIGIN/../lib" )

set(CMAKE_SKIP_BUILD_RPATH TRUE)

set(BUILD_SHARED_LIBS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")

######################################################################
#全局链接和包含
######################################################################

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB TAlASR_LIB ${PROJECT_SOURCE_DIR}/lib/lib*.*)

# find_package(Boost REQUIRED COMPONENTS filesystem log system)

#################################################################L#####
#全局编译输出指定
######################################################################

IF(CMAKE_BUILD_TYPE MATCHES Debug)
set(CMAKE_VERBOSE_MAKEFILE ON)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/debug/lib/${BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/debug/lib/${BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/debug/bin/${BUILD_TYPE})

######################################################################
#子工程目录
######################################################################

add_subdirectory(chrome-base)


######################################################################
#算法sdk编译对象
######################################################################

######################################################################
#http service编译对象
######################################################################

add_executable(main
    main.cpp)
target_link_libraries(main
    base
    #fst
    pthread
    ${TAlASR_LIB}
    )

######################################################################
#websocket service编译对象
######################################################################

######################################################################
#程序发布设置
######################################################################

install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ DESTINATION bin)
install(DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/ DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/kafka/cppkafka/lib/ DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/alg/resource/ DESTINATION bin/res)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/alg/lib/ DESTINATION lib)
#install(DIRECTORY config/ DESTINATION bin/config)
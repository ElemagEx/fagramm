cmake_minimum_required(VERSION 3.14)

project(fagramm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)

option(FAGRAMM_DEVELOPMENT "fagramm: Current Development" ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/EHsc /Wall /permissive- /FInowarns.h")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-pthread -Weverything -include\"nowarns.h\"")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
else()
    set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -include\"nowarns.h\"")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

message(STATUS "CMAKE_CXX_FLAGS         = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG   = ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")

set(fagramm_include ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(fagramm_inc_path ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(fagramm_src_path ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(
    fagramm
    STATIC
    ${fagramm_inc_path}/fagramm.h
    ${fagramm_src_path}/fagramm.cpp
)
target_include_directories(fagramm PUBLIC ${fagramm_include} "msvs")

if(FAGRAMM_DEVELOPMENT)
    add_executable(
        main
        msvs/main.cpp
    )
    target_link_libraries(main fagramm)
endif()

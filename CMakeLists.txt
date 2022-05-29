cmake_minimum_required(VERSION 3.8)

set(fagramm_standard 11)

project(fagramm)

option(FAGRAMM_DEVELOPMENT "fagramm: Current Development" ON)

set(fagramm_include ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(fagramm_inc_path ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(fagramm_src_path ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(
    fagramm
    STATIC
    ${fagramm_inc_path}/fagramm.h
    ${fagramm_src_path}/fagramm.cpp
)
target_include_directories(fagramm PUBLIC ${fagramm_include})

if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET fagramm PROPERTY CXX_STANDARD ${fagramm_standard})
endif()

if(FAGRAMM_DEVELOPMENT)
    add_executable(
        main
        msvs/main.cpp
    )
    target_link_libraries(main fagramm)

    if(CMAKE_VERSION VERSION_GREATER 3.12)
        set_property(TARGET main PROPERTY CXX_STANDARD ${fagramm_standard})
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /JMC /permissive-")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

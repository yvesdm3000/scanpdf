cmake_minimum_required(VERSION 3.16)

project(scanpdf)

# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++17)

link_directories(/usr/local/lib/)
include_directories(/usr/include/hpdf)

file(GLOB SCANPDF_SOURCES "src/scanpdf.cpp")

#add_library(libscanpdf ${SCANPDF_SOURCES})
#target_include_directories(libscanpdf PUBLIC include)

add_executable(scanpdf ${SCANPDF_SOURCES})

target_link_libraries(scanpdf PUBLIC libpdfgen sanepp hpdf png sane)

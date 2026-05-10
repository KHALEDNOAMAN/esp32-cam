# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)
add_custom_target(
        bump_version ALL
        COMMAND python ${CMAKE_SOURCE_DIR}/scripts/bump_version.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_dependencies(${COMPONENT_LIB} bump_version)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(esp_cam C CXX)
include(GNUInstallDirs)

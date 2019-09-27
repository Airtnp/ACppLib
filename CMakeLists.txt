cmake_minimum_required (VERSION 3.9)
project (ACppLib VERSION 0.95 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 17)

# configure_file()

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.hpp")
add_library(ACppLib SHARED
        ${SOURCES}
)
TARGET_LINK_LIBRARIES(ACppLib)
set_target_properties(ACppLib PROPERTIES LINKER_LANGUAGE CXX)

set(TESTS test/sn_test.cpp src/sn_DB.hpp)
add_executable(ACppTest ${TESTS})
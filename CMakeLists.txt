cmake_minimum_required (VERSION 3.9)
project (ACppLib VERSION 0.95 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 17)
set (CXX_EXTRA_FLAGS "-fconcepts") # G++ 9.2 in MSYS2 cannot support -fcoroutines...

add_definitions(${CXX_EXTRA_FLAGS})

# configure_file()

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.hpp")
add_library(ACppLib SHARED
        ${SOURCES}
)
target_link_libraries(ACppLib)
set_target_properties(ACppLib PROPERTIES LINKER_LANGUAGE CXX)

set(TESTS test/sn_test.cpp src/sn_DB.hpp src/sn_Concept.hpp)
add_executable(ACppTest ${TESTS})
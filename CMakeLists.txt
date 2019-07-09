cmake_minimum_required(VERSION 3.12)
project(TermUtils VERSION 2019.6 LANGUAGES C)

include_directories(PUBLIC include)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

# install(DIRECTORY include/term DESTINATION include)

if (${CMAKE_C_COMPILER_ID} STREQUAL Clang OR ${CMAKE_C_COMPILER_ID} STREQUAL AppleClang)
    add_compile_options (-fcolor-diagnostics)
endif()

add_subdirectory(src)

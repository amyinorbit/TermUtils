add_library(termutils STATIC
    arg_parsing.c
    arg_printing.c
    arg_utils.c
    colors.c
    printing.c
    line.c
    editor.c
    hexes.c
    string_buf.c
)
target_include_directories(termutils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS termutils ARCHIVE DESTINATION lib)

add_executable(demo main.c)
target_link_libraries(demo PRIVATE termutils)

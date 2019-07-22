add_library(TermUtils STATIC arg_parsing.c arg_printing.c arg_utils.c colors.c printing.c repl.c)
target_include_directories(TermUtils PUBLIC ${PROJECT_SOURCE_DIR}/include)
install(TARGETS TermUtils ARCHIVE)

add_executable(demo main.c)
target_link_libraries(demo PRIVATE TermUtils)

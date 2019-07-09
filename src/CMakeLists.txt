add_library(termutils STATIC arg_parsing.c arg_printing.c arg_utils.c)
target_include_directories(termutils PUBLIC ${PROJECT_SOURCE_DIR}/include)
install(TARGETS termutils ARCHIVE)

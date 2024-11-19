aux_source_directory(. DIR_LIB_SRCS)
add_library(codecs ${DIR_LIB_SRCS})

target_link_libraries(codecs base)
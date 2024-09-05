aux_source_directory(. DIR_net_SRCS)
add_library(net ${DIR_net_SRCS})
target_link_libraries(net base)
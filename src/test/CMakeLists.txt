aux_source_directory(. TESTSRC)
add_executable(test ${TESTSRC})
target_link_libraries(test base net)
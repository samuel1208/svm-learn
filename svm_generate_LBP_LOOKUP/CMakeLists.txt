
include_directories(./inc)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
aux_source_directory(src src_files)

SET(target_name "svm_generate_LBP_LOOKUP") 
add_executable(${target_name}  ${src_files} )
target_link_libraries(${target_name})
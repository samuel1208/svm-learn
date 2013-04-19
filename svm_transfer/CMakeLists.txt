cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)


include_directories(./inc)
aux_source_directory(src src_files)

SET(target_name "svm_transfer") 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable(${target_name}  ${src_files} )
target_link_libraries(${target_name})
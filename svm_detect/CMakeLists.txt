
include_directories(./inc)

aux_source_directory(src src_files)

SET(target_name "svm_detector") 
add_executable(${target_name}  ${src_files} )
target_link_libraries(${target_name} ${OpenCV_LIBS} svm_common)
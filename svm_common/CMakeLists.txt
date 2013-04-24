
include_directories(./inc)

aux_source_directory(src src_files)

SET(target_name "svm_common") 
add_LIBRARY(${target_name}  STATIC ${src_files} )
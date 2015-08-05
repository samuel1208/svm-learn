SVM Learn    
===    

1. 使用`svm_extract_feature`进行特征提取    
2. 将提取好的特征文件放到libsvm中进行训练， 训练细节见libsvm官方    
3. 使用`svm_transfer`将训练好的模型文件放到`svm_common/src`中，如`svm_common/src/svm_constant_gender.cpp`    

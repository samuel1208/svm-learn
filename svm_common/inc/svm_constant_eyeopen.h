#ifndef  __SVM_CONSTANT_EYEOPEN_H__
#define  __SVM_CONSTANT_EYEOPEN_H__

#include "svm.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int i32MinRange_eyeopen;
extern int i32MaxRange_eyeopen;
extern int pMinMaxFeaVal_eyeopen[];
extern int svm_type_eyeopen;
extern int kernel_type_eyeopen;
extern double gamma_eyeopen;
extern int nr_class_eyeopen;
extern int total_sv_eyeopen;
extern double pRho_eyeopen[];
extern int pLabel_eyeopen[];
extern int pNr_sv_eyeopen[];
extern int degree_eyeopen;
extern double coef0_eyeopen;
extern double *pProbA_eyeopen;
extern double *pProbB_eyeopen;
extern double pCoef_eyeopen[];
extern svm_node pSvm_node_eyeopen[];
extern int free_sv_eyeopen;

#ifdef __cplusplus
}  
#endif

#endif

#ifndef  __SVM_CONSTANT_EYECLOSED_H__
#define  __SVM_CONSTANT_EYECLOSED_H__

#include "svm.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int i32MinRange_eyeclosed;
extern int i32MaxRange_eyeclosed;
extern int pMinMaxFeaVal_eyeclosed[];
extern int svm_type_eyeclosed;
extern int kernel_type_eyeclosed;
extern double gamma_eyeclosed;
extern int nr_class_eyeclosed;
extern int total_sv_eyeclosed;
extern double pRho_eyeclosed[];
extern int pLabel_eyeclosed[];
extern int pNr_sv_eyeclosed[];
extern int degree_eyeclosed;
extern double coef0_eyeclosed;
extern double *pProbA_eyeclosed;
extern double *pProbB_eyeclosed;
extern double pCoef_eyeclosed[];
extern svm_node pSvm_node_eyeclosed[];
extern int free_sv_eyeclosed;

#ifdef __cplusplus
}  
#endif

#endif

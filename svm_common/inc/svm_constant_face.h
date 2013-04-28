#ifndef  __SVM_CONSTANT_FACE_H__
#define  __SVM_CONSTANT_FACE_H__

#include "svm.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int i32MinRange_face;
extern int i32MaxRange_face;
extern int pMinMaxFeaVal_face[];
extern int svm_type_face;
extern int kernel_type_face;
extern double gamma_face;
extern int nr_class_face;
extern int total_sv_face;
extern double pRho_face[];
extern int pLabel_face[];
extern int pNr_sv_face[];
extern int degree_face;
extern double coef0_face;
extern double *pProbA_face;
extern double *pProbB_face;
extern double pCoef_face[];
extern svm_node pSvm_node_face[];
extern int free_sv_face;

#ifdef __cplusplus
}  
#endif

#endif

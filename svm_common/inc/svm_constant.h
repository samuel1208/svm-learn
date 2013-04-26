#ifndef  __SVM_TRAIN_MODEL_H__
#define  __SVM_TRAIN_MODEL_H__

#include "svm.h"
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

extern int i32MinRange_smile;
extern int i32MaxRange_smile;
extern int pMinMaxFeaVal_smile[];
extern int svm_type_smile;
extern int kernel_type_smile;
extern double gamma_smile;
extern int nr_class_smile;
extern int total_sv_smile;
extern double pRho_smile[];
extern int pLabel_smile[];
extern int pNr_sv_smile[];
extern int degree_smile;
extern double coef0_smile;
extern double *pProbA_smile;
extern double *pProbB_smile;
extern double pCoef_smile[];
extern svm_node pSvm_node_smile[];
extern int free_sv_smile;

extern int i32MinRange_gesture;
extern int i32MaxRange_gesture;
extern int pMinMaxFeaVal_gesture[];
extern int svm_type_gesture;
extern int kernel_type_gesture;
extern double gamma_gesture;
extern int nr_class_gesture;
extern int total_sv_gesture;
extern double pRho_gesture[];
extern int pLabel_gesture[];
extern int pNr_sv_gesture[];
extern int degree_gesture;
extern double coef0_gesture;
extern double *pProbA_gesture;
extern double *pProbB_gesture;
extern double pCoef_gesture[];
extern svm_node pSvm_node_gesture[];
extern int free_sv_gesture;

#endif

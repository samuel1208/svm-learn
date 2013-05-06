#ifndef __SVM_H__
#define __SVM_H__

#include "tcomdef.h"
#ifdef __cplusplus
extern "C"{
#endif

typedef struct __tagSvm_node
{
	int index;
	int value;
}svm_node;

enum { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };	/* svm_type */
enum { LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED }; /* kernel_type */

typedef struct __tagSvm_parameter
{
    int kernel_type;
    int degree;
	int svm_type;
	double coef0;		
	double gamma;			
}svm_parameter;


typedef struct __tagSvm_model
{		
	int nr_class;		
	int l;		
    double *rho;
    double *probB;
    double *probA;
	svm_node **SV;		
	double **sv_coef;		
	int *label;	
	int *nSV;						
	int free_sv;	
    int *pMinMaxFeaVal;
    int feaLower;
    int feaUpper;	
    svm_parameter param;
}svm_model;

svm_model* Init_svm(THandle hMemBuf, const char *suffix);
void  Uninit_svm(THandle hMemBuf, svm_model ** ppModel);

/* 0 - nonface  1 face */
int   SvmPredict(THandle hMemBuf, svm_model *pSvmModel, int *pFea, int feaLength, int *label);


#ifdef __cplusplus
}
#endif
#endif

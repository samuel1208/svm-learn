#ifndef __SVM_DETECTOE_H__
#define __SVM_DETECTOE_H__


#include"tcomdef.h"
#include "svm.h"
#ifdef __cplusplus
extern "C" {
#endif

    int SVMDetector(THandle hMemBuf, svm_model *pSvmModel, TUInt8 *pBGR, int srcWidth,
                    int srcHeight, int srcWidthStep,TRECT region, int *label);


#ifdef __cplusplus
}
#endif
#endif

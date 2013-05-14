#ifndef __SVM_FEATURE_H__
#define __SVM_FEATURE_H__

#include "tcomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

    enum ASD_FEATURE_TYPE{
        FEAT_WAN_COLOR			=	0x00000001,
        FEAT_HOG	            =	0x00000002,	
        FEAT_LBP_8     			=	0x00000004,
        FEAT_LBP_16    			=	0x00000008
    };

    int svm_feature(THandle hMemBuf,TUInt8 *pBGR, int srcWidth, int srcHeight, 
                    int srcWidthStep, TRECT region, int *pFea, int feaUsed);




#ifdef __cplusplus
}
#endif

#endif 

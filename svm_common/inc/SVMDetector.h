#ifndef __SVM_DETECTOE_H__
#define __SVM_DETECTOE_H__


#include"tcomdef.h"
#ifdef __cplusplus
extern "C" {
#endif

    THandle SVMDetector_init(THandle hMemBuf, const char *name);

    void SVMDetector_uninit(THandle *hDetector);

    int SVMDetector_detect_ex(THandle hMemBuf, 
                              TUInt8 *pBGR, int srcWidth, int srcHeight,
                              int srcWidthStep, TRECT region, int *label,
                              int bIsExtended);

    int SVMDetector_detect(THandle hMemBuf, 
                           TUInt8 *pBGR, int srcWidth, int srcHeight,
                           int srcWidthStep, TRECT region, int *label);

#ifdef __cplusplus
}
#endif
#endif

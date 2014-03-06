#ifndef __SVM_DETECTOE_H__
#define __SVM_DETECTOE_H__


#include"tcomdef.h"
#ifdef __cplusplus
extern "C" {
#endif

    THandle SVMDetector_init(THandle hMemBuf, const char *name);

    void SVMDetector_uninit(THandle *hDetector);

    /*
      
     */
    typedef enum{
        FORMAT_BGR,
        FORMAT_GRAY
    }COLOR_FORMAT;
    
    int SVMDetector_detect(THandle hMemBuf, 
                           TUInt8 *pData, COLOR_FORMAT format,
                           int srcWidth, int srcHeight, int srcWidthStep, 
                           TRECT region, int *label);

#ifdef __cplusplus
}
#endif
#endif

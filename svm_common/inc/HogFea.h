#ifndef __HOG_FEATURE_H__
#define __HOG_FEATURE_H__

typedef enum __tagGRADIENT_MODE
{
    
    GRADIENT_SIMPLE = 0,
    GRADIENT_SOBEL = 1,
    GRADIENT_OTHER = 5
}GRADIENT_MODE;

#ifdef __cplusplus
extern "C" {
#endif
#include "tcomdef.h"

    int GetHOGDim(int width, int height);
    int GetGradient(const unsigned char* pGrayImg, int widthStep_src, 
                    int *pGradient_x, int *pGradient_y, int widthStep_dst, 
                    int width, int height, GRADIENT_MODE mode);

    /*return value : the number of HOG features*/
    int HogFea(THandle hMemBuf, unsigned char *grayImg, int widthStep,int width,
               int height, int *pHogFea);

#ifdef __cplusplus
}
#endif

#endif

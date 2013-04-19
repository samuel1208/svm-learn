#include "SVMDetector.h"
#include "tmem.h"
#include "WanHuaLinFea.h"
#include "HogFea.h"
#include "svm.h"

#define IMG_WIDTH  48
#define IMG_HEIGHT 48

#define HOG_DIM  (900)
#define WAN_DIM  (73)
#define SVM_FEA_DIM  (HOG_DIM+WAN_DIM)

int SVMDetector(THandle hMemBuf, svm_model *pSvmModel, TUInt8 *pBGR, int srcWidth, int srcHeight, int srcWidthStep,TRECT region, int *label)
{
    int rVal = 0;
    TUInt8 *pHSL = TNull, *pBGR_scale = TNull, *pGray = TNull;
    int dstWidth=0, dstHeight=0;
    int *pFea = TNull;

    if((TNull == hMemBuf) || (TNull == pBGR) || (TNull == label))
    {
        rVal = -1;
        goto EXIT;
    }
    dstWidth = region.right- region.left;
    dstHeight = region.bottom - region.top;

    if(!(  ((region.top>=0)&&(region.top<srcHeight))
       &&((region.bottom>0)&&(region.bottom<=srcHeight))
       &&((region.left>=0)&&(region.left<srcHeight))
       &&((region.right>0)&&(region.right<+srcHeight))
       &&((dstWidth>0)&&(dstWidth <= srcWidth))
       &&((dstHeight>0)&&(dstWidth <= srcHeight))
           ))
    {
        rVal =-1;
        goto EXIT;
    }

    pHSL       = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH*IMG_HEIGHT*3);
    pBGR_scale = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH*IMG_HEIGHT*3);
    pGray      = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH*IMG_HEIGHT);
    pFea       = (int *)TMemAlloc(hMemBuf, sizeof(*pFea)* SVM_FEA_DIM);
    if ((TNull == pHSL) || (TNull == pBGR_scale) || (TNull == pGray) || (TNull == pFea))
    {
        rVal = -1;
        goto EXIT;
    }
    
    // scale image to IMG_WIDTH
    rVal = ScaleImg3(pBGR, dstWidth, dstHeight, srcWidthStep,
                     pBGR_scale, IMG_WIDTH, IMG_HEIGHT, IMG_WIDTH*3);
    if(0 != rVal)
        goto EXIT;
    
    // convert to HSL
    rVal = BGRtoHSL(pBGR_scale, pHSL, IMG_HEIGHT, IMG_WIDTH, IMG_WIDTH*3, IMG_WIDTH*3);
    if(0 != rVal)
        goto EXIT;

    // convert to gray
    rVal = BGRtoGray(pBGR_scale, IMG_WIDTH, IMG_HEIGHT, IMG_WIDTH*3, pGray);
    if(0 != rVal)
        goto EXIT;

    //get wan feature
    rVal = WanHuaLinColorFea(pHSL, IMG_WIDTH*3, IMG_WIDTH,  IMG_HEIGHT, pFea);
    if(0 != rVal)
        goto EXIT;

    // get Hog Fea
    rVal =  HogFea(pGray, IMG_WIDTH,  IMG_HEIGHT, pFea+WAN_DIM);

    //predict
    
     rVal = SvmPredict(hMemBuf, pSvmModel, pFea, SVM_FEA_DIM, label);
  

    rVal = 0;
 EXIT:
    if(pHSL)       TMemFree(hMemBuf, pHSL);
    if(pBGR_scale) TMemFree(hMemBuf, pBGR_scale);
    if(pGray)      TMemFree(hMemBuf, pGray);
    if(pFea)       TMemFree(hMemBuf, pFea);
    return rVal;
}




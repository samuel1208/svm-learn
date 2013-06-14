#include "SVMDetector.h"
#include "tmem.h"
#include "svm_feature.h"
#include "svm_config.h"

int SVMDetector(THandle hMemBuf, svm_model *pSvmModel,int feaUsed,
                    TUInt8 *pBGR, int srcWidth, int srcHeight, int srcWidthStep,
                TRECT region, int *label)
{
    return SVMDetector_ex(hMemBuf, pSvmModel, feaUsed,
                          pBGR, srcWidth, srcHeight, srcWidthStep,
                          region, label, 0);
}
int SVMDetector_ex(THandle hMemBuf, svm_model *pSvmModel,int feaUsed,
                TUInt8 *pBGR, int srcWidth, int srcHeight, int srcWidthStep,
                TRECT region, int *label, int bIsExtended)
{
    int rVal = 0;
    int *pFea = TNull;
    int feaDim = 0;
    

    if((TNull == pBGR) || (TNull == label))
    {
        rVal = -1;
        goto EXIT;
    }
    if(feaUsed & FEAT_WAN_COLOR)
        feaDim += WAN_HUA_LIN_DIM;
    if((feaUsed & FEAT_HOG))
        feaDim += HOG_DIM;
    if((feaUsed & FEAT_LBP_8))
        feaDim += LBP_DIM_8;    
    if((feaUsed & FEAT_LBP_16))
        feaDim += LBP_DIM_16;
    if((feaUsed & FEAT_SURF))
        feaDim += SURF_LEN;
    
    pFea = (int *)TMemAlloc(hMemBuf, sizeof(*pFea)* feaDim);
    if  (TNull == pFea)
    {
        rVal = -1;
        goto EXIT;
    }
        //extend two side
    if(1 == bIsExtended)
    {
        int dstWidth = region.right- region.left;
        int dstHeight = region.bottom - region.top;
        int extend = (int)(dstWidth*0.2f), h;
        TUInt8 *extendBuf = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*dstHeight*(dstWidth+2*extend)*3);
        if(TNull == extendBuf)
        {
            rVal = -1;
            goto EXIT;
        }
        TMemSet(extendBuf, 0 ,sizeof(TUInt8)*dstHeight*(dstWidth+2*extend)*3);
        
        pBGR += region.left*3;
        for(h=0; h<dstHeight; h++)
        {
            TMemCpy(extendBuf+3*extend+h*(dstWidth+2*extend)*3, pBGR , dstWidth*3);
            pBGR += srcWidthStep;
        }
        
        dstWidth += 2*extend;
        region.left = region.top = 0;
        region.bottom = dstHeight;
        region.right = dstWidth;
        rVal = svm_feature(hMemBuf, extendBuf, dstWidth, dstHeight, 
                           dstWidth*3, region, pFea, feaUsed);
        TMemFree(hMemBuf, extendBuf);
    }
    else
        //get feature
        rVal = svm_feature(hMemBuf, pBGR, srcWidth, srcHeight, 
                           srcWidthStep, region, pFea, feaUsed);
    if(0 != rVal)
        goto EXIT; 

    //predict    
    // time_stamp(0,"svm_predict");
    rVal = SvmPredict(hMemBuf, pSvmModel, pFea, feaDim, label);
    if(0 != rVal)
        goto EXIT;  
    // time_stamp(1, "svm_predict");

    rVal = 0;
 EXIT:
    if(pFea)       TMemFree(hMemBuf, pFea);
    return rVal;
}




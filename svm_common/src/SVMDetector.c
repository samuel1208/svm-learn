#include "SVMDetector.h"
#include "tmem.h"
#include "svm_feature.h"
#include "svm_config.h"


int SVMDetector(THandle hMemBuf, svm_model *pSvmModel,int feaUsed,
                TUInt8 *pBGR, int srcWidth, int srcHeight, int srcWidthStep,
                TRECT region, int *label)
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
    
    pFea = (int *)TMemAlloc(hMemBuf, sizeof(*pFea)* feaDim);
    if  (TNull == pFea)
    {
        rVal = -1;
        goto EXIT;
    }
    
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




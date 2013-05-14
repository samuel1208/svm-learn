#include "svm_feature.h"
#include "WanHuaLinFea.h"
#include "HogFea.h"
#include "svm.h"
#include "svm_config.h"
#include "LBP_Fea.h"
#include "tcomdef.h"
#include "tmem.h"


int svm_feature(THandle hMemBuf,TUInt8 *pBGR, int srcWidth, int srcHeight, 
                int srcWidthStep, TRECT region, int *pFea, int feaUsed)
{
    int rVal = 0;
    TUInt8 *__pBGR, *pHSL = TNull, *pBGR_scale = TNull, *pGray = TNull;
    int dstWidth=0, dstHeight=0;
	int scaleWidthStep = 0;
    int *pDst = TNull;

    if((TNull == pBGR) || (TNull == pFea))
    {
        rVal = -1;
        goto EXIT;
    }

    dstWidth = region.right- region.left;
    dstHeight = region.bottom - region.top;

    if(!(  ((region.top>=0)&&(region.top<srcHeight))
       &&((region.bottom>0)&&(region.bottom<=srcHeight))
       &&((region.left>=0)&&(region.left<srcWidth))
       &&((region.right>0)&&(region.right<=srcWidth))
       &&((dstWidth>0)&&(dstWidth <= srcWidth))
       &&((dstHeight>0)&&(dstHeight<= srcHeight))
           ))
    {
        rVal =-1;
        goto EXIT;
    }

	if((dstWidth != IMG_WIDTH) || (dstHeight != IMG_HEIGHT))
	{
		scaleWidthStep = IMG_WIDTH*3;
		pBGR_scale = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*scaleWidthStep*IMG_HEIGHT);
		if(TNull == pBGR_scale)
			goto EXIT;
		
		__pBGR = pBGR + region.top*srcWidthStep + region.left*3;

		// scale image to IMG_WIDTH*IMG_HEIGHT
		rVal = ScaleImg3(__pBGR , dstWidth, dstHeight, srcWidthStep,
                     pBGR_scale, IMG_WIDTH, IMG_HEIGHT, scaleWidthStep);
		if(0 != rVal)
			goto EXIT;
	}
	else 
	{
		pBGR_scale = pBGR + region.top*srcWidthStep + region.left*3;
		scaleWidthStep = srcWidthStep;
	}


    pDst = pFea;
    if(feaUsed & FEAT_WAN_COLOR)
    {
        pHSL = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH*IMG_HEIGHT*3);
        if(TNull == pHSL)
        {
            rVal = -1;
            goto EXIT;
        }

        rVal = BGRtoHSL(pBGR_scale, pHSL, IMG_HEIGHT, IMG_WIDTH,
                        scaleWidthStep, IMG_WIDTH*3);
        if(0 != rVal)
            goto EXIT;
        
        //get wan feature : fast sample by setp 2
        rVal = WanHuaLinColorFea(pHSL, IMG_WIDTH*3, IMG_WIDTH,  IMG_HEIGHT, pDst);
        if(0 != rVal)
            goto EXIT;
        pDst += WAN_HUA_LIN_DIM;
    }

    if((feaUsed & FEAT_HOG) || (feaUsed & FEAT_LBP_16) || (feaUsed & FEAT_LBP_8))
    {
        pGray = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH*IMG_HEIGHT);
        if(TNull == pGray)
        {
            rVal = -1; 
            goto EXIT;
        }
        
        rVal = BGRtoGray(pBGR_scale, IMG_WIDTH, IMG_HEIGHT, scaleWidthStep, pGray);
        if(0 != rVal)
            goto EXIT;
    }

    if(feaUsed & FEAT_HOG)
    {
        if(HOG_DIM != HogFea(hMemBuf, pGray, IMG_WIDTH, IMG_WIDTH, IMG_HEIGHT,pDst))
        { 
            rVal = -1;
            goto EXIT;
        }
        pDst += HOG_DIM;
    }

    if(feaUsed & FEAT_LBP_16)
    {
        if(LBP_DIM_16 != LBPH_Fea(hMemBuf, pGray, IMG_WIDTH, IMG_WIDTH,
                                  IMG_HEIGHT, LBP_RADIUS_2,LBP_NEIGHBOR_16,
                                  LBP_GRID_X, LBP_GRID_Y, pDst))
        {
            rVal = -1;
            goto EXIT;
        }
    }
    
 EXIT:
    if(feaUsed & FEAT_WAN_COLOR)
    { 
        if(pHSL)       
            TMemFree(hMemBuf, pHSL);
    }
    
    if((feaUsed & FEAT_HOG) || (feaUsed & FEAT_LBP_8) || (feaUsed & FEAT_LBP_16))
    {
        if(pGray)      TMemFree(hMemBuf, pGray);
    }

	if((dstWidth != IMG_WIDTH) || (dstHeight != IMG_HEIGHT))
	{
		if(pBGR_scale) TMemFree(hMemBuf, pBGR_scale);
	}
    return rVal;
}

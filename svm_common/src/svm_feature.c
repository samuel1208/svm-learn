#include "svm_feature.h"
#include "WanHuaLinFea.h"
#include "HogFea.h"
#include "LBP_Fea.h"
#include "tcomdef.h"
#include "tmem.h"
#include "SURFDescriptor.h"


int svm_feature(THandle hMemBuf,TUInt8 *pBGR, int srcWidth, int srcHeight, 
                int srcWidthStep, TRECT region, int *pFea, int feaUsed,
                int IMG_WIDTH_BASE, int IMG_HEIGHT_BASE)
{
    int rVal = 0;
    TUInt8 *__pBGR, *pHSL = TNull, *pBGR_scale = TNull, *pGray = TNull;
    int dstWidth=0, dstHeight=0;
	int scaleWidthStep = 0;
    int *pDst = TNull;
    int feaDim = 0;

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

	if((dstWidth != IMG_WIDTH_BASE) || (dstHeight != IMG_HEIGHT_BASE))
	{
		scaleWidthStep = IMG_WIDTH_BASE*3;
		pBGR_scale = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*scaleWidthStep*IMG_HEIGHT_BASE);
		if(TNull == pBGR_scale)
        {
            rVal =-1;
			goto EXIT;
		}
		__pBGR = pBGR + region.top*srcWidthStep + region.left*3;

		// scale image to IMG_WIDTH_BASE*IMG_HEIGHT_BASE
		rVal = ScaleImg3(__pBGR , dstWidth, dstHeight, srcWidthStep,
                     pBGR_scale, IMG_WIDTH_BASE, IMG_HEIGHT_BASE, scaleWidthStep);
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
        pHSL = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH_BASE*IMG_HEIGHT_BASE*3);
        if(TNull == pHSL)
        {
            rVal = -1;
            goto EXIT;
        }

        rVal = BGRtoHSL(pBGR_scale, pHSL, IMG_HEIGHT_BASE, IMG_WIDTH_BASE,
                        scaleWidthStep, IMG_WIDTH_BASE*3);
        if(0 != rVal)
            goto EXIT;
        
        //get wan feature : fast sample by setp 2
        rVal = WanHuaLinColorFea(pHSL, IMG_WIDTH_BASE*3, IMG_WIDTH_BASE,  IMG_HEIGHT_BASE, pDst);
        if(0 != rVal)
            goto EXIT;
        pDst += GetWANDim();
    }

    if((feaUsed & FEAT_SURF)||(feaUsed & FEAT_HOG) 
       || (feaUsed & FEAT_LBP_16) || (feaUsed & FEAT_LBP_8))
    {
        pGray = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH_BASE*IMG_HEIGHT_BASE);
        if(TNull == pGray)
        {
            rVal = -1; 
            goto EXIT;
        }
        
        rVal = BGRtoGray(pBGR_scale, IMG_WIDTH_BASE, IMG_HEIGHT_BASE, scaleWidthStep, pGray);
        if(0 != rVal)
            goto EXIT;
    }

    feaDim = GetHOGDim(IMG_WIDTH_BASE, IMG_HEIGHT_BASE);
    if(feaUsed & FEAT_HOG)
    {
        if(feaDim != HogFea(hMemBuf, pGray, IMG_WIDTH_BASE,
                            IMG_WIDTH_BASE, IMG_HEIGHT_BASE,pDst))
        { 
            rVal = -1;
            goto EXIT;
        }
        pDst += feaDim;
    }

    feaDim = GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y);
    if(feaUsed & FEAT_LBP_16)
    {
        if(feaDim != LBPH_Fea(hMemBuf, pGray, IMG_WIDTH_BASE, IMG_WIDTH_BASE,
                              IMG_HEIGHT_BASE, 2 , 16,
                              LBP_GRID_X, LBP_GRID_Y, pDst))
        {
            rVal = -1;
            goto EXIT;
        }
    }

    feaDim = GetSURFDim();
    if(feaUsed & FEAT_SURF)
    {
        if(feaDim != SURFFea(hMemBuf, pGray, IMG_WIDTH_BASE,
                             IMG_WIDTH_BASE, IMG_HEIGHT_BASE,pDst))
        { 
            rVal = -1;
            goto EXIT;
        }
        pDst += feaDim;
    }
    
 EXIT:
    if(feaUsed & FEAT_WAN_COLOR)
    { 
        if(pHSL)       
            TMemFree(hMemBuf, pHSL);
    }
    
    if((feaUsed & FEAT_HOG) || (feaUsed & FEAT_LBP_8) || (feaUsed & FEAT_LBP_16)||(feaUsed & FEAT_SURF))
    {
        if(pGray)      TMemFree(hMemBuf, pGray);
    }

	if((dstWidth != IMG_WIDTH_BASE) || (dstHeight != IMG_HEIGHT_BASE))
	{
		if(pBGR_scale) TMemFree(hMemBuf, pBGR_scale);
	}
    return rVal;
}

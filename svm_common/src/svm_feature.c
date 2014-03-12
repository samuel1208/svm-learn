#include "svm_feature.h"
#include "WanHuaLinFea.h"
#include "HogFea.h"
#include "LBP_Fea.h"
#include "tcomdef.h"
#include "tmem.h"
#include "SURFDescriptor.h"

static int ScaleImg3(unsigned char *pSrc, int srcWidth, int srcHeight,
                     int srcWidthStep,
                     unsigned char *pDst, int dstWidth, int dstHeight,
                     int dstWidthStep)
{
    int w,h, index_x, index_y;
    float scale_x, scale_y;
    unsigned char *pData;
    
    if((TNull == pSrc)||(TNull == pDst))
        return -1;
    
    scale_x = srcWidth*1.0f/dstWidth;
    scale_y = srcHeight*1.0f/dstHeight;

    pData = pDst;
    for(h=0; h<dstHeight; h++)
    {
        index_y = (int)(h*scale_y/* + 0.5f*/); //the same to opencv CV_INTER_NN
        index_y = TMIN(index_y, srcHeight-1);
        
        for(w=0; w<dstWidth; w++)
        {
            index_x = (int)(w*scale_x/* + 0.5f*/);
            index_x = TMIN(index_x, srcWidth-1);
            pData[3*w] = pSrc[index_y*srcWidthStep + index_x*3];
            pData[3*w+1] = pSrc[index_y*srcWidthStep + index_x*3+1];
            pData[3*w+2] = pSrc[index_y*srcWidthStep + index_x*3+2];
        }
        pData += dstWidthStep;
    }
    return 0;
}

static int ScaleImg1(unsigned char *pSrc, int srcWidth, int srcHeight,
                     int srcWidthStep,
                     unsigned char *pDst, int dstWidth, int dstHeight,
                     int dstWidthStep)
{
    int w,h, index_x, index_y;
    float scale_x, scale_y;
    unsigned char *pData;
    
    if((TNull == pSrc)||(TNull == pDst))
        return -1;
    
    scale_x = srcWidth*1.0f/dstWidth;
    scale_y = srcHeight*1.0f/dstHeight;

    pData = pDst;
    for(h=0; h<dstHeight; h++)
    {
        index_y = (int)(h*scale_y/* + 0.5f*/); //the same to opencv CV_INTER_NN
        index_y = TMIN(index_y, srcHeight-1);
        
        for(w=0; w<dstWidth; w++)
        {
            index_x = (int)(w*scale_x/* + 0.5f*/);
            index_x = TMIN(index_x, srcWidth-1);
            pData[w] = pSrc[index_y*srcWidthStep + index_x];
        }
        pData += dstWidthStep;
    }
    return 0;
}
#include <string.h>
#include <stdio.h>
int svm_feature_gray(THandle hMemBuf,TUInt8 *pGray, int srcWidth, int srcHeight, 
                     int srcWidthStep, TRECT region, int *pFea, int feaUsed,
                     int IMG_WIDTH_BASE, int IMG_HEIGHT_BASE)
{
    int rVal = 0;
    TUInt8 *pGray_scale = TNull;
    int dstWidth=0, dstHeight=0;
	int scaleWidthStep = 0;
    int *pDst = TNull;
    int feaDim = 0;
    
    if((TNull == pGray) || (TNull == pFea))
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
        TUInt8 *__pGray = pGray + region.top*srcWidthStep + region.left;
        pGray_scale = (TUInt8 *)TMemAlloc(hMemBuf, sizeof(TUInt8)*IMG_WIDTH_BASE*IMG_HEIGHT_BASE);
        if(TNull == pGray)
        {
            rVal = -1; 
            goto EXIT;
        }
        scaleWidthStep = IMG_WIDTH_BASE;
        // scale image to IMG_WIDTH_BASE*IMG_HEIGHT_BASE
		rVal = ScaleImg1(__pGray , dstWidth, dstHeight, srcWidthStep,
                         pGray_scale, IMG_WIDTH_BASE, IMG_HEIGHT_BASE,
                         scaleWidthStep);
		if(0 != rVal)
			goto EXIT;
	}
	else 
	{
		pGray_scale = pGray + region.top*srcWidthStep + region.left;
		scaleWidthStep = srcWidthStep;
	}

    pDst = pFea;
    feaDim = GetHOGDim(IMG_WIDTH_BASE, IMG_HEIGHT_BASE);
    if(feaUsed & FEAT_HOG)
    {
        if(feaDim != HogFea(hMemBuf, pGray_scale, IMG_WIDTH_BASE,
                            IMG_WIDTH_BASE, IMG_HEIGHT_BASE,pDst))
        { 
            rVal = -1;
            goto EXIT;
        }
        pDst += feaDim;
    }

    feaDim = GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y, 0);
    if(feaUsed & FEAT_LBP_16)
    {
        if(feaDim != LBPH_Fea(hMemBuf, pGray_scale, IMG_WIDTH_BASE, IMG_WIDTH_BASE,
                              IMG_HEIGHT_BASE, 2 , 16,
                              LBP_GRID_X, LBP_GRID_Y, pDst))
        {
            rVal = -1;
            goto EXIT;
        }
    }

    feaDim = GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y, 0);
    if(feaUsed & FEAT_LBP_8)
    {
        if(feaDim != LBPH_Fea(hMemBuf, pGray_scale, IMG_WIDTH_BASE, IMG_WIDTH_BASE,
                              IMG_HEIGHT_BASE, 1 , 8,
                              LBP_GRID_X, LBP_GRID_Y, pDst))
        {
            rVal = -1;
            goto EXIT;
        }
    }

    feaDim = GetSURFDim();
    if(feaUsed & FEAT_SURF)
    {
        if(feaDim != SURFFea(hMemBuf, pGray_scale, IMG_WIDTH_BASE,
                             IMG_WIDTH_BASE, IMG_HEIGHT_BASE,pDst))
        { 
            rVal = -1;
            goto EXIT;
        }
        pDst += feaDim;
    }
    
 EXIT:
    if((dstWidth != IMG_WIDTH_BASE) || (dstHeight != IMG_HEIGHT_BASE))
	{
        if(pGray)      TMemFree(hMemBuf, pGray_scale);
	}
    return rVal;
}


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

    feaDim = GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y, 0);
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
    feaDim = GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y, 1);
    if(feaUsed & FEAT_LBP_OV_16)
    {
        if(feaDim != LBPH_Fea_OV(hMemBuf, pGray, IMG_WIDTH_BASE, IMG_WIDTH_BASE,
                                 IMG_HEIGHT_BASE, 2 , 16,
                                 LBP_GRID_X, LBP_GRID_Y, pDst))
        {
            rVal = -1;
            goto EXIT;
        }
    }

    feaDim = GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y, 0);
    if(feaUsed & FEAT_LBP_8)
    {
        if(feaDim != LBPH_Fea(hMemBuf, pGray, IMG_WIDTH_BASE, IMG_WIDTH_BASE,
                              IMG_HEIGHT_BASE, 1 , 8,
                              LBP_GRID_X, LBP_GRID_Y, pDst))
        {
            rVal = -1;
            goto EXIT;
        }
    }

    feaDim = GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y, 1);
    if(feaUsed & FEAT_LBP_OV_8)
    {
        if(feaDim != LBPH_Fea_OV(hMemBuf, pGray, IMG_WIDTH_BASE, IMG_WIDTH_BASE,
                                 IMG_HEIGHT_BASE, 1 , 8,
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

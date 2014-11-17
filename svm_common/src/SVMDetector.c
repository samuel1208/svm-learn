#include "SVMDetector.h"
#include "tmem.h"
#include "svm.h"
#include "svm_feature.h"
#include "HogFea.h"
#include "LBP_Fea.h"
#include "SURFDescriptor.h"
#include "WanHuaLinFea.h"

typedef struct __tagSvmDetector
{
    svm_model *pSvmModel;
    int width_base;
    int height_base;
    int feaUsed;
    THandle hMemBuf;
}SvmDetector;

extern svm_model *g_pSvmModel_scene;
static int feaUsed_scene = FEAT_HOG;
static int width_base_scene = 64;
static int height_base_scene = 64;


extern svm_model *g_pSvmModel_face;
static int feaUsed_face = FEAT_HOG;
static int width_base_face = 48;
static int height_base_face = 48;

extern svm_model *g_pSvmModel_smile;
static int feaUsed_smile = FEAT_SURF;
static int width_base_smile = 48;
static int height_base_smile = 48;

extern svm_model *g_pSvmModel_gesture;
static int feaUsed_gesture = FEAT_SURF;
static int width_base_gesture = 48;
static int height_base_gesture = 48;

extern svm_model *g_pSvmModel_gender;
static int feaUsed_gender = FEAT_LBP_OV_8;
static int width_base_gender = 48;
static int height_base_gender = 48;


static int SVMDetector_detect_ex(THandle hDetector, 
                                 TUInt8 *pBGR, int srcWidth, int srcHeight,
                                 int srcWidthStep, TRECT region, int *label, 
                                 int bIsExtended);

static int SVMDetector_detect_gray(THandle hDetector, 
                                   TUInt8 *pGray, int srcWidth, int srcHeight,
                                   int srcWidthStep, TRECT region, int *label);

THandle SVMDetector_init(THandle hMemBuf, const char *name)
{
    SvmDetector *hDetector = 0;
    
    hDetector = (SvmDetector *)TMemAlloc(hMemBuf, sizeof(SvmDetector));
    if (0 == hDetector)
        return 0;
    TMemSet(hDetector, 0, sizeof(SvmDetector));

    //// Set the svm model
    if(strcmp(name,"face") == 0)
        hDetector->pSvmModel = g_pSvmModel_face;
    else if(strcmp(name,"smile") == 0)
        hDetector->pSvmModel = g_pSvmModel_smile;
    else if(strcmp(name,"gesture") == 0)
        hDetector->pSvmModel = g_pSvmModel_gesture;
	else if(strcmp(name,"gender") == 0)
        hDetector->pSvmModel = g_pSvmModel_gender;
	else if(strcmp(name,"scene") == 0)
        hDetector->pSvmModel = g_pSvmModel_scene;

    if (0 == hDetector->pSvmModel)
    {
        TMemFree(hMemBuf, hDetector);
        hDetector = 0;
        return 0;
    }

    /// Set the base Para
    if(strcmp(name,"face") == 0)
    {
        hDetector->width_base = width_base_face;
        hDetector->height_base = height_base_face;
        hDetector->feaUsed = feaUsed_face;
    }
    else if(strcmp(name,"smile") == 0)
    {
        hDetector->width_base = width_base_smile;
        hDetector->height_base = height_base_smile;
        hDetector->feaUsed = feaUsed_smile;
    }
    else if(strcmp(name,"gesture") == 0)
    {
        hDetector->width_base = width_base_gesture;
        hDetector->height_base = height_base_gesture;
        hDetector->feaUsed = feaUsed_gesture;
    }
	else if(strcmp(name,"gender") == 0)
    {
        hDetector->width_base = width_base_gender;
        hDetector->height_base = height_base_gender;
        hDetector->feaUsed = feaUsed_gender;
    }
	else if(strcmp(name,"scene") == 0)
    {
        hDetector->width_base = width_base_scene;
        hDetector->height_base = height_base_scene;
        hDetector->feaUsed = feaUsed_scene;
    }
    
    hDetector->hMemBuf = hMemBuf;
    return hDetector;
}

void SVMDetector_uninit(THandle *hDetector)
{
    if (*hDetector)
    {
        TMemFree(((SvmDetector *)(*hDetector))->hMemBuf, (*hDetector));
        *hDetector = 0;
    }        
}

int SVMDetector_detect(THandle hDetector, 
                       TUInt8 *pData, COLOR_FORMAT format,
                       int srcWidth, int srcHeight, int srcWidthStep,
                       TRECT region, int *label)
{
    if (FORMAT_BGR == format)
        return SVMDetector_detect_ex(hDetector, 
                                     pData, srcWidth, srcHeight, srcWidthStep,
                                     region, label, 0);
    else if (FORMAT_GRAY == format)
        return SVMDetector_detect_gray(hDetector, 
                                       pData, srcWidth, srcHeight, srcWidthStep,
                                       region, label);
    return -1;
}


int SVMDetector_detect_gray(THandle hDetector, 
                            TUInt8 *pGray, int srcWidth, int srcHeight,
                            int srcWidthStep, TRECT region, int *label)
{
    int rVal = 0;
    int *pFea = TNull;
    int feaDim = 0;

    SvmDetector *detector = (SvmDetector *)hDetector;
    THandle hMemBuf = detector->hMemBuf;
    int feaUsed = detector->feaUsed;    

    if((TNull == pGray) || (TNull == label) || (TNull == detector))
    {
        rVal = -1;
        goto EXIT;
    }

    if((feaUsed & FEAT_HOG))
        feaDim += GetHOGDim(detector->width_base, detector->height_base);
    if((feaUsed & FEAT_LBP_8))
        feaDim += GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y, 0);    
    if((feaUsed & FEAT_LBP_16))
        feaDim += GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y, 0);
    if((feaUsed & FEAT_LBP_OV_8))
        feaDim += GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y, 1);    
    if((feaUsed & FEAT_LBP_OV_16))
        feaDim += GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y, 1);
    if((feaUsed & FEAT_SURF))
        feaDim += GetSURFDim();
    
    pFea = (int *)TMemAlloc(hMemBuf, sizeof(*pFea)* feaDim);
    if  (TNull == pFea)
    {
        rVal = -1;
        goto EXIT;
    }

    //get feature
    rVal = svm_feature_gray(hMemBuf, pGray, srcWidth, srcHeight, 
                            srcWidthStep, region, pFea, feaUsed,
                            detector->width_base, detector->height_base);
    if(0 != rVal)
        goto EXIT; 

    //predict    
    // time_stamp(0,"svm_predict");
    rVal = SvmPredict(hMemBuf, detector->pSvmModel, pFea, feaDim, label);
    if(0 != rVal)
        goto EXIT;  
    // time_stamp(1, "svm_predict");

    rVal = 0;
 EXIT:
    if(pFea)       TMemFree(hMemBuf, pFea);
    return rVal;
}

static int SVMDetector_detect_ex(THandle hDetector, 
                                 TUInt8 *pBGR, int srcWidth, int srcHeight,
                                 int srcWidthStep, TRECT region, int *label, 
                                 int bIsExtended)
{
    int rVal = 0;
    int *pFea = TNull;
    int feaDim = 0;

    SvmDetector *detector = (SvmDetector *)hDetector;
    THandle hMemBuf = detector->hMemBuf;
    int feaUsed = detector->feaUsed;    

    if((TNull == pBGR) || (TNull == label) || (TNull == detector))
    {
        rVal = -1;
        goto EXIT;
    }
    if(feaUsed & FEAT_WAN_COLOR)
        feaDim += GetWANDim();
    if((feaUsed & FEAT_HOG))
        feaDim += GetHOGDim(detector->width_base, detector->height_base);
    if((feaUsed & FEAT_LBP_8))
        feaDim += GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y, 0);    
    if((feaUsed & FEAT_LBP_OV_8))
        feaDim += GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y, 1);    
    if((feaUsed & FEAT_LBP_16))
        feaDim += GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y, 0);
    if((feaUsed & FEAT_LBP_OV_16))
        feaDim += GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y, 1);
    if((feaUsed & FEAT_SURF))
        feaDim += GetSURFDim();
    
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
                           dstWidth*3, region, pFea, feaUsed,
                           detector->width_base, detector->height_base);
        TMemFree(hMemBuf, extendBuf);
    }
    else
        //get feature
        rVal = svm_feature(hMemBuf, pBGR, srcWidth, srcHeight, 
                           srcWidthStep, region, pFea, feaUsed,
                           detector->width_base, detector->height_base);
    if(0 != rVal)
        goto EXIT; 

    //predict    
    // time_stamp(0,"svm_predict");
    rVal = SvmPredict(hMemBuf, detector->pSvmModel, pFea, feaDim, label);
    if(0 != rVal)
        goto EXIT;  
    // time_stamp(1, "svm_predict");

    rVal = 0;
 EXIT:
    if(pFea)       TMemFree(hMemBuf, pFea);
    return rVal;
}




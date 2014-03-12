#include "LBP_Fea.h"
#include "tmem.h"
#include <math.h>
#include "LBP_lookup.h"

/* #include <opencv/cv.h> */
/* #include <opencv/highgui.h> */

/*
  MAX_RADIUS=3  make the radius-2 and radius-3 have interal block num.
  (only work for 24 and 48) 
*/
#define LBP_MAX_RADIUS (3)

int GetLBPDim(int neighbor , int lbp_grid_x, int lbp_grid_y, int isOV)
{
    if (1 == isOV)
    {
        lbp_grid_x = lbp_grid_x*2 -1;
        lbp_grid_y = lbp_grid_y*2 -1;
    }
    
        
#ifdef LBP_USE_UNIFORM_2
    if (8==neighbor)
        return 59*lbp_grid_x *lbp_grid_y;
    else if (16==neighbor)
        return 243*lbp_grid_x *lbp_grid_y;
#else
    return (neighbor+2)* lbp_grid_x *lbp_grid_y;
#endif
}


static int getLBPImg(unsigned char *pImg, int widthStep, int width, int height, 
                     int *pLBPImg,  int width_lbp, int height_lbp, int radius, int neighbor)
{
    const float pi = 3.141593f;
    int n, w, h;
    unsigned char *pSrc;
    unsigned char *pLookupTable = TNull;
    int *pDst;
    unsigned char val_c;
    unsigned int  val_lbp;
    if((TNull == pImg) || (TNull == pLBPImg))
        return -1;

#ifdef LBP_USE_UNIFORM_2
    if(8 == neighbor)
        pLookupTable = pLBP_lookUP_table_8_u2;
	else if(16 == neighbor)
        pLookupTable = pLBP_lookUP_table_16_u2;
#else
    if(8 == neighbor)
        pLookupTable = pLBP_lookUP_table_8_roi_u2;
	else if(16 == neighbor)
        pLookupTable = pLBP_lookUP_table_16_roi_u2;
#endif
    else 
        return -1;

    TMemSet(pLBPImg, 0, width_lbp*height_lbp*sizeof(int));
    
    pSrc = pImg + LBP_MAX_RADIUS * widthStep;
    pDst = pLBPImg;
    
    for(h=LBP_MAX_RADIUS; h<height-LBP_MAX_RADIUS; h++)
    {
        for(w=LBP_MAX_RADIUS; w<width-LBP_MAX_RADIUS; w++)
        {
            val_c = pSrc[w];
            val_lbp = 0;
            for(n=0; n<neighbor; n++)
            {
                // should move outside
                float val_f;
                float x = (float)(-radius * sin(2.0*pi*n/(float)(neighbor))) + w;
                float y = (float)(radius * cos(2.0*pi*n/(float)(neighbor))) + h;

                int x1  = (int)x; 
                int y1 =  (int)y;
                float fc_x = x-x1;
                float fc_y = y-y1;
                int x2 = x1+1;
                int y2 = y1+1;
                unsigned char v1, v2, v3, v4;
                //               float w1,w2,w3,w4;
                v1 = pImg[y1*widthStep + x1];
                v2 = pImg[y1*widthStep + x2];
                v3 = pImg[y2*widthStep + x1];
                v4 = pImg[y2*widthStep + x2];

                /* w1 = (1-fc_y)*(1-fc_x); */
                /* w2 = (1-fc_y)*fc_x; */
                /* w3 = fc_y*(1-fc_x); */
                /* w4 = (fc_y*fc_x); */
                
                //val_f = w1*v1 + w2*v2 + w3*v3 + w4*v4;
                val_f = (1-fc_y)*((1-fc_x)*v1 + fc_x*v2) + fc_y*((1-fc_x)*v3 + fc_x*v4);

                val_lbp += ((val_f>val_c) || (fabs(val_f-val_c)<0.0000001f))<<n;                 
            }
            
            // get the min val
            // pDst[w] = val_lbp;
            pDst[w-LBP_MAX_RADIUS] = pLookupTable[val_lbp];
        }
        pSrc += widthStep;
        pDst += width_lbp;
    }
    return 0;    
}

static int getBlockLBPH(int *pLBPImg,int widthStep, int width, int height, int *pHist, int histBin)
{
    int x, y;
    int *pSrc;
    if((TNull == pLBPImg) || (TNull == pHist))
        return -1;
    
    pSrc = pLBPImg;
    TMemSet(pHist, 0, histBin*sizeof(int));
    for(y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            pHist[pSrc[x]]++;            
        }
        pSrc += widthStep;
    }
    for(x=0; x<histBin; x++)
        pHist[x] = (pHist[x]/(width*height*1.0f))*(1<<24);

    return 0;   
}

int LBPH_Fea(THandle hMem, unsigned char *pSrcImg, int widthStep,
             int width, int height, int radius, int neighbor, 
             int grid_x, int grid_y, int *pFea)
{
    int nFeaNum = 0;
    int x, y;
    int grid_width, grid_height;
    int width_lbp, height_lbp;
    int histBin;
    
    int *pLBPImg = TNull;
    if((TNull == pSrcImg) || (TNull == pFea))
        goto EXIT;

    if((grid_x<1)||(grid_x>width) || (grid_y<1) || (grid_y>height))
        goto EXIT;

    width_lbp = width - 2*LBP_MAX_RADIUS;
    height_lbp = height - 2*LBP_MAX_RADIUS;
    pLBPImg = (int *)TMemAlloc(hMem, (width_lbp) * (height_lbp) * sizeof(int));
    if(TNull == pLBPImg)
        goto EXIT;
        
     if( 0 != getLBPImg(pSrcImg, widthStep, width, height,
                        pLBPImg, width_lbp, height_lbp, radius, neighbor))
         goto EXIT;
    
     /*      
	{	
        int _i, _j;
		IplImage  *tmp_lbp  = cvCreateImage(cvSize(width_lbp, height_lbp), 8, 1);
        IplImage  *tmp_1  = cvCreateImage(cvSize(width, height), 8, 1);

        cvNamedWindow("LBP", 0);
        for(_i=0; _i<height; _i++)
        {
            for(_j=0; _j<width; _j++)
            {
                tmp_1->imageData[_i*tmp_1->widthStep + _j] = pSrcImg[_i*widthStep + _j];
            }
        }
        cvShowImage("LBP", tmp_1);
        cvWaitKey(0);

        for(_i=0; _i<height_lbp; _i++)
        {
            for(_j=0; _j<width_lbp; _j++)
            {
                tmp_lbp->imageData[_i*tmp_lbp->widthStep + _j] = pLBPImg[_i*width_lbp + _j];
            }
        }     
        cvShowImage("LBP", tmp_lbp);
        cvWaitKey(0);
		cvReleaseImage(&tmp_lbp);
        cvReleaseImage(&tmp_1);
        cvDestroyWindow("LBP");
	}
    */
     
    
    grid_width = width_lbp / grid_x;
    grid_height = height_lbp / grid_y;

#ifdef LBP_USE_UNIFORM_2
    if (8==neighbor)
        histBin = 59;
    else if (16==neighbor)
        histBin = 243;
#else
    histBin = neighbor+2;
#endif
    
    for(y=0; y<grid_y; y++)
    {
        for(x=0; x<grid_x; x++)
        {
            if(0 != getBlockLBPH(pLBPImg+(y*grid_height*width_lbp + x*grid_width),
                                 width_lbp, grid_width, grid_height,
                                 pFea+(histBin*(y*grid_x+x)), histBin))
                goto EXIT;
        }
    }

    nFeaNum = GetLBPDim(neighbor , grid_x, grid_y, 0);
 EXIT:
    if(pLBPImg) TMemFree(hMem, pLBPImg);
    return nFeaNum;
}

int LBPH_Fea_OV(THandle hMem, unsigned char *pSrcImg, int widthStep,
                int width, int height, int radius, int neighbor, 
                int grid_x, int grid_y, int *pFea)
{
    int nFeaNum = 0;
    int x, y;
    int grid_width, grid_height;
    int width_lbp, height_lbp;
    int histBin;
    int grid_x_ov, grid_y_ov;
    
    int *pLBPImg = TNull;
    if((TNull == pSrcImg) || (TNull == pFea))
        goto EXIT;

    if((grid_x<1)||(grid_x>width) || (grid_y<1) || (grid_y>height))
        goto EXIT;

    width_lbp = width - 2*LBP_MAX_RADIUS;
    height_lbp = height - 2*LBP_MAX_RADIUS;
    pLBPImg = (int *)TMemAlloc(hMem, (width_lbp) * (height_lbp) * sizeof(int));
    if(TNull == pLBPImg)
        goto EXIT;
        
    if( 0 != getLBPImg(pSrcImg, widthStep, width, height,
                       pLBPImg, width_lbp, height_lbp, radius, neighbor))
        goto EXIT;
    
    grid_width = width_lbp / grid_x;
    grid_height = height_lbp / grid_y;

#ifdef LBP_USE_UNIFORM_2
    if (8==neighbor)
        histBin = 59;
    else if (16==neighbor)
        histBin = 243;
#else
    histBin = neighbor+2;
#endif
    
    grid_x_ov = 2 * grid_x -1;
    grid_y_ov = 2 * grid_y -1;
    for(y=0; y<grid_y_ov; y++)
    {
        for(x=0; x<grid_x_ov; x++)
        {
            int *ptr = pLBPImg + y*(grid_height/2)*width_lbp + x*grid_width/2;
            if(0 != getBlockLBPH(ptr,
                                 width_lbp, grid_width, grid_height,
                                 pFea+(histBin*(y*grid_x_ov+x)), histBin))
                goto EXIT;
        }
    }

    nFeaNum = GetLBPDim(neighbor , grid_x, grid_y, 1);
 EXIT:
    if(pLBPImg) TMemFree(hMem, pLBPImg);
    return nFeaNum;
}


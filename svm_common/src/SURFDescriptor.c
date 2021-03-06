#include "SURFDescriptor.h"
#include <math.h>
#define TABS(x)   ((x)>0? (x):(-x))  

#define SURF_CELL_NUM_X (4)
#define SURF_CELL_NUM_Y (4)
#define  SURF_TYPE      (8)

int GetSURFDim()
{
    return (SURF_CELL_NUM_X * SURF_CELL_NUM_Y * SURF_TYPE);
}

static int __getGradient_simple(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height);
static int __getCellFea(int *pGx, int *pGy, int nWidthStep, int width, int height, int *pFea);

static int __getGradient_simple(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height)
{
    int w, h;
    const unsigned char *pSrc;
    int *pDst_x, *pDst_y;
    unsigned char x1,y1 , x2 , y2;

    if ((TNull == pGrayImg) || (TNull == pGradient_x) || (TNull == pGradient_y))
        return -1;

    height -= 1;
    width  -= 1;
    pDst_x = pGradient_x;
    pDst_y = pGradient_y;
    pSrc   = pGrayImg;
   
    //the first line
    w=0;
    x1 = 0;
    x2 = pSrc[w + 1];
    y1 = pSrc[w + widthStep_src];
    y2 = 0;
    pDst_x[w] = (x2-x1);
    pDst_y[w] = (y2-y1);
    for(w=1; w<width; w++)
    {
        x1 = pSrc[w - 1];
        x2 = pSrc[w + 1];
        y1 = pSrc[w + widthStep_src];
        y2 = 0;
        pDst_x[w] = (x2-x1);
        pDst_y[w] = (y2-y1);
    }        
    x1 = pSrc[w-1];
    x2 = 0;
    y1 = pSrc[w + widthStep_src];
    y2 = 0;
    pDst_x[w] = (x2-x1);
    pDst_y[w] = (y2-y1);
    
    //the middle lines
    for(h=1; h<height; h++)
    {
        pDst_x += widthStep_dst;
        pDst_y += widthStep_dst;
        pSrc   += widthStep_src;

        w=0;
        x1 = 0;
        x2 = pSrc[w + 1];
        y1 = pSrc[w + widthStep_src];
        y2 = pSrc[w - widthStep_src];
        pDst_x[w] = (x2-x1);
        pDst_y[w] = (y2-y1);
        for(w=1;w<width; w++)
        {
            x1 = pSrc[w - 1];
            x2 = pSrc[w + 1];
            y1 = pSrc[w + widthStep_src];
            y2 = pSrc[w - widthStep_src];
            pDst_x[w] = (x2-x1);
            pDst_y[w] = (y2-y1);            
        }        
        x1 = pSrc[w - 1];
        x2 = 0;
        y1 = pSrc[w + widthStep_src];
        y2 = pSrc[w - widthStep_src];
        pDst_x[w] = (x2-x1);
        pDst_y[w] = (y2-y1);
    }

    //the last line
    pDst_x += widthStep_dst;
    pDst_y += widthStep_dst;
    pSrc   += widthStep_src;

    w=0;
    x1 = 0;
    x2 = pSrc[w + 1];
    y1 = 0;
    y2 = pSrc[w - widthStep_src];
    pDst_x[w] = (x2-x1);
    pDst_y[w] = (y2-y1);
    for(w=1;w<width; w++)
    {
        x1 = pSrc[w - 1];
        x2 = pSrc[w + 1];
        y1 = 0;
        y2 = pSrc[w - widthStep_src];
        pDst_x[w] = (x2-x1);
        pDst_y[w] = (y2-y1);
    }        
    x1 = pSrc[w - 1];
    x2 = 0;
    y1 = 0;
    y2 = pSrc[w - widthStep_src];
    pDst_x[w] = (x2-x1);
    pDst_y[w] = (y2-y1);
    
    return 0;    
}
static int __getCellFea(int *pGx, int *pGy, int widthStep, 
                        int width, int height, int *pFea)
{
    int w, h;
    int *ptr1, *ptr2;
    int val1, val2;
    int fea[SURF_TYPE] = {0};

    if((TNull == pGx) || (TNull == pGy))
        return -1;

    ptr1 = pGx;
    ptr2 = pGy;
    
    for(h=0; h<height; h++)
    {
        for(w=0; w<width; w++)
        {
            val1 = ptr1[w];
            val2 = ptr2[w];
            if(val1 < 0)
            {
                fea[0] += val2;
                fea[1] += TABS(val2);
            }
            else
            {
                fea[2] += val2;
                fea[3] += TABS(val2);
            }
            if(val2 < 0)
            {
                fea[4] += val1;
                fea[5] += TABS(val1);
            }
            else
            {
                fea[6] += val1;
                fea[7] += TABS(val1);
            }
        }        
        ptr1 += widthStep;
        ptr2 += widthStep;
    }
    
    TMemCpy(pFea, fea , SURF_TYPE * sizeof(int));
    return 0;
}

int SURFFea(THandle hMemBuf, unsigned char *pGray, int nWidthStep, int width, int height, int *pSURFFea)
{
    int rVal = 0;
    int x, y;
    int cell_width, cell_height;
    int *pGx = TNull, *pGy = TNull;
    float *pFea_temp = TNull;
    double norm = 0;
    float val;
    float clip_val = 0.2;
    int SURF_LEN =0; 
    if((TNull == pGray) || (TNull == pSURFFea))
    {
        rVal = -1;
        goto EXIT;        
    }

    SURF_LEN = GetSURFDim();
    pGx = (int *)TMemAlloc(hMemBuf, width *height*sizeof(int));
    pGy = (int *)TMemAlloc(hMemBuf, width *height*sizeof(int));
    pFea_temp = (float *)TMemAlloc(hMemBuf, SURF_LEN*sizeof(float));
    
    if((TNull == pGx) || (TNull == pGy) || (TNull == pFea_temp))
    {
        rVal = -1;
        goto EXIT;
    }

    rVal = __getGradient_simple(pGray, nWidthStep, pGx, pGy, width, width, height);
    if(0 != rVal)
        goto EXIT;
    
    cell_width = width / SURF_CELL_NUM_X;
    cell_height = height / SURF_CELL_NUM_Y;
    
    for(y=0; y<SURF_CELL_NUM_Y; y++)
    {
        for(x=0; x<SURF_CELL_NUM_X; x++)
        {
            __getCellFea(pGx+x*cell_width+ y*cell_height*width, 
                         pGy+x*cell_width+ y*cell_height*width,
                         width,cell_width, cell_height, pSURFFea);
            pSURFFea += SURF_TYPE;
        }
    }

    pSURFFea -= SURF_LEN;
    //normalize use L2-Hys
    norm = 0;
    for(x=0; x<SURF_LEN; x++)
        norm += pSURFFea[x]*pSURFFea[x];
    norm = sqrt(norm);
    if(norm > 0)
    {
        for(x=0; x<SURF_LEN; x++)
        {
            val = pSURFFea[x]/norm;
            if(val > clip_val)
                val = clip_val;
            else if(val < -clip_val)
                val = -clip_val;
            pFea_temp[x] = val;
        }
    }
    else
    {
        for(x=0; x<SURF_LEN; x++)
            pFea_temp[x] = 0;
    }

    norm = 0;
    for(x=0; x<SURF_LEN; x++)
        norm += pFea_temp[x]*pFea_temp[x];
    norm = sqrt(norm);
    if(norm > 0)
    {
        for(x=0; x<SURF_LEN; x++)
        {
            val = pFea_temp[x]/norm;
            pSURFFea[x] = val * (1<<24);
        }
    }
    else
    {
        for(x=0; x<SURF_LEN; x++)
            pSURFFea[x] = 0;
    }



    rVal = SURF_LEN;
 EXIT:
    if(pGx) TMemFree(hMemBuf, pGx);
    if(pGy) TMemFree(hMemBuf, pGy);
    if(pFea_temp) TMemFree(hMemBuf, pFea_temp);
    return rVal;
}

#include "HogFea.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "tmem.h"

#if 0
//(0,0) is in  left-bottom

/*
                     sobel : use the nearest value when the index is out of range 
            |-1 0 1|      | 1  2  1| 
         Gx=|-2 0 2|   Gy=| 0  0  0|
            |-1 0 1|      |-1 -2 -1|
*/
static int __getGradient_sobel(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height);
/*
                     simple : use 0  when the index is out of range
                          | 1 | 
         Gx=|-1 0 1|   Gy=| 0 |
                          |-1 |
*/
static int __getGradient_simple(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height);

/*
    IsPi ::  1 -- The angle of gradient  is in (0-180)
             0 -- The angle of gradient  is in (0-360)
*/
static int __getCellHogFea(const int *pGrad_x, const int *pGrad_y, int widthStep, int cellWidth, int cellHeight, float  *pCellHist, const int DirBin, int IsPi);

static void HoG(THandle hMemBuf,unsigned char *pixels, float *params, int *img_size, double *dth_des, unsigned int grayscale);

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

static int __getGradient_sobel(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height)
{
    int w, h;
    const unsigned char *pSrc;
    int *pDst_x, *pDst_y;
    unsigned char x1,x2,x3, x11, x22, x33, y1, y2, y3, y11, y22, y33;

    if ((TNull == pGrayImg) || (TNull == pGradient_x) || (TNull == pGradient_y))
        return -1;
    width -= 1;
    height -= 1;

    // the first line
    w = h = 0;
    pSrc = pGrayImg;
    pDst_x = pGradient_x;
    pDst_y = pGradient_y;
    x1 = pSrc[0];
    x2 = pSrc[0];
    x3 = pSrc[widthStep_src];
    x11 = pSrc[1];
    x22 = pSrc[1];
    x33 = pSrc[widthStep_src+1];
    y1 = pSrc[widthStep_src];
    y2 = pSrc[widthStep_src];
    y3 = pSrc[widthStep_src+1];
    y11 = pSrc[0];
    y22 = pSrc[0];
    y33 = pSrc[1];
    pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
    pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
    for(w=1; w<width; w++)
    {
        x1 = pSrc[w-1];
        x2 = pSrc[w-1];
        x3 = pSrc[w-1 + widthStep_src];
        x11 = pSrc[w+1];
        x22 = pSrc[w+1];
        x33 = pSrc[w+1 + widthStep_src];	
        y1 = pSrc[w-1 + widthStep_src];
        y2 = pSrc[w + widthStep_src];
        y3 = pSrc[w+1 + widthStep_src];
        y11 = pSrc[w-1];
        y22 = pSrc[w];
        y33 = pSrc[w+1];
        pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
        pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
    } 
    x1 = pSrc[w-1];
    x2 = pSrc[w-1];
    x3 = pSrc[w-1 + widthStep_src];
    x11 = pSrc[w];
    x22 = pSrc[w];
    x33 = pSrc[w + widthStep_src];
    y1 = pSrc[w-1 + widthStep_src];
    y2 = pSrc[w + widthStep_src];
    y3 = pSrc[w + widthStep_src];
    y11 = pSrc[w-1];
    y22 = pSrc[w];
    y33 = pSrc[w];
    pGradient_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
    pGradient_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
    
    // the middle lines
    for(h=1; h<height; h++)
    {
        pDst_x += widthStep_dst;
        pDst_y += widthStep_dst;
        pSrc   += widthStep_src; 

        x1 = pSrc[w-widthStep_src];
        x2 = pSrc[w];
        x3 = pSrc[w + widthStep_src];
        x11 = pSrc[w+1 - widthStep_src];
        x22 = pSrc[w+1];
        x33 = pSrc[w+1 + widthStep_src];
	
        y1 = pSrc[w + widthStep_src];
        y2 = pSrc[w + widthStep_src];
        y3 = pSrc[w+1 + widthStep_src];
        y11 = pSrc[w - widthStep_src];
        y22 = pSrc[w - widthStep_src];
        y33 = pSrc[w+1 - widthStep_src];
        pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
        pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
        for(w=1; w<width; w++)
        {
            x1 = pSrc[w-1-widthStep_src];
            x2 = pSrc[w-1];
            x3 = pSrc[w-1 + widthStep_src];
            x11 = pSrc[w+1 - widthStep_src];
            x22 = pSrc[w+1];
            x33 = pSrc[w+1 + widthStep_src];
            
            y1 = pSrc[w-1 + widthStep_src];
            y2 = pSrc[w + widthStep_src];
            y3 = pSrc[w+1 + widthStep_src];
            y11 = pSrc[w-1 - widthStep_src];
            y22 = pSrc[w - widthStep_src];
            y33 = pSrc[w+1 - widthStep_src];
            pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
            pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
        }
        x1 = pSrc[w-1-widthStep_src];
        x2 = pSrc[w-1];
        x3 = pSrc[w-1 + widthStep_src];
        x11 = pSrc[w - widthStep_src];
        x22 = pSrc[w];
        x33 = pSrc[w + widthStep_src];
        
        y1 = pSrc[w-1 + widthStep_src];
        y2 = pSrc[w + widthStep_src];
        y3 = pSrc[w + widthStep_src];
        y11 = pSrc[w-1 - widthStep_src];
        y22 = pSrc[w - widthStep_src];
        y33 = pSrc[w - widthStep_src];
        pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
        pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
    }
    
    //the last line
    pDst_x += widthStep_dst;
    pDst_y += widthStep_dst;
    pSrc   += widthStep_src; 
    w=0; 
    x1 = pSrc[w-widthStep_src];
    x2 = pSrc[w];
    x3 = pSrc[w];
    x11 = pSrc[w+1 - widthStep_src];
    x22 = pSrc[w+1];
    x33 = pSrc[w+1];	
    y1 = pSrc[w];
    y2 = pSrc[w];
    y3 = pSrc[w+1];
    y11 = pSrc[w - widthStep_src];
    y22 = pSrc[w - widthStep_src];
    y33 = pSrc[w+1 - widthStep_src];
    pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
    pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
    for(w=1; w<width; w++)
    {
        x1 = pSrc[w-1-widthStep_src];
	x2 = pSrc[w-1];
	x3 = pSrc[w-1];
	x11 = pSrc[w+1 - widthStep_src];
	x22 = pSrc[w+1];
	x33 = pSrc[w+1];
	
	y1 = pSrc[w-1];
	y2 = pSrc[w];
	y3 = pSrc[w+1];
	y11 = pSrc[w-1 - widthStep_src];
	y22 = pSrc[w - widthStep_src];
	y33 = pSrc[w+1 - widthStep_src];
	pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
	pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);
    }
    x1 = pSrc[w-1-widthStep_src];
    x2 = pSrc[w-1];
    x3 = pSrc[w-1];
    x11 = pSrc[w - widthStep_src];
    x22 = pSrc[w];
    x33 = pSrc[w];
    
    y1 = pSrc[w-1];
    y2 = pSrc[w];
    y3 = pSrc[w];
    y11 = pSrc[w-1 - widthStep_src];
    y22 = pSrc[w - widthStep_src];
    y33 = pSrc[w - widthStep_src];
    pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
    pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);

    return 0;
}

int GetGradient(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height, GRADIENT_MODE mode)
{

  if ((TNull == pGrayImg) || (TNull == pGradient_x) || (TNull == pGradient_y))
        return -1;

    if (GRADIENT_SOBEL == mode)
        return __getGradient_sobel(pGrayImg, widthStep_src, pGradient_x, pGradient_y, widthStep_dst, width, height);
    else if(GRADIENT_SIMPLE == mode)
        return __getGradient_simple(pGrayImg, widthStep_src, pGradient_x, pGradient_y, widthStep_dst, width, height);
    else 
        return -1;
}

#if 1
int HogFea(THandle hMemBuf, unsigned char *grayImg, int width, int height, int *pHogFea)
{    
#define HOG_GAP 2   
    //initialize  para 
    const float pi = 3.141593f;
    const int nBins    = 9 ;    
    const int cell_size = 12;
    const int block_size = 2;
    const float clip_val = 0.2f;
    const int isPi = 1; //1: 0-180 range, 0: use 0-360

    int nFeaDim = 0;
    int cell_width = 0, cell_height = 0, hist_width=0, hist_height=0;
    double *cell_hist=TNull, *block=TNull;
    int dx=0, dy=0;
    float grad_mag=0.0f, grad_ori=0.0f;
    unsigned char *pImg;
    int x,y,i,j,k;

    float bin_size = (1+(0 == isPi))*pi/nBins;
    float fac_x1, fac_x2, fac_y1, fac_y2, fac_o1, fac_o2;
    float block_norm;
    int x1, x2, y1, y2, bin1, bin2, index=0, index_fea=0;
    
    if((TNull == grayImg)||(TNull == pHogFea))
    {
        nFeaDim = 0;
        goto EXIT;
    }
    cell_width = floor(width*1.0f/cell_size + 0.5f);
    cell_height = floor(height*1.0f/cell_size + 0.5f);
    nFeaDim = (cell_width-(block_size-1))*(cell_height-(block_size-1))*nBins*block_size*block_size;
    
    hist_width = cell_width+2;
    hist_height = cell_height + 2;
    cell_hist = (double*)TMemAlloc(hMemBuf, hist_width*hist_height*nBins*sizeof(double));
	block = (double*)TMemAlloc(hMemBuf, block_size*block_size*nBins*sizeof(double));

    if((TNull == cell_hist) || (TNull == block))
    {
        nFeaDim = 0;
        goto EXIT;
    }

    TMemSet(cell_hist, 0,hist_width*hist_height*nBins*sizeof(double));
	TMemSet(block, 0,block_size*block_size*nBins*sizeof(double));
    
    //get cell histgram first
    pImg = grayImg;
    for(y=0; y<height; y+=HOG_GAP)
    {
        for(x=0; x<width; x+=HOG_GAP)
        {
            ////////////////////////////////////////////////////////////
            //         simple : use 0  when the index is out of range //
            //                                 | 1 |                  //
            //                Gx=|-1 0 1|   Gy=| 0 |                  //
            //                                 |-1 |                  //
            ////////////////////////////////////////////////////////////
            //compute the dx and dy
            if(0==x)  
                dx = pImg[x+1];
            else if (width-1==x)
                dx = -pImg[x-1];
            else
                dx = pImg[x+1] - pImg[x-1];
            if(0==y)
                dy = -pImg[width+x];
            else if(height-1 == y)
                dy = pImg[-width+x];
            else
                dy = -pImg[width + x] + pImg[-width+x];
            
            grad_mag = (float)sqrt((float)(dx*dx + dy*dy));
            grad_ori = (float)atan2((float)dy,(float)dx);
            if(grad_ori < 0)
                grad_ori += (1+(0==isPi))*pi;
            
            printf("%d, %d, %f, %f\n", dx, dy, grad_mag, grad_ori);
            //use trilinearly interpolation
            bin1 = (int)floor(0.5f + grad_ori/bin_size) - 1;
            bin2 = bin1 + 1;
            
            x1   = (int)floor(0.5f + x*1.0f/cell_size);
            x2   = x1+1;
            y1   = (int)floor(0.5f + y*1.0f/cell_size);
            y2   = y1 + 1;

            fac_x2 = (x-(x1-0.5f)*cell_size + 0.5f)/cell_size;
            fac_x1 = 1.0f-fac_x2;
            fac_y2 = (y-(y1-0.5f)*cell_size + 0.5f)/cell_size;
            fac_y1 = 1.0f-fac_y2;
            fac_o2 = (grad_ori-(bin1+0.5f)*bin_size)/bin_size;
            fac_o1 = 1.0f-fac_o2;
            if (bin2==nBins)
            {
                bin2=0;
            }
            if (bin1<0)
            {
                bin1=nBins-1;
            }  
            printf("%f, %f , %f , %f, %f, %f\n",fac_x1, fac_x2, fac_y1, fac_y2, fac_o1, fac_o2);
            index = y1 * hist_width*nBins + x1*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y1*fac_x1*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
			index = y1 * hist_width*nBins + x1*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y1*fac_x1*fac_o2;
            printf("%f %d\n",cell_hist[index], index);
			index = y2 * hist_width*nBins + x1*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y2*fac_x1*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
			index = y2 * hist_width*nBins + x1*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y2*fac_x1*fac_o2;
            printf("%f %d\n",cell_hist[index], index);
            index = y1 * hist_width*nBins + x2*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y1*fac_x2*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
            index = y1 * hist_width*nBins + x2*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y1*fac_x2*fac_o2;
            printf("%f %d\n",cell_hist[index], index);
            index = y2 * hist_width*nBins + x2*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y2*fac_x2*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
            index = y2 * hist_width*nBins + x2*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y2*fac_x2*fac_o2;
            // if(cell_hist[index]<0)
            // printf("%d, %f, %f, %f\n ", index, cell_hist[index], grad_mag, fac_y2*fac_x2*fac_o2);
            printf("%f %d\n",cell_hist[index], index);
        }
        pImg += HOG_GAP*width;
    }

    // printf("%f, %f\n", cell_hist[hist_width*nBins + nBins + 1],block_norm);
    //get fea
     for(y=1; y<hist_height-block_size; y++)
     {
        for (x=1; x<hist_width-block_size; x++)
        {            
            block_norm = 0.0f;
            for (i=0; i<block_size; i++)
            {
                for(j=0; j<block_size; j++)
                {
                    for(k=0; k<nBins; k++)
                    {
						index = (y+i) * hist_width*nBins + (x+j)*nBins + k;
                        block_norm += cell_hist[index]*cell_hist[index];
                    }
                }
            }
    
            if (block_norm>0)
            {            
                for (i=0; i<block_size; i++)
                {
                    for(j=0; j<block_size; j++)
                    {
                        for(k=0; k<nBins; k++)
                        {
                            index = i*block_size*nBins + j* nBins + k;
                            block[index]=cell_hist[(y+i) * hist_width*nBins + (x+j)*nBins + k]/block_norm;
                            if (block[index]>clip_val) 
                                block[index]=clip_val;
                        }
                    }
                }
            }
            else
            {            
                for (i=0; i<block_size; i++)
                {
                    for(j=0; j<block_size; j++)
                    {
                        for(k=0; k<nBins; k++)
                        {                           
                            index = i*block_size*nBins + j* nBins + k;
                            block[index] = 0;
                        }
                    }
                }
            }
            
            block_norm=0.0f;
            for (i=0; i<block_size; i++)
            {
                for(j=0; j<block_size; j++)
                {
                    for(k=0; k<nBins; k++)
                    {
						index = i*block_size*nBins + j*nBins + k;
                        block_norm+=block[index]*block[index];
                    }
                }
            }
            
            block_norm=sqrt(block_norm);
            if (block_norm>0)
            {
                for (i=0; i<block_size; i++)
                {
                    for(j=0; j<block_size; j++)
                    {
                        for(k=0; k<nBins; k++)
                        {
                            index = i*block_size*nBins + j* nBins + k;
                            
                            pHogFea[index_fea]=(block[index]/block_norm)*(1<<24);
                            index_fea++;
                        }
                    }
                }
            }
            else
            {
                for (i=0; i<block_size; i++)
                {
                    for(j=0; j<block_size; j++)
                    {
                        for(k=0; k<nBins; k++)
                        {
                            pHogFea[index_fea]=0;
                            index_fea++;
                        }
                    }
                }
            }
        }
     }
 EXIT:
    if(cell_hist) TMemFree(hMemBuf, cell_hist);
    if(block) TMemFree(hMemBuf, block);
    return nFeaDim;
}

#else

int HogFea(THandle hMemBuf, unsigned char *grayImg, int width, int height, int *pHogFea)
{
    int feaDim = 0;
    double *dFea = TNull;
    float params[5];
    int img_size[2]={width, height};
    int i;

	int hist1 = 0;
    int hist2 = 0;
    int nb_bins    = 0 ;    
    int block_size = 0 ;
    
    params[0]=9;
    params[1]=12;
    params[2]=2;
    params[3]=0;
    params[4]=0.2f;

    hist1= (int)(2+ceil(-0.5f + img_size[0]/params[1]));
    hist2= (int)(2+ceil(-0.5f + img_size[1]/params[1]));
    nb_bins       = (int) params[0];    
    block_size    = (int) params[2];

    feaDim = (hist1-2-(block_size-1))*(hist2-2-(block_size-1))*nb_bins*block_size*block_size;

    dFea = (double*)TMemAlloc(hMemBuf, feaDim*sizeof(double));
	if(TNull == dFea)
		return -1;
    
    HoG(hMemBuf, grayImg,params, img_size, dFea,1);
    for(i=0; i<feaDim; i++)
    {
        pHogFea[i] =(int)( dFea[i]*(1<<24));
    }


	if(dFea)TMemFree(hMemBuf, dFea);
    return 0;
}


static void HoG(THandle hMemBuf, unsigned char *pixels, float *params, int *img_size, double *dth_des, unsigned int grayscale){
    
    const float pi = 3.141593f;
#define HOG_GAP (2)
    int nb_bins       = (int) params[0];
    float cwidth     =  params[1];
    int block_size    = (int) params[2];
    int orient        = (int) params[3];
    double clip_val   = params[4];
    
    int img_width  = img_size[1];
    int img_height = img_size[0];
    
    int hist1= (int)(2+ceil(-0.5f + img_height/cwidth));
    int hist2= (int)(2+ceil(-0.5f + img_width/cwidth));
    
    float bin_size = (1+(orient==1))*pi/nb_bins;
    
    int dx[3], dy[3];
	float grad_or, grad_mag, temp_mag;
    float Xc, Yc, Oc, block_norm;
    int x1, x2, y1, y2, bin1, bin2;
    int des_indx = 0, index_tmp;
    float fac_x1, fac_x2, fac_y1, fac_y2, fac_o1, fac_o2;
	int x, y, i, j, k;
    
	double *h = (double*)TMemAlloc(hMemBuf, hist1*hist2*nb_bins*sizeof(double));
	double *block = (double*)TMemAlloc(hMemBuf, block_size*block_size*nb_bins*sizeof(double));
	if((TNull == h) || (TNull == block))
	{
		if(h) TMemFree(hMemBuf, h);
		if(block) TMemFree(hMemBuf, block);
		return;
	}
	TMemSet(h, 0,hist1*hist2*nb_bins*sizeof(double));
	TMemSet(h, 0,block_size*block_size*nb_bins*sizeof(double));
    //vector<vector<vector<double> > > h(hist1, vector<vector<double> > (hist2, vector<double> (nb_bins, 0.0) ) );    
   // vector<vector<vector<double> > > block(block_size, vector<vector<double> > (block_size, vector<double> (nb_bins, 0.0) ) );
    
    //Calculate gradients (zero padding)
   // FILE *file = fopen("./Hig.txt", "w");
    
    for(y=0; y<img_height; y+=HOG_GAP) {
        for(x=0; x<img_width; x+=HOG_GAP) {
            if (grayscale == 1){
                /*
                if(x==0) dx[0] = pixels[y +(x+1)*img_height];
                else{
                    if (x==img_width-1) dx[0] = -pixels[y + (x-1)*img_height];
                    else dx[0] = pixels[y+(x+1)*img_height] - pixels[y + (x-1)*img_height];
                }
                if(y==0) dy[0] = -pixels[y+1+x*img_height];
                else{
                    if (y==img_height-1) dy[0] = pixels[y-1+x*img_height];
                    else dy[0] = -pixels[y+1+x*img_height] + pixels[y-1+x*img_height];
                    }
					*/
                if(x==0) dx[0] = pixels[y*img_width + (x+1)];
                else{
                    if (x==img_width-1) dx[0] = -pixels[y*img_width + (x-1)];
                    else dx[0] = pixels[y*img_width+(x+1)] - pixels[y*img_width + (x-1)];
                }
                if(y==0) dy[0] = -pixels[(y+1)*img_width+x];
                else{
                    if (y==img_height-1) dy[0] = pixels[(y-1)*img_width+x];
                    else dy[0] = -pixels[(y+1)*img_width+x] + pixels[(y-1)*img_width +x];
                }
				
            }
            else{
                if(x==0){
                    dx[0] = pixels[y +(x+1)*img_height];
                    dx[1] = pixels[y +(x+1)*img_height + img_height*img_width];
                    dx[2] = pixels[y +(x+1)*img_height + 2*img_height*img_width];                    
                }
                else{
                    if (x==img_width-1){
                        dx[0] = -pixels[y + (x-1)*img_height];                        
                        dx[1] = -pixels[y + (x-1)*img_height + img_height*img_width];
                        dx[2] = -pixels[y + (x-1)*img_height + 2*img_height*img_width];
                    }
                    else{
                        dx[0] = pixels[y+(x+1)*img_height] - pixels[y + (x-1)*img_height];
                        dx[1] = pixels[y+(x+1)*img_height + img_height*img_width] - pixels[y + (x-1)*img_height + img_height*img_width];
                        dx[2] = pixels[y+(x+1)*img_height + 2*img_height*img_width] - pixels[y + (x-1)*img_height + 2*img_height*img_width];
                        
                    }
                }
                if(y==0){
                    dy[0] = -pixels[y+1+x*img_height];
                    dy[1] = -pixels[y+1+x*img_height + img_height*img_width];
                    dy[2] = -pixels[y+1+x*img_height + 2*img_height*img_width];
                }
                else{
                    if (y==img_height-1){
                        dy[0] = pixels[y-1+x*img_height];
                        dy[1] = pixels[y-1+x*img_height + img_height*img_width];
                        dy[2] = pixels[y-1+x*img_height + 2*img_height*img_width];
                    }
                    else{
                        dy[0] = -pixels[y+1+x*img_height] + pixels[y-1+x*img_height];
                        dy[1] = -pixels[y+1+x*img_height + img_height*img_width] + pixels[y-1+x*img_height + img_height*img_width];
                        dy[2] = -pixels[y+1+x*img_height + 2*img_height*img_width] + pixels[y-1+x*img_height + 2*img_height*img_width];
                    }
                }
            }
            grad_mag = (float)sqrt((float)(dx[0]*dx[0] + dy[0]*dy[0]));
            grad_or= (float)atan2((float)dy[0], (float)dx[0]);
            
            if (grayscale == 0){
				unsigned int cli=1;
                temp_mag = grad_mag;
                for (cli=1;cli<3;++cli){
                    temp_mag= (float)sqrt((float)(dx[cli]*dx[cli] + dy[cli]*dy[cli]));
                    if (temp_mag>grad_mag){
                        grad_mag=temp_mag;
                        grad_or= (float)atan2((float)dy[cli], (float)dx[cli]);
                    }
                }
            }
            
            if (grad_or<0) grad_or+=pi + (orient==1) * pi;
            printf("%d, %d, %f, %f\n", dx[0], dy[0], grad_mag, grad_or);

            // trilinear interpolation
            
            bin1 = (int)floor(0.5 + grad_or/bin_size) - 1;
            bin2 = bin1 + 1;
            x1   = (int)floor(0.5+ x/cwidth);
            x2   = x1+1;
            y1   = (int)floor(0.5+ y/cwidth);
            y2   = y1 + 1;
            
            Xc = (x1+1-1.5f)*cwidth + 0.5f;
            Yc = (y1+1-1.5f)*cwidth + 0.5f;
            
            Oc = (bin1+1+1-1.5f)*bin_size;

            fac_x2 = (x-(x1-0.5f)*cell_size + 0.5f)/cell_size;
            fac_x1 = 1.0f-fac_x2;
            fac_y2 = (y-(y1-0.5f)*cell_size + 0.5f)/cell_size;
            fac_y1 = 1.0f-fac_y2;
            fac_o2 = (grad_ori-(bin1+0.5f)*bin_size)/bin_size;
            fac_o1 = 1.0f-fac_o2;
            if (bin2==nBins)
            {
                bin2=0;
            }
            if (bin1<0)
            {
                bin1=nBins-1;
            }  
            printf("%f, %f , %f , %f, %f, %f\n",fac_x1, fac_x2, fac_y1, fac_y2, fac_o1, fac_o2);
            index = y1 * hist_width*nBins + x1*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y1*fac_x1*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
			index = y1 * hist_width*nBins + x1*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y1*fac_x1*fac_o2;
            printf("%f %d\n",cell_hist[index], index);
			index = y2 * hist_width*nBins + x1*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y2*fac_x1*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
			index = y2 * hist_width*nBins + x1*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y2*fac_x1*fac_o2;
            printf("%f %d\n",cell_hist[index], index);
            index = y1 * hist_width*nBins + x2*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y1*fac_x2*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
            index = y1 * hist_width*nBins + x2*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y1*fac_x2*fac_o2;
            printf("%f %d\n",cell_hist[index], index);
            index = y2 * hist_width*nBins + x2*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y2*fac_x2*fac_o1;
            printf("%f %d\n",cell_hist[index], index);
            index = y2 * hist_width*nBins + x2*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y2*fac_x2*fac_o2;
            printf("%f %d\n",cell_hist[index], index);
            /*  
            if (bin2==nb_bins){
                bin2=0;
            }
            if (bin1<0){
                bin1=nb_bins-1;
            }            
           
            printf("%f, %f , %f , %f, %f, %f\n",(1.0f-((x+1-Xc)/cwidth)), ((x+1-Xc)/cwidth), (1.0f-((y+1-Yc)/cwidth)), ((y+1-Yc)/cwidth),(1.0f-((grad_or-Oc)/bin_size)), ((grad_or-Oc)/bin_size)  );
			index_tmp = y1 * hist2*nb_bins + x1*nb_bins + bin1;
            h[index_tmp]= h[index_tmp]+grad_mag*(1.0f-((x+1-Xc)/cwidth))*(1.0f-((y+1-Yc)/cwidth))*(1.0f-((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);
			index_tmp = y1 * hist2*nb_bins + x1*nb_bins + bin2;
            h[index_tmp]= h[index_tmp]+grad_mag*(1.0f-((x+1-Xc)/cwidth))*(1.0f-((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);
			index_tmp = y2 * hist2*nb_bins + x1*nb_bins + bin1;
            h[index_tmp]= h[index_tmp]+grad_mag*(1.0f-((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(1.0f-((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);
			index_tmp = y2 * hist2*nb_bins + x1*nb_bins + bin2;
            h[index_tmp]= h[index_tmp]+grad_mag*(1.0f-((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);
			index_tmp = y1 * hist2*nb_bins + x2*nb_bins + bin1;
            h[index_tmp]= h[index_tmp]+grad_mag*(((x+1-Xc)/cwidth))*(1.0f-((y+1-Yc)/cwidth))*(1.0f-((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);
			index_tmp = y1 * hist2*nb_bins + x2*nb_bins + bin2;
            h[index_tmp]= h[index_tmp]+grad_mag*(((x+1-Xc)/cwidth))*(1.0f-((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);
			index_tmp = y2 * hist2*nb_bins + x2*nb_bins + bin1;
            h[index_tmp]= h[index_tmp]+grad_mag*(((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(1.0f-((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);
			index_tmp = y2 * hist2*nb_bins + x2*nb_bins + bin2;
            h[index_tmp]= h[index_tmp]+grad_mag*(((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
            printf("%f %d\n",h[index_tmp], index_tmp);*/
        }
    }
    
    
    
    //Block normalization
    
    for(x=1; x<hist2-block_size; x++){
        for (y=1; y<hist1-block_size; y++){
            
            block_norm=0;
            for (i=0; i<block_size; i++){
                for(j=0; j<block_size; j++){
                    for(k=0; k<nb_bins; k++){
						index_tmp = (y+i) * hist2*nb_bins + (x+j)*nb_bins + k;
                        block_norm+=h[index_tmp]*h[index_tmp];
                    }
                }
            }
            
            block_norm=sqrt(block_norm);
            for (i=0; i<block_size; i++){
                for(j=0; j<block_size; j++){
                    for(k=0; k<nb_bins; k++){
                        if (block_norm>0){
							index_tmp = i*block_size*nb_bins + j* nb_bins + k;
                            block[index_tmp]=h[(y+i) * hist2*nb_bins + (x+j)*nb_bins + k]/block_norm;
                            if (block[index_tmp]>clip_val) block[index_tmp]=clip_val;
                        }
                    }
                }
            }
            
            block_norm=0;
            for (i=0; i<block_size; i++){
                for(j=0; j<block_size; j++){
                    for(k=0; k<nb_bins; k++){
						index_tmp = i*block_size*nb_bins + j* nb_bins + k;
                        block_norm+=block[index_tmp]*block[index_tmp];
                    }
                }
            }
            
            block_norm=sqrt(block_norm);
            for (i=0; i<block_size; i++){
                for(j=0; j<block_size; j++){
                    for(k=0; k<nb_bins; k++){
						index_tmp = i*block_size*nb_bins + j* nb_bins + k;
                        if (block_norm>0) dth_des[des_indx]=block[index_tmp]/block_norm;
                        else dth_des[des_indx]=0.0;
                        des_indx++;
                    }
                }
            }
        }
    }

	if(h) TMemFree(hMemBuf, h);
	if(block) TMemFree(hMemBuf, block);
	return;
    //fclose(file);
}
#endif

#endif

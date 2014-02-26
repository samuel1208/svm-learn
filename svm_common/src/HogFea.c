#include "HogFea.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "tmem.h"


int GetHOGDim(int width, int height)
{
    return ((width/12)-1)*((height/12)-1)*4*9;
}

static int _getCellHist(unsigned char *grayImg, int widthStep, int width, int height,
                        double *cell_hist, int hist_width, int hist_height, 
                        const int cell_size, const int nBins, const int isPi );


static int _getCellHist(unsigned char *grayImg, int widthStep, int width, int height,
                        double *cell_hist, int hist_width, int hist_height, 
                        const int cell_size, const int nBins, const int isPi )
{
#define HOG_GAP (2)   // speedup  
    const float pi = 3.141593f;   
    unsigned char *pImg;
    int x, y, dx, dy;     
    float grad_mag=0.0f, grad_ori=0.0f;
    float fac_x1, fac_x2, fac_y1, fac_y2, fac_o1, fac_o2;
    int x1, x2, y1, y2, bin1, bin2,  index;

    float bin_size = (1+(0 == isPi))*pi/nBins;

    if((TNull == grayImg)||(TNull == cell_hist))
        return -1;

    TMemSet(cell_hist, 0,hist_width*hist_height*nBins*sizeof(double));

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

            index = y1 * hist_width*nBins + x1*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y1*fac_x1*fac_o1;
			index = y1 * hist_width*nBins + x1*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y1*fac_x1*fac_o2;
			index = y2 * hist_width*nBins + x1*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y2*fac_x1*fac_o1;
			index = y2 * hist_width*nBins + x1*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y2*fac_x1*fac_o2;
            index = y1 * hist_width*nBins + x2*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y1*fac_x2*fac_o1;
            index = y1 * hist_width*nBins + x2*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y1*fac_x2*fac_o2;
            index = y2 * hist_width*nBins + x2*nBins + bin1;
            cell_hist[index] += grad_mag*fac_y2*fac_x2*fac_o1;
            index = y2 * hist_width*nBins + x2*nBins + bin2;
            cell_hist[index] += grad_mag*fac_y2*fac_x2*fac_o2;
        }
        pImg += HOG_GAP*widthStep;
    }    
    return 0;
}

int HogFea(THandle hMemBuf, unsigned char *grayImg, int widthStep, int width, int height, int *pHogFea)
{    
 
    //initialize  para 
    const int nBins    = 9 ;    
    const int cell_size = 12;
    const int block_size = 2;
    const float clip_val = 0.2f;
    const int   isPi = 1; //1: 0-180 range, 0: use 0-360
    int x,y,i,j,k;

    int nFeaDim = 0;
    int cell_width = 0, cell_height = 0, hist_width=0, hist_height=0;
    double *cell_hist=TNull, *block=TNull; 
   
    float block_norm;
    int  index=0, index_fea=0;
    
    if((TNull == grayImg)||(TNull == pHogFea))
    {
        nFeaDim = 0;
        goto EXIT;
    }
    cell_width = floor(width*1.0f/cell_size + 0.5f);
    cell_height = floor(height*1.0f/cell_size + 0.5f);
    nFeaDim = (cell_width-(block_size-1))*(cell_height-(block_size-1))*nBins*block_size*block_size;
    
    //这里加2是为了后面的插值
    hist_width = cell_width+2;
    hist_height = cell_height + 2;
    cell_hist = (double*)TMemAlloc(hMemBuf, hist_width*hist_height*nBins*sizeof(double));
	block = (double*)TMemAlloc(hMemBuf, block_size*block_size*nBins*sizeof(double));

    if((TNull == cell_hist) || (TNull == block))
    {
        nFeaDim = 0;
        goto EXIT;
    }
    
    //get cell histgram first, only support trilinearly interpolation
    if(0 != _getCellHist(grayImg, widthStep, width, height, 
                         cell_hist, hist_width, hist_height,
                         cell_size, nBins, isPi))
    {
        nFeaDim = 0;
        goto EXIT;
    }

    //由于前面加2进行插值， 所以这里要跳过第一个和最后一个
    //get fea -- normalize with L2-Hys
    TMemSet(block, 0,block_size*block_size*nBins*sizeof(double));
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
            block_norm = sqrt(block_norm);
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

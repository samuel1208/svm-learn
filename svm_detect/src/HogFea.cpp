#include "HogFea.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <vector>


#define MNULL 0

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

static void HoG(double *pixels, double *params, int *img_size, double *dth_des, unsigned int grayscale);

static int __getGradient_simple(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height)
{
    int w, h;
    const unsigned char *pSrc;
    int *pDst_x, *pDst_y;
    unsigned char x1,y1 , x2 , y2;

    if ((MNULL == pGrayImg) || (MNULL == pGradient_x) || (MNULL == pGradient_y))
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

    if ((MNULL == pGrayImg) || (MNULL == pGradient_x) || (MNULL == pGradient_y))
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
    y22 = pSrc[1];
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
        y22 = pSrc[w+1];
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
    y22 = pSrc[w];
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
        y22 = pSrc[w+1 - widthStep_src];
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
            y22 = pSrc[w+1 - widthStep_src];
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
        y22 = pSrc[w - widthStep_src];
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
    y22 = pSrc[w+1 - widthStep_src];
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
	y22 = pSrc[w+1 - widthStep_src];
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
    y22 = pSrc[w - widthStep_src];
    pDst_x[w] = (x11+2*x22+x33) - (x1+2*x2+x3);
    pDst_y[w] = -(y11+2*y22+y33) + (y1+2*y2+y3);

    return 0;
}

int GetGradient(const unsigned char* pGrayImg, int widthStep_src, int *pGradient_x, int *pGradient_y, int widthStep_dst, int width, int height, GRADIENT_MODE mode)
{

  if ((MNULL == pGrayImg) || (MNULL == pGradient_x) || (MNULL == pGradient_y))
        return -1;

    if (GRADIENT_SOBEL == mode)
        return __getGradient_sobel(pGrayImg, widthStep_src, pGradient_x, pGradient_y, widthStep_dst, width, height);
    else if(GRADIENT_SIMPLE == mode)
        return __getGradient_simple(pGrayImg, widthStep_src, pGradient_x, pGradient_y, widthStep_dst, width, height);
    else 
        return -1;
}

using namespace std;   

int HogFea(unsigned char *grayImg, int width, int height, int *pHogFea)
{
    int feaDim = 0;
    double *pPixel = (double*)malloc(width*height*sizeof(double));
    double *dPtr = MNULL;
    double *dFea = MNULL;
    double params[5];
    unsigned char *cPtr;
    int img_size[2]={width, height};
    int i,j;

    
    params[0]=9;
    params[1]=8;
    params[2]=2;
    params[3]=1;
    params[4]=0.2;

    int hist1= 2+ceil(-0.5 + img_size[0]/params[1]);
    int hist2= 2+ceil(-0.5 + img_size[1]/params[1]);
    int nb_bins       = (int) params[0];    
    int block_size    = (int) params[2];
    feaDim = (hist1-2-(block_size-1))*(hist2-2-(block_size-1))*nb_bins*block_size*block_size;

    dFea = (double*)malloc(feaDim*sizeof(double));

    
    for(i=0; i<height; i++)
    {
        dPtr = pPixel + i*width;
	cPtr = grayImg + i*width;
	for(j=0; j<width; j++)
	{
	  dPtr[j] = cPtr[j];	       
	}
    }
    HoG(pPixel,params, img_size, dFea,1);
    for(i=0; i<feaDim; i++)
    {
        pHogFea[i] =(int)( dFea[i]*(1<<24));
    }
    return 0;
}
static void HoG(double *pixels, double *params, int *img_size, double *dth_des, unsigned int grayscale){
    
    const float pi = 3.1415926536;
    
    int nb_bins       = (int) params[0];
    double cwidth     =  params[1];
    int block_size    = (int) params[2];
    int orient        = (int) params[3];
    double clip_val   = params[4];
    
    int img_width  = img_size[1];
    int img_height = img_size[0];
    
    int hist1= 2+ceil(-0.5 + img_height/cwidth);
    int hist2= 2+ceil(-0.5 + img_width/cwidth);
    
    double bin_size = (1+(orient==1))*pi/nb_bins;
    
    float dx[3], dy[3], grad_or, grad_mag, temp_mag;
    float Xc, Yc, Oc, block_norm;
    int x1, x2, y1, y2, bin1, bin2;
    int des_indx = 0;
    
    vector<vector<vector<double> > > h(hist1, vector<vector<double> > (hist2, vector<double> (nb_bins, 0.0) ) );    
    vector<vector<vector<double> > > block(block_size, vector<vector<double> > (block_size, vector<double> (nb_bins, 0.0) ) );
    
    //Calculate gradients (zero padding)
   // FILE *file = fopen("./Hig.txt", "w");
    
    for(unsigned int y=0; y<img_height; y++) {
        for(unsigned int x=0; x<img_width; x++) {
            if (grayscale == 1){
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
					/*
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
				*/
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
            //fprintf(file, "%d, %d,", (int)dx[0], (int)dy[0]);
            grad_mag = sqrt(dx[0]*dx[0] + dy[0]*dy[0]);
            grad_or= atan2(dy[0], dx[0]);
            
            if (grayscale == 0){
                temp_mag = grad_mag;
                for (unsigned int cli=1;cli<3;++cli){
                    temp_mag= sqrt(dx[cli]*dx[cli] + dy[cli]*dy[cli]);
                    if (temp_mag>grad_mag){
                        grad_mag=temp_mag;
                        grad_or= atan2(dy[cli], dx[cli]);
                    }
                }
            }
            
            if (grad_or<0) grad_or+=pi + (orient==1) * pi;

            // trilinear interpolation
            
            bin1 = (int)floor(0.5 + grad_or/bin_size) - 1;
            bin2 = bin1 + 1;
            x1   = (int)floor(0.5+ x/cwidth);
            x2   = x1+1;
            y1   = (int)floor(0.5+ y/cwidth);
            y2   = y1 + 1;
            
            Xc = (x1+1-1.5)*cwidth + 0.5;
            Yc = (y1+1-1.5)*cwidth + 0.5;
            
            Oc = (bin1+1+1-1.5)*bin_size;
            
            if (bin2==nb_bins){
                bin2=0;
            }
            if (bin1<0){
                bin1=nb_bins-1;
            }            
           
            h[y1][x1][bin1]= h[y1][x1][bin1]+grad_mag*(1-((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
            h[y1][x1][bin2]= h[y1][x1][bin2]+grad_mag*(1-((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
            h[y2][x1][bin1]= h[y2][x1][bin1]+grad_mag*(1-((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
            h[y2][x1][bin2]= h[y2][x1][bin2]+grad_mag*(1-((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
            h[y1][x2][bin1]= h[y1][x2][bin1]+grad_mag*(((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
            h[y1][x2][bin2]= h[y1][x2][bin2]+grad_mag*(((x+1-Xc)/cwidth))*(1-((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
            h[y2][x2][bin1]= h[y2][x2][bin1]+grad_mag*(((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(1-((grad_or-Oc)/bin_size));
            h[y2][x2][bin2]= h[y2][x2][bin2]+grad_mag*(((x+1-Xc)/cwidth))*(((y+1-Yc)/cwidth))*(((grad_or-Oc)/bin_size));
        }
    }
    
    
    
    //Block normalization
    
    for(unsigned int x=1; x<hist2-block_size; x++){
        for (unsigned int y=1; y<hist1-block_size; y++){
            
            block_norm=0;
            for (unsigned int i=0; i<block_size; i++){
                for(unsigned int j=0; j<block_size; j++){
                    for(unsigned int k=0; k<nb_bins; k++){
                        block_norm+=h[y+i][x+j][k]*h[y+i][x+j][k];
                    }
                }
            }
            
            block_norm=sqrt(block_norm);
            for (unsigned int i=0; i<block_size; i++){
                for(unsigned int j=0; j<block_size; j++){
                    for(unsigned int k=0; k<nb_bins; k++){
                        if (block_norm>0){
                            block[i][j][k]=h[y+i][x+j][k]/block_norm;
                            if (block[i][j][k]>clip_val) block[i][j][k]=clip_val;
                        }
                    }
                }
            }
            
            block_norm=0;
            for (unsigned int i=0; i<block_size; i++){
                for(unsigned int j=0; j<block_size; j++){
                    for(unsigned int k=0; k<nb_bins; k++){
                        block_norm+=block[i][j][k]*block[i][j][k];
                    }
                }
            }
            
            block_norm=sqrt(block_norm);
            for (unsigned int i=0; i<block_size; i++){
                for(unsigned int j=0; j<block_size; j++){
                    for(unsigned int k=0; k<nb_bins; k++){
                        if (block_norm>0) dth_des[des_indx]=block[i][j][k]/block_norm;
                        else dth_des[des_indx]=0.0;
                        des_indx++;
                    }
                }
            }
        }
    }

    //fclose(file);
}

/*
    IsPi ::  1 -- The angle of gradient  is in (0-180)
             0 -- The angle of gradient  is in (0-360)
*/
/*
static int __getCellHogFea(const int *pGrad_x, const int *pGrad_y, int widthStep, int cellWidth, int cellHeight, float  *pCellHist, const int DirBin, int IsPi)
{
    int x, y;
    float pi = 3.1415926536f, period = 0;
    const int *pSrc_x, *pSrc_y;
    int val_x, val_y, index;
    float grad_mag, grad_ori;
    double bin_gap;

    if( (MNULL == pGrad_x) || (MNULL == pGrad_y) || (MNULL == pCellHist))
        return -1;
    
    if(0 == IsPi)
    {
        period = pi * 2;
        bin_gap = pi / DirBin;
    }
    else 
    {
        period = pi;
        bin_gap = pi / DirBin;
    }

    memset(pCellHist, 0, sizeof(*pCellHist)* DirBin);
    pSrc_x = pGrad_x;
    pSrc_y = pGrad_y;
    for(y=0; y<cellHeight; y++)
    {
        for(x=0; x<cellWidth; x++)
        {
            val_x = pSrc_x[x];
            val_y = pSrc_y[x];
            grad_mag = sqrt(val_x*val_x + val_y*val_y);
            grad_ori = atan2(val_y, val_x);  
            if(grad_ori <0)
                grad_ori += period;
            index = (int)(grad_ori / bin_gap);
            pCellHist[index] += grad_mag;            
        }
        pSrc_x += widthStep;
        pSrc_y += widthStep;
    }
    return 0;
}

int HogFea(const int *pGradient_x, const int *pGradient_y, int widthStep, int width, int height, int *pHogFea, int feaDim)
{
#define CELL_SIZE  8
#define BLOCK_SIZE 2
#define DIR_BIN    9
    int rval = 0;
    int cellNum_x = width/CELL_SIZE;
    int cellNum_y = height/CELL_SIZE;
    float *pCellHist = MNULL;
    int x, y;
    const int *pSrc_x, *pSrc_y;

    // The feature must <= feaDim
    if(  
         (cellNum_x<BLOCK_SIZE) || (cellNum_y<BLOCK_SIZE) 
      || (cellNum_x-1) * (cellNum_y-1) * DIR_BIN * BLOCK_SIZE * BLOCK_SIZE > feaDim
      )
    {
        rval = -1;
        goto EXIT;
    }

    pCellHist = (float *)malloc(cellNum_x*cellNum_y*DIR_BIN*sizeof(*pCellHist));
    if ( 
		(MNULL == pGradient_x)||(MNULL == pGradient_y)
	  ||(MNULL == pHogFea) || (MNULL == pCellHist)
      ||(MNULL == pCellHist)
	   )
    {
        rval = -1;
        goto EXIT;
    }


    pSrc_x = pGradient_x;
    pSrc_y = pGradient_y;
    for(y=0; y<cellNum_y; y++)
    {
        pSrc_x = pGradient_x + y*cellNum_y*widthStep;
        pSrc_y = pGradient_y + y*cellNum_y*widthStep;
        for(x=0; x<cellNum_x; x++)
        {            
            if(0 != __getCellHogFea(pSrc_x, pSrc_y, widthStep, CELL_SIZE, CELL_SIZE, pCellHist, DIR_BIN, 0))
            {
                rval =-1;
                goto EXIT;
            }
            pSrc_x += cellNum_x;
            pSrc_y += cellNum_x;
        }        
        
    }
    
 EXIT: 
    if(pCellHist) free(pCellHist);   
    return rval;
}
*/

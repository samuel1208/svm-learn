

#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "WanHuaLinFea.h"
#include "HogFea.h"
#include "LBP_Fea.h"
#include "svm_feature.h"
#include "SURFDescriptor.h"
//#define SHOW_IMG

#ifdef SHOW_IMG
static int checkGradientImg(int *pGrad_x, int *pGrad_y, int *pGrad, unsigned char *pDst, int width, int height)
{
    int w, h, min=1000000, max = 0, val;
    unsigned char *ptr;
    FILE *file = fopen("./simple.txt","w");
    for(h=0; h<height; h++)
    {
        for(w=0; w<width; w++)
        {
            fprintf(file, "%d, %d,", pGrad_x[h*width+w], pGrad_y[h*width+w]);
            val = sqrt((pGrad_x[h*width+w]*pGrad_x[h*width+w]) + (pGrad_y[h*width+w]*pGrad_y[h*width+w]));
            pGrad[h*width+w] = val;
            if(val >max)
                max = val;
            if(val<min)
                min = val;
        }
    }
    fclose(file);
    for(h=0; h<height; h++)
    {
        ptr = pDst + h*width;
        for(w=0; w<width; w++)
        {
            val = pGrad[h*width+w];
            ptr[w] = 255*(val-min)/(max-min);
        }
    }
    return 0;
}
#endif

void usage()
{
    printf("\n******************************************************\n");
    printf("usage:\n");
    printf("\textract_feature    base_w base_h feature label infile  outfile\n");
    printf("\t\tSupported Fea: [surf, lbp8, lbp16, wan, hog] \n");
    printf("\tPara label : 1 or 0 for  two-cass\n");
    printf("\tPara infile   : include the absolut path for each image\n");
    printf("\tPara outfile  : the absolut path of output file\n");
    printf("******************************************************\n\n");
    
}
int main(int argc, char **argv)
{
    FILE *file = NULL, *file_image = NULL, *file_save=NULL;
    char image_path_name[1024] = {0}, save_path[1024] = {0}; 
    int label = 0;

    IplImage  *src_img    = NULL;
    IplImage  *__src_img = NULL;
    IplImage  *resize_img = 0;
    IplImage  *hsl_img    = 0;
    IplImage  *gray_img   = 0;
    IplImage  *gradient_img = 0;
    int *pGradient_x=NULL, *pGradient_y=NULL, *pGradient=NULL;
    
    int fea_used_num = 0;
    int maxFeaNum = 0;
    int *pFea_tmp, *pFea =0; 
    int feaUsed = -1;
    int i,img_num=0;

    int IMG_WIDTH = 0;
    int IMG_HEIGHT = 1;
    int feaDim = 0;

    if (argc < 2)
    {
        usage();
        goto EXIT;
    }

    if (0 == strcmp(argv[3], "surf"))
        feaUsed = FEAT_SURF;
    else if (0 == strcmp(argv[3], "wan"))
        feaUsed = FEAT_WAN_COLOR;
    else if (0 == strcmp(argv[3], "hog"))
        feaUsed = FEAT_HOG;
    else if (0 == strcmp(argv[3], "lbp8"))
        feaUsed = FEAT_LBP_8;
    else if (0 == strcmp(argv[3], "lbp16"))
        feaUsed = FEAT_LBP_16;
    
    IMG_WIDTH = atoi(argv[1]);
    IMG_HEIGHT = atoi(argv[2]);
    maxFeaNum += GetWANDim();
    maxFeaNum += GetHOGDim(IMG_WIDTH, IMG_HEIGHT) ;
    maxFeaNum += GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y);
    maxFeaNum += GetSURFDim();
    pFea = (int *)malloc(maxFeaNum * sizeof(int));

    resize_img = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT),8,3);
    hsl_img    = cvCreateImage(cvSize(resize_img->width, resize_img->height), 8,3);
    gray_img   = cvCreateImage(cvSize(resize_img->width, resize_img->height), 8,1);
    gradient_img=cvCreateImage(cvSize(resize_img->width, resize_img->height),8 ,1);

    pGradient_x = (int *)malloc(resize_img->width*resize_img->height* sizeof(int));
    pGradient_y = (int *)malloc(resize_img->width*resize_img->height* sizeof(int));
    pGradient   = (int *)malloc(resize_img->width*resize_img->height* sizeof(int));

    if((NULL == pFea) || (NULL == hsl_img) || (NULL == resize_img)
       ||(NULL == pGradient_x) || (NULL == pGradient_y) || (NULL == pGradient))
    {
        printf("ERROR :: No Memory\n");
        goto EXIT;
    }
    
    if(argc < 6)
    {
        usage();
        goto EXIT;
    }

    label = atoi(argv[4]);
    printf("The label of this processing is %d\n", label);
    sprintf(save_path, "%s", argv[6]);

    file = fopen(argv[5], "r");
    if(NULL == file)
    {
        printf("ERROR :: The input file not exist\n");
        goto EXIT;
    }

    file_save = fopen(save_path, "w");
    if(NULL == file_save)
    {
        printf("ERROR :: The save file not exist\n");
        goto EXIT;
    }

    fea_used_num = 0;
    printf("---------------------------------------------\n");
    printf("The feature info: \n");
    if(feaUsed & FEAT_WAN_COLOR)
    {
        feaDim = GetWANDim();
        printf("\t WanHuaLin dim: used %d\n", feaDim);
        fea_used_num += feaDim;
    }
    if(feaUsed & FEAT_HOG)
    {
        feaDim = GetHOGDim(IMG_WIDTH, IMG_HEIGHT);
        printf("\t HOG dim      : used %d\n", feaDim);
        fea_used_num += feaDim;
    }
    if(feaUsed & FEAT_LBP_8)
    {
        feaDim = GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y);
        printf("\t LBP_8 dim    : used %d\n", feaDim);
        fea_used_num += feaDim;
    }
    if(feaUsed & FEAT_LBP_16)
    {
        feaDim = GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y);
        printf("\t LBP_16 dim   : used %d\n", feaDim);
        fea_used_num += feaDim;
    }
    if(feaUsed & FEAT_SURF)
    {
        feaDim = GetSURFDim();
        printf("\t SURF  dim    : used %d\n", feaDim);
        fea_used_num += feaDim;
    }
    printf("\t Total Dim    :      %d\n", fea_used_num);
    printf("---------------------------------------------\n");

#ifdef SHOW_IMG
    cvNamedWindow("test", 0);
#endif

    while(!feof(file))
    {
        memset(image_path_name, 0, 1024);
        fgets(image_path_name, 1024,file);
        //clean the last character "\n"
		if((strlen(image_path_name)>1)&&('\n'==image_path_name[strlen(image_path_name)-1]))
            image_path_name[strlen(image_path_name)-1]=0;
        src_img = NULL; 
        src_img =  (IplImage*)cvLoadImage(image_path_name,1);
        if(NULL == src_img)
        {
            printf("ERROR :: Can't open this image : %s\n", image_path_name);
            continue;
        }        
        __src_img = cvCreateImage(cvSize(src_img->width, src_img->height), 8,3);
        if(src_img->nChannels == 1)
            cvCvtColor(src_img, __src_img, CV_GRAY2RGB);
        else
            memcpy(__src_img->imageData, src_img->imageData, src_img->widthStep*src_img->height);        
            
        if((resize_img->width != src_img->width) || (resize_img->height != src_img->height))
            cvResize(__src_img, resize_img, CV_INTER_NN);
        else
            memcpy(resize_img->imageData, __src_img->imageData, resize_img->width*resize_img->height*3);

        cvCvtColor(resize_img, gray_img, CV_RGB2GRAY);
       

        pFea_tmp = pFea;
        feaDim = GetWANDim();
        if(feaUsed & FEAT_WAN_COLOR)
        {
                    // extract WanHuaLin feature
            if(0 != RGBtoHSL(resize_img->imageData, hsl_img->imageData,resize_img->height,resize_img->width,resize_img->widthStep, hsl_img->widthStep))
            {
                printf("ERROR :: Error occured in RGB2HSL\n ");
                continue;
            } 

            if(0 != WanHuaLinColorFea(hsl_img->imageData, hsl_img->widthStep, IMG_WIDTH, IMG_HEIGHT,pFea))
            {
                printf("ERROR :: Error occured in extracting WanHuaLin feature\n ");
                continue;
            }
            pFea_tmp += feaDim;
        }

        // extract HOG feature	
        feaDim = GetHOGDim(IMG_WIDTH, IMG_HEIGHT);
        if(feaUsed & FEAT_HOG)
        {
            if(feaDim != HogFea(NULL, gray_img->imageData, gray_img->widthStep,IMG_WIDTH, IMG_HEIGHT, pFea_tmp))
            {
                printf("ERROR :: Error occured in extracting HOG feature\n ");
                continue;
            }
            pFea_tmp += feaDim;
        }

        
        feaDim = GetLBPDim(16, LBP_GRID_X, LBP_GRID_Y);
        if(feaUsed & FEAT_LBP_16)
        {
            if(feaDim != LBPH_Fea(NULL, gray_img->imageData, gray_img->widthStep, IMG_WIDTH, IMG_HEIGHT,
                                      2, 16, LBP_GRID_X, LBP_GRID_Y, pFea_tmp))
            {
                printf("ERROR :: Error occured in extracting LBP feature\n ");
                continue;   
            } 
            pFea_tmp += feaDim;
        }

        feaDim = GetLBPDim(8, LBP_GRID_X, LBP_GRID_Y);
        if(feaUsed & FEAT_LBP_8)
        {
            if(feaDim != LBPH_Fea(NULL, gray_img->imageData, gray_img->widthStep,
                                  IMG_WIDTH, IMG_HEIGHT,
                                  1, 8, LBP_GRID_X, LBP_GRID_Y, pFea_tmp))
            {
                printf("ERROR :: Error occured in extracting LBP feature\n ");
                continue;   
            } 
            pFea_tmp += feaDim;
        }

        
        feaDim = GetSURFDim();
        if(feaUsed & FEAT_SURF)
        {
            if(feaDim != SURFFea(NULL, gray_img->imageData, gray_img->widthStep,IMG_WIDTH, IMG_HEIGHT, pFea_tmp))
            {
                printf("ERROR :: Error occured in extracting SURF feature\n ");
                continue;
            }
            pFea_tmp += feaDim;
        }
		//{
		//	FILE *tmp = fopen("../bin/tmp.txt","wb");
		//	fwrite(hsl_img->imageData, 1,hsl_img->widthStep*hsl_img->height, tmp );
		//	fwrite(gray_img->imageData, 1,gray_img->widthStep*gray_img->height, tmp );
		//	fclose(tmp);
		//}
        
        //  if(0 != GetGradient(gray_img->imageData, gray_img->widthStep,
        //                    pGradient_x, pGradient_y,gray_img->widthStep,
        //                    gray_img->width, gray_img->height, GRADIENT_SIMPLE))
        //  {
        //     printf("ERROR :: Error occured in GetGradient\n");
        //     continue;
        // }
        
        //save to file
        fprintf(file_save, "%d ", label);
        for(i=0; i<fea_used_num; i++)
        {
            fprintf(file_save, "%d:%d ", i+1, pFea[i]);
        }
        fprintf(file_save,"\n");
        img_num ++;

#ifdef SHOW_IMG
        cvShowImage("test", resize_img);
        cvWaitKey(0);
        cvShowImage("test", hsl_img);
        cvWaitKey(0);
        cvShowImage("test", gray_img);
        cvWaitKey(0);
        /* checkGradientImg(pGradient_x, pGradient_y, pGradient,(unsigned char*)(gradient_img->imageData),IMG_WIDTH, IMG_HEIGHT); */
        /* cvShowImage("test", gradient_img); */
        /* cvWaitKey(0); */
#endif

        cvReleaseImage(&src_img);
        cvReleaseImage(&__src_img);
        src_img = NULL;        
    }

    printf("get %d images feature\n", img_num);
#ifdef SHOW_IMG
    cvDestroyWindow("test");
#endif

 EXIT:
    if(file)          fclose(file);
    if(file_save)     fclose(file_save);
    if(hsl_img)       cvReleaseImage(&hsl_img);
    if(resize_img)    cvReleaseImage(&resize_img);
    if(gray_img)      cvReleaseImage(&gray_img);
    if(gradient_img)  cvReleaseImage(&gradient_img);
    if(pFea)          free(pFea);
    if(pGradient_x)   free(pGradient_x);
    if(pGradient_y)   free(pGradient_y);
    if(pGradient)     free(pGradient);
    return 0;
}

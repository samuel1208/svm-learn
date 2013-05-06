

#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "WanHuaLinFea.h"
#include "HogFea.h"

#define IMG_WIDTH  48
#define IMG_HEIGHT 48

#define HOG_DIM  (((IMG_WIDTH/12)-1)*((IMG_WIDTH/12)-1)*4*9)
#define WAN_DIM  (0)//(73)
#define FEA_DIM  (HOG_DIM+WAN_DIM)
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

int main(int argc, char **argv)
{
    FILE *file = NULL, *file_image = NULL, *file_save=NULL;
    char image_path_name[1024] = {0}, save_path[1024] = {0}; 
    int label = 0;

    IplImage  *src_img    = NULL;
    IplImage  *__src_img = NULL;
    IplImage  *resize_img = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT), 8,3);
    IplImage  *hsl_img    = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT), 8,3);
    IplImage  *gray_img   = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT), 8,1);
    IplImage  *gradient_img = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT),8 ,1);
    int *pGradient_x=NULL, *pGradient_y=NULL, *pGradient=NULL;
    int *pFea = (int *)malloc(FEA_DIM * sizeof(int));
    int i,img_num=0;

    pGradient_x = (int *)malloc(IMG_WIDTH*IMG_HEIGHT* sizeof(int));
    pGradient_y = (int *)malloc(IMG_WIDTH*IMG_HEIGHT* sizeof(int));
    pGradient   = (int *)malloc(IMG_WIDTH*IMG_HEIGHT* sizeof(int));

    if((NULL == pFea) || (NULL == hsl_img) || (NULL == resize_img)
       ||(NULL == pGradient_x) || (NULL == pGradient_y) || (NULL == pGradient))
    {
        printf("ERROR :: No Memory\n");
        goto EXIT;
    }
    
    if(argc < 4)
    {
        printf("\n******************************************************\n");
        printf("usage:\n");
        printf("\textract_feature  label  infile  outfile\n");
        printf("\tPara label : 1 or 0 for  two-cass\n");
        printf("\tPara infile   : include the absolut path for each image\n");
		printf("\tPara outfile  : the absolut path of output file\n");
        printf("******************************************************\n\n");
        goto EXIT;
    }

    label = atoi(argv[1]);
    printf("The label of this processing is %d\n", label);
    sprintf(save_path, "%s", argv[3]);

    file = fopen(argv[2], "r");
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
            
        if((IMG_WIDTH != src_img->width) || (IMG_HEIGHT != src_img->height))
            cvResize(__src_img, resize_img, CV_INTER_NN);
        else
            memcpy(resize_img->imageData, __src_img->imageData, IMG_WIDTH*IMG_HEIGHT*3);
		//{
		//	FILE *tmp = fopen("../bin/resize_img.txt","wb");
		//	fwrite(resize_img->imageData, 1,resize_img->widthStep*resize_img->height, tmp );
		//	fclose(tmp);
		//}

        cvCvtColor(resize_img, gray_img, CV_RGB2GRAY);
	// extract WanHuaLin feature
        if(0 != RGBtoHSL(resize_img->imageData, hsl_img->imageData,IMG_HEIGHT, IMG_WIDTH,resize_img->widthStep, hsl_img->widthStep))
        {
            printf("ERROR :: Error occured in RGB2HSL\n ");
            continue;
        } 
	/* 
        if(0 != WanHuaLinColorFea(hsl_img->imageData, hsl_img->widthStep, IMG_WIDTH, IMG_HEIGHT,pFea))
        {
            printf("ERROR :: Error occured in extracting WanHuaLin feature\n ");
            continue;
        }
	*/
        // extract HOG feature	
        HogFea(NULL, gray_img->imageData, gray_img->width, gray_img->height,pFea+WAN_DIM);

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
        for(i=0; i<FEA_DIM; i++)
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
        checkGradientImg(pGradient_x, pGradient_y, pGradient,(unsigned char*)(gradient_img->imageData),IMG_WIDTH, IMG_HEIGHT);
        cvShowImage("test", gradient_img);
        cvWaitKey(0);
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



#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "tcomdef.h"
#include "SVMDetector.h"

//#define SHOW_IMG


int main(int argc, char **argv)
{	 
    FILE *file = NULL, *file_output=NULL;
    char image_path_name[1024] = {0}; 
    IplImage  *src_img    = NULL;
    IplImage  *__src_img =  NULL;
    IplImage  *__src_img_BGR =  NULL;
    int label, rval;
    TRECT rect;
    THandle detector = 0;
    
    if(argc < 3)
    {
        printf("\n******************************************************\n");
        printf("usage:\n");
        printf("svm-detector model  input output \n");
        printf("******************************************************\n\n");
        goto EXIT;
    }

    file = fopen(argv[2], "r");
    if(NULL == file)
    {
        printf("ERROR :: The input file not exist\n");
        goto EXIT;
    }

    file_output = fopen(argv[3], "w");
    if(NULL == file_output)
    {
        printf("ERROR :: The out file not exist\n");
        goto EXIT;
    }


#ifdef SHOW_IMG
    cvNamedWindow("test", 0);
#endif
    
    detector = SVMDetector_init(NULL, argv[1]);
    if(0 == detector)
    {
        printf("Init svm detector failed\n");
        goto EXIT;
    }

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
        __src_img_BGR = cvCreateImage(cvSize(src_img->width, src_img->height), 8,3);

		if(src_img->nChannels == 1)
            cvCvtColor(src_img, __src_img, CV_GRAY2RGB);
        else
            memcpy(__src_img->imageData, src_img->imageData, src_img->widthStep*src_img->height); 

        cvCvtColor(__src_img, __src_img_BGR, CV_RGB2BGR);

        rect.top = rect.left = 0;
        rect.bottom = __src_img->height;
        rect.right = __src_img->width;

        rval = SVMDetector_detect(detector, __src_img_BGR->imageData,
                                  __src_img_BGR->width, __src_img_BGR->height,
                                  __src_img_BGR->widthStep, rect, &label);
        if(0 == rval)
            fprintf(file_output, "%d\n", label);
        else
            fprintf(file_output, "%s :: failed",image_path_name);
      
#ifdef SHOW_IMG
        cvShowImage("test", __src_img);
        cvWaitKey(0);
        {
            CvFont font;    
            double hScale=1;   
            double vScale=1;    
            int lineWidth=2;   
            char showMsg[1024]={0};
            if(1 == label)
                sprintf(showMsg, "face");
            else if(0 == label)
                sprintf(showMsg, "non-face");
            if(0!=rval)
                sprintf(showMsg, "failed");
            cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth, 8);
            cvPutText(__src_img_BGR,showMsg,cvPoint(0,0),&font,CV_RGB(255,0,0));
        }
        cvShowImage("test", __src_img_BGR);
        cvWaitKey(0);
#endif

        cvReleaseImage(&src_img);
        cvReleaseImage(&__src_img);
        cvReleaseImage(&__src_img_BGR);
        src_img = NULL;        
    }

#ifdef SHOW_IMG
    cvDestroyWindow("test");
#endif

 EXIT:
    if(file)          fclose(file);
    if(file_output)   fclose(file_output);
    SVMDetector_uninit(&detector);
    return 0;
}

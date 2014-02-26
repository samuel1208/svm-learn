#ifndef __WAN_HUA_LIN_FEA_H__
#define __WAN_HUA_LIN_FEA_H__

#ifdef __cplusplus
extern "C" {
#endif

    int GetWANDim();
    //73 bin for WanHuaLin color feature
    int WanHuaLinColorFea(unsigned char *pHSL,  int i32LineByte, 
                          int i32Width, int i32Height, int *pFea);
    int BGRtoHSL(unsigned char *pBGR, unsigned char *pHsl, int rows, int cols,
                 int lineBytesBGR, int lineBytesHSL);
    
    int  RGBtoHSL(unsigned char *pRGB, unsigned char *pHSL, int rows, int cols, 
                  int lineBytesRGB, int lineBytesHSL);
    int  ScaleImg3(unsigned char *pSrc, int srcWidth, int srcHeight, 
                   int srcWidthStep,  unsigned char *pDst, int dstWidth,
                   int dstHeight, int dstWidthStep);
    
    int  BGRtoGray(unsigned char *pBGR, int width, int height, int widthStep, 
                   unsigned char *pGray);
    
#ifdef __cplusplus
}
#endif

#endif

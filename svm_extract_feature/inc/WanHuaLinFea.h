#ifndef __WAN_HUA_LIN_FEA_H__
#define __WAN_HUA_LIN_FEA_H__

#ifdef __cplusplus
extern "C" {
#endif

//73 bin for WanHuaLin color feature
int WanHuaLinColorFea(unsigned char *pHSL,  int i32LineByte, int i32Width, int i32Height, int *pFea);
int RGBtoHSL(unsigned char *pRGB, unsigned char *pHsl, int rows, int cols, int lineBytesRBG, int lineBytesHSL);

#ifdef __cplusplus
}
#endif

#endif

#ifndef __LBP_FEA_H__
#define __LBP_FEA_H__

#include "tcomdef.h"

#ifdef __cplusplus
extern "C" {
#endif

    /*return value : the number of LBP features*/    
    int LBPH_Fea(THandle hMem, unsigned char *pSrcImg, int widthStep,
                 int width, int height, int radius, int neighbor, 
                 int grid_x, int grid_y, int *pFea);


#ifdef __cplusplus
}
#endif

#endif

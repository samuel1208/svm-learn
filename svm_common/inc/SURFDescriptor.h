#ifndef __SURF_DESCRIPTOR_H__
#define __SURF_DESCRIPTOR_H__


#ifdef __cplusplus
extern "C"{
#endif
    

#include "tcomdef.h"
#include "tmem.h"

    int SURFFea(THandle hMemBuf, unsigned char *pGray, int nWidthStep, int width, int height, int *pSURFFea);


#ifdef __cplusplus
}
#endif

#endif


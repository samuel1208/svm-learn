#include "WanHuaLinFea.h"
#include "tcomdef.h"
#define trimBYTE(x)		(unsigned char)((x)&(~255) ? ((-(x))>>31) : (x))

static const  char  hueTab[256] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 
3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 
4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 
5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 
6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
};

static const unsigned short g_ByteDivTab[] = {
	65535, 65535, 32768, 21845, 16384, 13107, 10922, 9362, 8192, 7281, 6553, 5957, 5461, 5041, 4681, 4369,
	 4096,  3855,  3640,  3449,  3276,  3120,  2978, 2849, 2730, 2621, 2520, 2427, 2340, 2259, 2184, 2114,
	 2048,  1985,  1927,  1872,  1820,  1771,  1724, 1680, 1638, 1598, 1560, 1524, 1489, 1456, 1424, 1394,
  	 1365,  1337,  1310,  1285,  1260,  1236,  1213, 1191, 1170, 1149, 1129, 1110, 1092, 1074, 1057, 1040,
	 1024,  1008,   992,   978,   963,   949,   936,  923,  910,  897,  885,  873,  862,  851,  840,  829,
	  819,   809,   799,   789,   780,   771,   762,  753,  744,  736,  728,  720,  712,  704,  697,  689,
	  682,   675,   668,   661,   655,   648,   642,  636,  630,  624,  618,  612,  606,  601,  595,  590,
	  585,   579,   574,   569,   564,   560,   555,  550,  546,  541,  537,  532,  528,  524,  520,  516,
	  512,   508,   504,   500,   496,   492,   489,  485,  481,  478,  474,  471,  468,  464,  461,  458,
	  455,   451,   448,   445,   442,   439,   436,  434,  431,  428,  425,  422,  420,  417,  414,  412,
	  409,   407,   404,   402,   399,   397,   394,  392,  390,  387,  385,  383,  381,  378,  376,  374,
	  372,   370,   368,   366,   364,   362,   360,  358,  356,  354,  352,  350,  348,  346,  344,  343,
	  341,   339,   337,   336,   334,   332,   330,  329,  327,  326,  324,  322,  321,  319,  318,  316,
	  315,   313,   312,   310,   309,   307,   306,  304,  303,  302,  300,  299,  297,  296,  295,  293,
	  292,   291,   289,   288,   287,   286,   284,  283,  282,  281,  280,  278,  277,  276,  275,  274,
	  273,   271,   270,   269,   268,   267,   266,  265,  264,  263,  262,  261,  260,  259,  258,  257
};

int WanHuaLinColorFea(unsigned char *pHSL,  int i32LineByte, int i32Width, int i32Height, int *pFea)
{
    #define COLOR_BIN 73
    #define GAP 1
	unsigned char  h, s, l;
	int   index;
	int   x, y, pt, lineBytes;
	int   hist[COLOR_BIN]={0};
	int   size;
	
    if ((TNull == pHSL) || (TNull == pFea))
        return -1;

	lineBytes = i32LineByte;	
	// calculate color index and construct color histogram 
	for(y=0; y<i32Height; y+=GAP)
	{
		pt = y*lineBytes;
		for(x=0; x<i32Width; x+=GAP)
		{
			h = pHSL[pt];
			s = pHSL[pt+1];
			l = pHSL[pt+2];

			if(l<32)
			{
				hist[72]++;
				pt+=3*GAP;
				continue;
			}
			
			// hue factor
			index = hueTab[h]*9;
			
			// sat factor
			if(s>180)  
				index += 6;
			else if(s>52)  
				index += 3;
			
			// lum factor			
			if(l>180) 
				index += 2;
			else if(l>80) 
				index += 1;
			
			hist[index]++;			
			pt+=3*GAP;
		}
	}
	
	size = (1<<24)/((i32Height>>1)*(i32Width>>1)); //largen the feature with (1<<24)
	for(x=0; x<COLOR_BIN; x++)
		pFea[x] = hist[x]*size;
	
    return 0;
}

int  BGRtoHSL(unsigned char *pBGR, unsigned char *pHSL, int rows, int cols, int lineBytesBGR, int lineBytesHSL)
{
	register int RH, GS, BL;
	register int  nAdd,nDelta,cMax,cMin;
	
    if ((TNull == pBGR) || (TNull == pHSL))
        return -1;

	lineBytesBGR -= cols * 3;
	lineBytesHSL -= cols * 3;
	rows |= cols << 16;
	for(; (rows << 16) != 0; rows--)
	{
		for(cols = rows >> 16; cols!= 0; cols--)
		{

			BL = pBGR[0];			
			GS = pBGR[1];
            RH = pBGR[2];
			
			if(RH > GS)
			{
				if(BL > RH)
				{
					cMax = BL;
					cMin = GS;
					nDelta = (int)GS - (int)RH;
					nAdd = 170;
				}
				else
				{
					cMax = RH;
					nDelta = (int)BL - (int)GS;
					nAdd = 0;
					if(BL < GS)
						cMin = BL;
					else
						cMin = GS;
				}
			}
			else
			{
				if(BL > GS)
				{
					cMax = BL;
					cMin = RH;
					nDelta = (int)GS - (int)RH;
					nAdd = 170;
				}
				else 
				{
					cMax = GS;
					nDelta = (int)RH - (int)BL;
					nAdd = 85;
					if(BL < RH)
						cMin = BL;
					else
						cMin = RH;
				}
			}

			BL = (unsigned char)(((cMax+cMin)+1)>>1);
			if (cMax==cMin){			// r=g=b --> achromatic case
				GS = 0;					// saturation
				RH = 170;
			}
			else 
			{	// chromatic case		
				cMax += cMin;
				cMin = cMax - (cMin << 1);
//				mid = cMax+cMin;
//				sub = cMax-cMin;
				
				if (BL > 127)
					cMax = 510 - cMax;

				GS = g_ByteDivTab[cMax];
				RH = g_ByteDivTab[cMin];

				//GS = (MByte)(((cMin*255)+(cMax>>1))/cMax);
				//RH = (MByte)(nAdd - ( nDelta * 42  + (cMin>>1) ) / cMin);
				GS = (((cMin*255)+(cMax>>1)) * GS >> 16);
				RH = (nAdd - (( nDelta * 42  + (cMin>>1) )  * RH >> 16));
			}	
			pHSL[0]=trimBYTE(RH);
			pHSL[1]=trimBYTE(GS);
			pHSL[2]=trimBYTE(BL);			
			pBGR += 3;
			pHSL += 3;
		}
		pBGR += lineBytesBGR;
		pHSL += lineBytesHSL;
	}
    return 0;
}


int  ScaleImg3(unsigned char *pSrc, int srcWidth, int srcHeight, int srcWidthStep,
               unsigned char *pDst, int dstWidth, int dstHeight, int dstWidthStep)
{
    int w,h, index_x, index_y;
    float scale_x, scale_y;
    unsigned char *pData;
    
    if((TNull == pSrc)||(TNull == pDst))
        return -1;
    
    scale_x = srcWidth*1.0f/dstWidth;
    scale_y = srcHeight*1.0f/dstHeight;

    pData = pDst;
    for(h=0; h<dstHeight; h++)
    {
        index_y = (int)(h*scale_y + 0.5f);
        index_y = TMIN(index_y, srcHeight-1);
        
        for(w=0; w<dstWidth; w++)
        {
            index_x = (int)(w*scale_x + 0.5f);
            index_x = TMIN(index_x, srcWidth-1);
            pData[3*w] = pSrc[index_y*srcWidthStep + index_x*3];
            pData[3*w+1] = pSrc[index_y*srcWidthStep + index_x*3+1];
            pData[3*w+2] = pSrc[index_y*srcWidthStep + index_x*3+2];
        }
        pData += dstWidthStep;
    }
    return 0;
}


int  BGRtoGray(unsigned char *pBGR, int width, int height, int widthStep, unsigned char *pGray)
{
    int w,h;
    int val;
    unsigned char r, g, b;
    unsigned char *pSrc = TNull, *pDst=TNull;
    
    if((TNull == pBGR)||(TNull == pGray))
        return -1;
    pSrc = pBGR;
    pDst = pGray;
    for(h=0; h<height; h++)
    {
        for(w=0; w<width; w++)
        {
            b = pSrc[w*3];
            g = pSrc[w*3+1];
            r = pSrc[w*3+2];
            pDst[w] = (int)(r*0.299 + g*0.587 + b*0.114);
        }
        pSrc += widthStep;
        pDst += width;
    }
    return 0;
}

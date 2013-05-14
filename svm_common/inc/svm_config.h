#ifndef __SVM_CONFIG_H__
#define __SVM_CONFIG_H__

#define WAN_HUA_LIN_ENABLE
#define HOG_ENABLE
#define LBP_ENABLE


#define IMG_WIDTH  48
#define IMG_HEIGHT 48

#ifdef HOG_ENABLE
    #define HOG_DIM  (((IMG_WIDTH/12)-1)*((IMG_WIDTH/12)-1)*4*9)
#else
    #define HOG_DIM  (0)
#endif

#ifdef WAN_HUA_LIN_ENABLE
    #define WAN_HUA_LIN_DIM  (73)
#else
    #define WAN_HUA_LIN_DIM  (0)
#endif

#ifdef LBP_ENABLE
    #define LBP_MAX_RADIUS (3)
    #define LBP_RADIUS_2   (2)
    #define LBP_RADIUS_1   (1)
    #define LBP_GRID_X  (3)
    #define LBP_GRID_Y  (3)
    #define LBP_NEIGHBOR_16  (16)
    #define LBP_DIM_16  ((LBP_NEIGHBOR_16+2)*(LBP_GRID_X)*(LBP_GRID_Y))
    #define LBP_NEIGHBOR_8  (8)
    #define LBP_DIM_8  ((LBP_NEIGHBOR_8+2)*(LBP_GRID_X)*(LBP_GRID_Y))
#else
    #define LBP_DIM_16  (0)
    #define LBP_DIM_8   (0)
    #define LBP_MAX_RADIUS (0)
#endif

#define SVM_FEA_DIM (HOG_DIM + WAN_HUA_LIN_DIM + LBP_DIM_16)



#endif

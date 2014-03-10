#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int neighbor;
    int *pLabel = NULL;
    int label;
    int i, j;
    char outputPath[1024] = {0};
    FILE *file = NULL;
    if(argc<2)
    {
        printf("-------------------------------------------------\n");
        printf("usage: \n");
        printf("       svm_generate_LBP_LOOKUP  neighborNumber\n");
        printf("-------------------------------------------------\n");
        return -1;
    }
    
    neighbor = atoi(argv[1]);
    if(neighbor < 0)
    {
        printf("ERROR :: The neighbor is < 0 \n");
        return -1;
    }

    pLabel = (int *)malloc((1<<neighbor)*sizeof(int));
    if(NULL == pLabel)
    {
        printf("ERROR :: Can't alloc buffer for pLabel\n");
        return -1;
    }

    //intial the label
    for(i=0; i<(1<<neighbor); i++)
    {
        pLabel[i] = -1;
    }
    
    label = 0;
    pLabel[0] = label;    
    // compute the label
    for(i=1; i<(neighbor); i++)
    {
        unsigned int val = 0;
        unsigned int val_base = 0;
        int highestBit= 0;
        label++;
        //get the base value
        for(j=0; j<neighbor; j++)
        {
            val_base = val_base<<1;
            val_base += 1;
        }
        // get the value first
        for(j=0; j<i; j++)
        {
            val = val << 1 ;
            val += 1;
        }
        pLabel[val] = label;


        for(j=1; j<neighbor; j++)
        {
#if 1 //Don't use ori_invariance property
            label++;
#endif
            // make a circle move
            highestBit = val >> (neighbor-1);
            val = val<<1;
            val = (val & val_base) + highestBit;
            pLabel[val] = label;
        }

    }
    label ++;
    pLabel[(1<<neighbor)-1] = label;
    label ++;
    for(i=0; i<(1<<neighbor); i++)
    {
        if(-1 == pLabel[i])
            pLabel[i]= label;
    }
    
    //
    sprintf(outputPath, "../../LBP_lookup.h");
    file = fopen(outputPath, "w");
    if(NULL == file)
    {
        printf("ERROR :: Can't Open the output file\n");
        return -1;
    }
    
    fprintf(file, "#ifndef  __LBP_LOOKUP_TABLE_H__\n");
    fprintf(file, "#define  __LBP_LOOKUP_TABLE_H__\n\n");

    fprintf(file, "#ifdef __cplusplus\n");
    fprintf(file, "extern \"C\"{\n");
    fprintf(file, "#endif\n");
    fprintf(file,"unsigned char pLBP_lookUP_table_%d[]={\n", neighbor);
    fprintf(file,"  %3d", pLabel[0]);
    for(i=1; i<(1<<neighbor); i++)
    {
        fprintf(file,", %3d", pLabel[i]);
        if(0 == (i+1)%16)
            fprintf(file,"\n");
    }
    fprintf(file,"};", neighbor);


    fprintf(file, "\n#ifdef __cplusplus\n");
    fprintf(file, "}\n");
    fprintf(file, "#endif\n\n");
    fprintf(file, "#endif\n");

    printf("The Label Number :: %d\n", ++label);
    fclose(file);
    free(pLabel);
    return 0;    
}

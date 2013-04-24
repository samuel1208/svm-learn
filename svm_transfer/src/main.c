
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static const char *svm_type_table[] =
{
	"c_svc","nu_svc","one_class","epsilon_svr","nu_svr",NULL
};

static const char *kernel_type_table[]=
{
	"linear","polynomial","rbf","sigmoid","precomputed",NULL
};

static char *line = NULL;
static int max_line_len;


static int readMinMaxFile(FILE *srcFile,FILE *file_h, FILE *file_c, char *suffix);
static int readTrainModel(FILE *srcFile,FILE *file_h, FILE *file_c, char *suffix);
static char* readline(FILE *input);

static char* readline(FILE *input)
{
	int len;

	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

static int readMinMaxFile(FILE *srcFile,FILE *file_h, FILE *file_c, char *suffix)
{
    char lineStr[1024]={0};
    int minVal, maxVal;
    int index, index_full=1, i;
    
    if ((NULL == srcFile) || (NULL == file_h) || (NULL == file_c) || (NULL == suffix))
        return -1; 

    // Read the first line, and discard it
    fgets(lineStr, 1024, srcFile);

    // read the range
    memset(lineStr, 0, 1024);
    fgets(lineStr, 1024, srcFile);
    if(2 != sscanf(lineStr, "%d %d", &minVal, &maxVal))
        return -1;

    fprintf(file_h,  "extern int i32MinRange_%s;\n", suffix);
    fprintf(file_h,  "extern int i32MaxRange_%s;\n", suffix);
    fprintf(file_c,  "int i32MinRange_%s = %d;\n", suffix, minVal);
    fprintf(file_c,  "int i32MaxRange_%s = %d;\n", suffix, maxVal);

    fprintf(file_h,  "extern int pMinMaxFeaVal_%s[];\n", suffix);
    fprintf(file_c,  "int pMinMaxFeaVal_%s[] = {\n", suffix );
    //read the min and max value for each dim

	index_full = 1;
    memset(lineStr, 0, 1024);
    fgets(lineStr, 1024, srcFile);
    if(3 == sscanf(lineStr,"%d %d %d",&index, &minVal, &maxVal))
    {
		if(index > index_full)
		{
			fprintf(file_c,  "%d, %d", 0, 0);
			for(i=index_full+1; i<index; i++)
				fprintf(file_c,  ", %d, %d", 0, 0);
			fprintf(file_c,  ", %d, %d", minVal, maxVal);
		}
		else
			fprintf(file_c,  "%d, %d", minVal, maxVal);
		index_full = index +1;
    }
    while(!feof(srcFile))
    {
        memset(lineStr, 0, 1024);
        fgets(lineStr, 1024, srcFile);
        if(3 != sscanf(lineStr,"%d %d %d",&index, &minVal, &maxVal))
            continue;
		if(index > index_full)
		{
			for(i=index_full; i<index; i++)
				fprintf(file_c,  ", %d, %d", 0, 0);
		}
		index_full = index +1;

		fprintf(file_c,  ", %d, %d", minVal, maxVal);        
        if(index != 0 && index % 100 == 0)
            fprintf(file_c, "\n");
    }
    fprintf(file_c,  "};\n");

    return 0;   
}

static int readTrainModel(FILE *srcFile,FILE *file_h, FILE *file_c, char *suffix)
{
    int rVal = 0;
    char cmd[80] = {0};
    int i,j,k, nClass=0, total_sv_num=0;
    int isDegree=0,isGamma=0, isNr_class=0, isTotal_sv=0, isRho=0, isLabel=0, isNr_sv=0, isCoef0=0, isProbA=0, isProbB=0; 
    
    int elements = 0, m, l;
	long pos = 0;
    char *p,*endptr,*idx,*val;
    
    double *pCoef = NULL, *pSv_value = NULL;
    int  *pSv_index = NULL;
  
    char *old_locale = strdup(setlocale(LC_ALL, NULL));
	setlocale(LC_ALL, "C");

    if ((NULL == srcFile) || (NULL == file_h) || (NULL == file_c) || (NULL == suffix))
    {
        rVal = -1; 
        goto EXIT;
    }

    while(1)
    {       
        fscanf(srcFile, "%80s", cmd);
        if(strcmp(cmd,"svm_type")==0) // read svm type
        {
            fscanf(srcFile, "%80s", cmd);
			for(i=0;svm_type_table[i];i++)
			{
				if(strcmp(svm_type_table[i],cmd)==0)
				{
                    fprintf(file_h, "extern int svm_type_%s;\n", suffix);
                    fprintf(file_c, "int svm_type_%s = %d ;\n", suffix, i);
					break;
				}
			}
			if(svm_type_table[i] == NULL)
                return -1;
        }
        else if(strcmp(cmd,"kernel_type")==0)
        {
            fscanf(srcFile, "%80s", cmd);
			for(i=0;kernel_type_table[i];i++)
			{
				if(strcmp(kernel_type_table[i],cmd)==0)
				{
                    fprintf(file_h, "extern int kernel_type_%s;\n", suffix);
                    fprintf(file_c, "int kernel_type_%s = %d ;\n", suffix, i);
					break;
				}
			}
			if(kernel_type_table[i] == NULL)
                return -1;            
        }
        else if(strcmp(cmd,"degree")==0)
        {
            int degree;
			fscanf(srcFile, "%d", &degree);
            fprintf(file_h, "extern int degree_%s;\n", suffix);
            fprintf(file_c, "int degree_%s = %d ;\n", suffix, degree);
            isDegree = 1;
        }
		else if(strcmp(cmd,"gamma")==0)
        {
            double gamma;
			fscanf(srcFile, "%lf", &gamma);
            fprintf(file_h, "extern double gamma_%s;\n", suffix);
            fprintf(file_c, "double gamma_%s = %g ;\n", suffix, gamma);
            isGamma = 1;
        }
		else if(strcmp(cmd,"coef0")==0)
        {
            double coef0;
			fscanf(srcFile,"%lf",&coef0);
            fprintf(file_h, "extern double coef0_%s;\n", suffix);
            fprintf(file_c, "double coef0_%s = %g ;\n", suffix, coef0);
            isCoef0 = 1;
        }
		else if(strcmp(cmd,"nr_class")==0)
        {
			fscanf(srcFile, "%d", &nClass);
            fprintf(file_h, "extern int nr_class_%s;\n",suffix);
            fprintf(file_c, "int nr_class_%s = %d ;\n",suffix, nClass);
            isNr_class=1;
        }
		else if(strcmp(cmd,"total_sv")==0)
        {
			fscanf(srcFile, "%d", &total_sv_num);
            fprintf(file_h, "extern int total_sv_%s;\n", suffix);
            fprintf(file_c, "int total_sv_%s = %d ;\n", suffix, total_sv_num);
            isTotal_sv=1;
        }
		else if(strcmp(cmd,"rho")==0)
		{
			int n = nClass * (nClass-1)/2;
            double rho;
            fprintf(file_h, "extern double pRho_%s[];\n", suffix);
            fprintf(file_c, "double pRho_%s[] = {", suffix);
            fscanf(srcFile, "%lf", &rho);
            fprintf(file_c, "%g",rho);
			for(i=0;i<n-1;i++)
            {
				fscanf(srcFile, "%lf", &rho);
                fprintf(file_c, ", %g ",rho);
            }
            fprintf(file_c, "};\n");
            isRho=1;
		}
		else if(strcmp(cmd,"label")==0)
		{
            int n = nClass;
            int label;
            
            fprintf(file_h, "extern int pLabel_%s[];\n", suffix);
            fprintf(file_c, "int pLabel_%s[] = {", suffix);
            fscanf(srcFile, "%d", &label);
            fprintf(file_c, "%d",label);
			for(i=0;i<n-1;i++)
            {
				fscanf(srcFile, "%d", &label);
                fprintf(file_c, ", %d",label);
            }
            fprintf(file_c, "};\n");
            isLabel = 1;
		}
		else if(strcmp(cmd,"probA")==0)
		{
            int n = nClass * (nClass-1)/2;
            double probA;
            fprintf(file_h, "extern double pProbA_%s[];\n", suffix);
            fprintf(file_c, "double pProbA_%s[] = {", suffix);
            fscanf(srcFile, "%lf", &probA);
            fprintf(file_c, "%g",probA);
			for(i=0;i<n-1;i++)
            {
				fscanf(srcFile, "%lf", &probA);
                fprintf(file_c, ", %g ",probA);
            }
            fprintf(file_c, "};\n");
            isProbA = 1;
		}
		else if(strcmp(cmd,"probB")==0)
		{
            int n = nClass * (nClass-1)/2;
            double probB;
            fprintf(file_h, "extern double pProbB_%s[];\n", suffix);
            fprintf(file_c, "double pProbB_%s[] = {", suffix);
            fscanf(srcFile, "%lf", &probB);
            fprintf(file_c, "%g",probB);
			for(i=0;i<n-1;i++)
            {
				fscanf(srcFile, "%lf", &probB);
                fprintf(file_c, ", %g ",probB);
            }
            fprintf(file_c, "};\n");
            isProbB=1;
		}
		else if(strcmp(cmd,"nr_sv")==0)
		{
			int n = nClass;
            int nr_sv;
            
            fprintf(file_h, "extern int pNr_sv_%s[];\n", suffix);
            fprintf(file_c, "int pNr_sv_%s[] = {", suffix);
            fscanf(srcFile, "%d", &nr_sv);
            fprintf(file_c, "%d",nr_sv);
			for(i=0;i<n-1;i++)
            {
				fscanf(srcFile, "%d", &nr_sv);
                fprintf(file_c, ", %d",nr_sv);
            }
            fprintf(file_c, "};\n");
            isNr_sv=1;
		}
		else if(strcmp(cmd,"SV")==0)
		{
			while(1)
			{
				int c = getc(srcFile);
				if(c==EOF || c=='\n') break;// read the \n character
			}
			break;
		}
		else
        {		
			rVal -1;
            goto EXIT;
        }	   
    }

    //update the unread para
    if(0 == isDegree)
    {
        fprintf(file_h, "extern int degree_%s;\n", suffix);
        fprintf(file_c, "int degree_%s = %d ;\n", suffix, 0);
    }
    if(0 == isGamma)
    {
        fprintf(file_h, "extern double gamma_%s;\n", suffix);
        fprintf(file_c, "double gamma_%s = %g ;\n", suffix, 0.0);
    }
    if(0 == isCoef0)
    {
         fprintf(file_h, "extern double coef0_%s;\n", suffix);
         fprintf(file_c, "double coef0_%s = %g ;\n", suffix, 0.0);
    }
    if(0 == isNr_class)
    {
        fprintf(file_h, "extern int nr_class_%s;\n",suffix);
        fprintf(file_c, "int nr_class_%s = %d ;\n",suffix, 0);
    }
    if(0 == isTotal_sv)
    {
        fprintf(file_h, "extern int total_sv_%s;\n", suffix);
        fprintf(file_c, "int total_sv_%s = %d ;\n", suffix, 0);
    }
    if(0 == isRho)
    {
        
        fprintf(file_h, "extern double *pRho_%s;\n", suffix);
        fprintf(file_c, "double *pRho_%s = 0; \n", suffix);
    }
    if(0 == isLabel)
    {
        fprintf(file_h, "extern int *pLabel_%s;\n", suffix);
        fprintf(file_c, "int *pLabel_%s = 0;\n", suffix);
    }
    if(0 == isProbA)
    {
        fprintf(file_h, "extern double *pProbA_%s;\n", suffix);
        fprintf(file_c, "double *pProbA_%s = 0; \n", suffix);
    }
    if(0 == isProbB)
    {
        fprintf(file_h, "extern double *pProbB_%s;\n", suffix);
        fprintf(file_c, "double *pProbB_%s = 0; \n", suffix);
    }
    if(0 == isNr_sv)
    {        
        fprintf(file_h, "extern int *pNr_sv_%s;\n", suffix);
        fprintf(file_c, "int *pNr_sv_%s = 0; \n", suffix);
    }
    
    // read sv_coef and SV
    elements = 0;
    pos = ftell(srcFile);
	max_line_len = 1024;
	line = malloc(sizeof(char)*max_line_len);
	

    while(readline(srcFile)!=NULL)
	{
		p = strtok(line,":");
		while(1)
		{
			p = strtok(NULL,":");
			if(p == NULL)
				break;
			++elements;
		}
	}
    //libsvm doesn't store the zero value. 
	elements += total_sv_num;// add 1 more for store -1 in the last as a sentry
    fseek(srcFile,pos,SEEK_SET);

    m = nClass-1;

    pCoef = (double *)malloc(sizeof(double)*m*total_sv_num);
    pSv_index = (int *)malloc(sizeof(int) * elements);
    pSv_value = (double *)malloc(sizeof(double)*elements);

    if((NULL == pCoef)||(NULL == pSv_index)||(NULL == pSv_value))
    {
        rVal = -1;
        goto EXIT;
    }
    j=0;
	for(i=0;i<total_sv_num;i++)
	{
		readline(srcFile);
		p = strtok(line, " \t");
        pCoef[i] =  strtod(p,&endptr);
		for(k=1;k<m;k++)
		{
			p = strtok(NULL, " \t");
            pCoef[k*total_sv_num+i] = strtod(p,&endptr);
		}

		while(1)
		{
			idx = strtok(NULL, ":");
			val = strtok(NULL, " \t");

			if(val == NULL)
				break;
            pSv_index[j] = (int) strtol(idx,&endptr,10);
            pSv_value[j] = strtod(val,&endptr);
			++j;
		}
	//	pSv_value[j] = 0.0f;
        pSv_index[j++] = -1;
                    
	}
    //save the coef
    fprintf(file_h, "extern double pCoef_%s[];\n", suffix);
    fprintf(file_c, "double pCoef_%s[] = {%.16g", suffix, pCoef[0]);
    
    for(i=1; i<m*total_sv_num; i++)
    {
        fprintf(file_c, ", %.16g", pCoef[i]);
        if(i%100 == 0)
            fprintf(file_c, "\n");
    }
    fprintf(file_c, "};\n");

    //save sv_index and value    
    fprintf(file_h, "extern svm_node pSvm_node_%s[];\n", suffix);
    fprintf(file_c, "svm_node pSvm_node_%s[] = {{%d,%.8g}", suffix, pSv_index[0], pSv_value[0]);
    for(i=1; i<elements; i++)
    {
        fprintf(file_c, ", {%d,%.8g}", pSv_index[i], pSv_value[i]);
        if(i%100 == 0)
            fprintf(file_c, "\n");
    }
    fprintf(file_c, "};\n");

    fprintf(file_h, "extern int free_sv_%s;\n", suffix);
    fprintf(file_c, "int free_sv_%s = %d;\n", suffix, 1);

    rVal = 0;
 EXIT:
    setlocale(LC_ALL, old_locale);
    if(old_locale) free(old_locale);
    if(pSv_index)  free(pSv_index);
    if(pSv_value)  free(pSv_value);
    if(pCoef)      free(pCoef);
    if(line)       free(line);
    return rVal;    
}

int main(int argc, char **argv)
{
    int rVal = 0;
    FILE *maxMinFile=NULL, *trainModelFile=NULL;
    FILE *saveFile_h = NULL, *saveFile_c = NULL;
    char *savePath_c = "./svm_constant.c";
    char *savePath_h = "./svm_constant.h";
    char lineStr[1024];
    if (argc<4)
    {
        printf("---------------------------------------------------------\n");
        printf("usage :\n ");
        printf("\tsvm_transfer  maxMinFile  trainModel  suffix\n");
        printf("---------------------------------------------------------\n");
        rVal = -1;
        goto EXIT;
    }    

    saveFile_h = fopen(savePath_h, "w");
    saveFile_c = fopen(savePath_c, "w");
    if(NULL == saveFile_h || NULL== saveFile_c)
    {
        printf("ERROR : The saveing file isn't exist\n");
        rVal = -1;
        goto EXIT;
    }

    fprintf(saveFile_h, "#ifndef  __SVM_TRAIN_MODEL_H__\n");
    fprintf(saveFile_h, "#define  __SVM_TRAIN_MODEL_H__\n\n");
    fprintf(saveFile_h, "#include \"svm.h\"\n");
	fprintf(saveFile_c, "\n#include \"svm_constant.h\"\n\n");

    //process the min and max file
    maxMinFile = fopen(argv[1], "r");
    if(NULL == maxMinFile)
    {
        printf("ERROR : The minMaxFile isn't exist\n");
        rVal = -1;
        goto EXIT;
    }

    if( 0 != readMinMaxFile(maxMinFile, saveFile_h, saveFile_c, argv[3]))
    {
        printf("ERROR : readMinMaxFile function failed\n");
        rVal = -1;
        goto EXIT;
    }

    //process the train mode file
    trainModelFile = fopen(argv[2], "r");
    if(NULL == trainModelFile)
    {
        printf("ERROR : The trainModelFile isn't exist\n");
        rVal = -1;
        goto EXIT;
    }
    if( 0 != readTrainModel(trainModelFile, saveFile_h, saveFile_c, argv[3]))
    {
        printf("ERROR : readTrainMode function failed\n");
        rVal = -1;
        goto EXIT;
    }
    fprintf(saveFile_h, "\n#endif\n");

    rVal = 0;
 EXIT:
    if(saveFile_h) fclose(saveFile_h);
    if(saveFile_c) fclose(saveFile_c);
    if(trainModelFile) fclose(trainModelFile);
    if(maxMinFile) fclose(maxMinFile);
    return rVal;;
}

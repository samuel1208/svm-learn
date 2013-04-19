#include "svm.h"
#include "svm_constant.h"
#include "tmem.h"
#include "math.h"
#include <stdio.h>
#define   INIT_SVM_FUNC(suffix)                                     \
          static svm_model*  Init_svm_##suffix (THandle hMemBuf)    \
          {                                                         \
              int rVal = 0, m = 0, total_sv_num=0, i, j;            \
              svm_model *pSvmModel = TNull;                         \
                                                                    \
              pSvmModel = TMemAlloc(hMemBuf, sizeof(svm_model));    \
              if(TNull == pSvmModel)                                \
              {                                                     \
                  rVal = -1;                                        \
                  goto EXIT;                                        \
              }                                                     \
              pSvmModel->param.svm_type = svm_type_##suffix;        \
              pSvmModel->param.kernel_type = kernel_type_##suffix;  \
              pSvmModel->param.degree = degree_##suffix;            \
              pSvmModel->param.gamma  = gamma_##suffix;             \
              pSvmModel->param.coef0  = coef0_##suffix;             \
              pSvmModel->nr_class = nr_class_##suffix;              \
              pSvmModel->l = total_sv_##suffix;                     \
              pSvmModel->rho = pRho_##suffix;                       \
              pSvmModel->probA = pProbA_##suffix;                    \
              pSvmModel->probB = pProbB_##suffix;                    \
              pSvmModel->label = pLabel_##suffix;                   \
              pSvmModel->nSV = pNr_sv_##suffix;                     \
                                                                    \
              m = pSvmModel->nr_class - 1;                          \
              total_sv_num = pSvmModel->l;                          \
              pSvmModel->sv_coef = TMemAlloc(hMemBuf, sizeof(double *)*m);           \
              pSvmModel->SV = TMemAlloc(hMemBuf, sizeof(svm_node *)*total_sv_num);   \
              if ((TNull == pSvmModel->sv_coef) || (TNull == pSvmModel->SV))         \
              {                                                                      \
                  rVal = -1;                                                         \
                  goto EXIT;                                                         \
              }                                                                      \
                                                                                     \
              for(i=0; i<m; i++)                                                     \
                  pSvmModel->sv_coef[i] = &pCoef_##suffix[i*total_sv_num];           \
              j = 0;                                                                 \
              for(i=0; i<total_sv_num;i++)                                           \
              {                                                                      \
                  pSvmModel->SV[i] = &pSvm_node_##suffix[j];                         \
                  while(1)                                                           \
                  {                                                                  \
                      j++;                                                           \
                      if(-1 == pSvm_node_##suffix[j].index)                          \
					  {                                                              \
					      j++;                                                       \
						  break;                                                     \
		              }                                                              \
                  }                                                                  \
              }                                                                      \
                                                                                     \
              pSvmModel->pMinMaxFeaVal = pMinMaxFeaVal_##suffix;                     \
              pSvmModel->feaLower = i32MinRange_##suffix;                            \
              pSvmModel->feaUpper = i32MaxRange_##suffix;                            \
              rVal = 0;                                                              \
          EXIT:                                                                      \
              if(0 != rVal)                                                          \
              {                                                                      \
                  if(pSvmModel)                                                      \
                  {                                                                  \
                      if(pSvmModel->sv_coef)                                         \
                          TMemFree(hMemBuf, pSvmModel->sv_coef);                     \
                      if(pSvmModel->SV)                                              \
                          TMemFree(hMemBuf, pSvmModel->SV);                          \
                      TMemFree(hMemBuf, pSvmModel);                                  \
                  }                                                                  \
                  pSvmModel = TNull;                                                 \
              }                                                                      \
              return pSvmModel;                                                      \
          }

INIT_SVM_FUNC(face);

/*************************************************************************************/
static int __featureScale(int *pFeaSrc, svm_node *pNode, int *pMinMax, int lower, int upper, int feaLen);
static double __svm_predict(THandle hMemBuf, const svm_model *model, const svm_node *x);
static double __svm_predict_values(THandle hMemBuf, const svm_model *model, const svm_node *x, double* dec_values);
static double k_function(const svm_node *x, const svm_node *y, const svm_parameter* param);
static double dot(const svm_node *px, const svm_node *py);
static double powi(double base, int times);

static double powi(double base, int times)
{
	double tmp = base, ret = 1.0;
    int t;

	for(t=times; t>0; t/=2)
	{
		if(t%2==1) ret*=tmp;
		tmp = tmp * tmp;
	}
	return ret;
}
static double dot(const svm_node *px, const svm_node *py)
{
	double sum = 0;
	while(px->index != -1 && py->index != -1)
	{
		if(px->index == py->index)
		{
			sum += px->value * py->value;
			++px;
			++py;
		}
		else
		{
			if(px->index > py->index)
				++py;
			else
				++px;
		}			
	}
	return sum;
}

svm_model* Init_svm(THandle hMemBuf, char *suffix)
{
    int rVal = 0;
    
    if(TNull == suffix)
        return TNull;

    if(strcmp(suffix,"face") == 0)
        return Init_svm_face(hMemBuf);
    else 
        return TNull;
}

void  Uninit_svm(THandle hMemBuf, svm_model ** ppModel)
{
             
    svm_model *pSvmModel = *ppModel;                                                      
    if(pSvmModel)                                                      
    {                                                                  
        if(pSvmModel->sv_coef)                                         
            TMemFree(hMemBuf, pSvmModel->sv_coef);                     
        if(pSvmModel->SV)                                              
            TMemFree(hMemBuf, pSvmModel->SV);                          
        TMemFree(hMemBuf, pSvmModel);                                  
    }                                                                 
     
    *ppModel = TNull;   
}

static double __svm_predict(THandle hMemBuf, const svm_model *model, const svm_node *x)
{
	int nr_class = model->nr_class;
	double *dec_values;
    double pred_result;
	if(model->param.svm_type == ONE_CLASS ||
	   model->param.svm_type == EPSILON_SVR ||
	   model->param.svm_type == NU_SVR)
		dec_values = TMemAlloc(hMemBuf,sizeof(double));
	else 
		dec_values = TMemAlloc(hMemBuf, sizeof(double)*nr_class*(nr_class-1)/2);
	pred_result = __svm_predict_values(hMemBuf, model, x, dec_values);

    if(dec_values) TMemFree(hMemBuf, dec_values);
	return pred_result;
}

static double k_function(const svm_node *x, const svm_node *y, const svm_parameter* param)
{
	switch(param->kernel_type)
	{
		case LINEAR:
			return dot(x,y);
		case POLY:
			return powi(param->gamma*dot(x,y)+param->coef0,param->degree);
		case RBF:
		{
			double sum = 0;
			while(x->index != -1 && y->index !=-1)
			{
				if(x->index == y->index)
				{
					double d = x->value - y->value;
					sum += d*d;
					++x;
					++y;
				}
				else
				{
					if(x->index > y->index)
					{	
						sum += y->value * y->value;
						++y;
					}
					else
					{
						sum += x->value * x->value;
						++x;
					}
				}
			}

			while(x->index != -1)
			{
				sum += x->value * x->value;
				++x;
			}

			while(y->index != -1)
			{
				sum += y->value * y->value;
				++y;
			}
			
			return exp(-param->gamma*sum);
		}
		case SIGMOID:
			return tanh(param->gamma*dot(x,y)+param->coef0);
		case PRECOMPUTED:  //x: test (validation), y: SV
			return x[(int)(y->value)].value;
		default:
			return 0;  // Unreachable 
	}
}

static double __svm_predict_values(THandle hMemBuf, const svm_model *model, const svm_node *x, double* dec_values)
{
	int i;
	if(model->param.svm_type == ONE_CLASS ||
	   model->param.svm_type == EPSILON_SVR ||
	   model->param.svm_type == NU_SVR)
	{
		double *sv_coef = model->sv_coef[0];
		double sum = 0;
		for(i=0;i<model->l;i++)
			sum += sv_coef[i] * k_function(x,model->SV[i],&model->param);
		sum -= model->rho[0];
		*dec_values = sum;

		if(model->param.svm_type == ONE_CLASS)
			return (sum>0)?1:-1;
		else
			return sum;
	}
	else
	{
		int nr_class = model->nr_class;
		int l = model->l;
		int p=0, j;
		double *kvalue = TNull;
        int *start;
        int *vote;
        int vote_max_idx = 0;

        kvalue =  TMemAlloc(hMemBuf, sizeof(double)*l);
		for(i=0;i<l;i++)
			kvalue[i] = k_function(x,model->SV[i],&model->param);

		start = TMemAlloc(hMemBuf, sizeof(int)*nr_class);
		start[0] = 0;
		for(i=1;i<nr_class;i++)
			start[i] = start[i-1]+model->nSV[i-1];

		vote = TMemAlloc(hMemBuf, sizeof(int)*nr_class);
		for(i=0;i<nr_class;i++)
			vote[i] = 0;
		p=0;
		for(i=0;i<nr_class;i++)
			for(j=i+1;j<nr_class;j++)
			{
				double sum = 0;
				int si = start[i];
				int sj = start[j];
				int ci = model->nSV[i];
				int cj = model->nSV[j];
				
				int k;
				double *coef1 = model->sv_coef[j-1];
				double *coef2 = model->sv_coef[i];
				for(k=0;k<ci;k++)
					sum += coef1[si+k] * kvalue[si+k];
				for(k=0;k<cj;k++)
					sum += coef2[sj+k] * kvalue[sj+k];
				sum -= model->rho[p];
				dec_values[p] = sum;

				if(dec_values[p] > 0)
					++vote[i];
				else
					++vote[j];
				p++;
			}

	    vote_max_idx = 0;
		for(i=1;i<nr_class;i++)
			if(vote[i] > vote[vote_max_idx])
				vote_max_idx = i;
        
        if(kvalue) TMemFree(hMemBuf, kvalue);
        if(start)  TMemFree(hMemBuf, start);
        if(vote)   TMemFree(hMemBuf, vote);
		return model->label[vote_max_idx];
	}
}


static int __featureScale(int *pFeaSrc, svm_node *pNode, int *pMinMax, int lower, int upper, int feaLen)
{
    int i, j, index;
    int i32FeaVal, i32MaxVal, i32MinVal;
    svm_node  node;
    
    if((TNull == pFeaSrc) || (TNull == pNode) || (TNull == pMinMax) )
        return -1;
    
    j = 0;
	index =1;
    for(i=0; i<feaLen; i++)
    {
        i32FeaVal = pFeaSrc[i];
        i32MinVal = pMinMax[i*2];
        i32MaxVal = pMinMax[i*2+1];    

        if(i32MinVal == i32MaxVal)
		{
			index ++;
            continue;
		}
        else if(i32FeaVal == i32MinVal)
            node.value = lower;
        else if(i32FeaVal == i32MaxVal)
            node.value = upper;
        else
            node.value = lower + (upper-lower)*(i32FeaVal-i32MinVal)*1.0/(i32MaxVal-i32MinVal);

		if(0 == node.value)
		{
			index++;
            continue;
		}
        node.index = index;
        pNode[j++] = node;
		index ++;
    }    
    pNode[j].index = -1;
    return 0;
}

int   SvmPredict(THandle hMemBuf, svm_model *pSvmModel, int *pFea, int feaLen, int *label)
{
    int rVal = 0;
    int i,j;
    svm_node *pNode = TNull;
    int *pMinMaxFeaVal = TNull ; 
    int i32Val, minVal, maxVal, feaLower, feaUpper;
    
    pNode = TMemAlloc(hMemBuf, sizeof(svm_node) * (feaLen+1));
    
    if ( 
          (TNull == pSvmModel) || (TNull == pFea) 
        ||(TNull == label) || (TNull == pNode)
       )
    {
        rVal = -1;
        goto EXIT;
    }
    
    // scale first
    pMinMaxFeaVal = pSvmModel->pMinMaxFeaVal;
    feaLower = pSvmModel->feaLower;
    feaUpper = pSvmModel->feaUpper;
    rVal = __featureScale(pFea, pNode, pMinMaxFeaVal, feaLower, feaUpper, feaLen);
    if(0 != rVal)
        goto EXIT;

	//{
	//	FILE *tmp = fopen("../bin/scal_fea.txt", "w");
	//	int i=0;
	//	fprintf(tmp, "%d ", 1);
 //       for(i=0; i<973; i++)
 //       {
	//		if(-1 ==pNode[i].index)
	//			break;
 //           fprintf(tmp, "%d:%g ", pNode[i].index, pNode[i].value);
	//		
 //       }
 //       fprintf(tmp,"\n");
	//	fclose(tmp);			
	//}

    *label = (int)__svm_predict(hMemBuf, pSvmModel,pNode);

    rVal = 0;
 EXIT:
    if(pNode) TMemFree(hMemBuf, pNode);
    return rVal;    
}

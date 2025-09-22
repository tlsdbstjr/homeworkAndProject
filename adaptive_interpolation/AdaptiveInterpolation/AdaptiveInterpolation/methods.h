#ifndef __INTERPOLATION_METHOD__
#define __INTERPOLATION_METHOD__

#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<math.h>
#define FILTER_SZ 7

typedef int Mat_t;
typedef struct _integer_filter
{
	int filterdata[FILTER_SZ][FILTER_SZ];
	int denominator;
}filter;

void** alloc2DArr(unsigned long long typeSize, unsigned long long ArrSize);
void** alloc2DArrRec(unsigned long long typeSize, unsigned long long row, unsigned long long col);
void free2DArr(void** target);
int getImage(char* fileName, int size, unsigned char** img);
void saveImage(char* fileName, int size, unsigned char** img);
int convolution(unsigned char** img, int imgSz, int row, int col, filter* filter);
int convolution_double(unsigned char** img, int imgSz, int row, int col, double* filter);
Mat_t** MatMul(Mat_t** A, int aRow, int aCol, Mat_t** B, int bRow, int bCol);
Mat_t** transposeMat(Mat_t** obj, int row, int col);
Mat_t** Gause_Seidel_Method(Mat_t** coeffcient, int eSZ, Mat_t** constant, int cCol);
Mat_t** Gauss_Jordan_Method(Mat_t** coeffcient, int eSZ, Mat_t** constant, int cCol);
double** Gauss_Jordan_Method_double(Mat_t** coeffcient, int eSZ, Mat_t** constant, int cCol);
double evalInterpole(char** gtImg, char** procImg);


int symCilp(int obj, int min, int max);

#endif
#include "methods.h"
#define _CRT_SECURE_NO_WARNINGS

void** alloc2DArr(unsigned long long typeSize, unsigned long long ArrSize)
{
    void** dst = (void*)malloc(sizeof(void*) * ArrSize);
    if (dst == NULL) return NULL;
    void* tmp = malloc(typeSize * ArrSize * ArrSize);
    if (tmp == NULL)
    {
        free(dst);
        return NULL;
    }
    for (int i = 0; i < ArrSize; i++)
        dst[i] = (void*)((char*)tmp + typeSize * ArrSize * i);
    return dst;
}

void** alloc2DArrRec(unsigned long long typeSize, unsigned long long row, unsigned long long col)
{
    void** dst = (void*)malloc(sizeof(void*) * row);
    if (dst == NULL) return NULL;
    void* tmp = malloc(typeSize * row * col);
    if (tmp == NULL)
    {
        free(dst);
        return NULL;
    }
    for (int i = 0; i < row; i++)
        dst[i] = (void*)((char*)tmp + typeSize * col * i);
    return dst;
}

void free2DArr(void** target)
{
    free(target[0]);
    free(target);
}

int getImage(char* fileName, int size, unsigned char** img)
{
    FILE* inStream = fopen(fileName, "rb");
    if (inStream == NULL)
    {
        fprintf(stderr, "there is no file!: %s\n",fileName);
        return -1;
    }
    for (int i = 0; i < size; fread((char*)img[i++], sizeof(unsigned char), size, inStream));
    fclose(inStream);
    return 0;
}

void saveImage(char* fileName, int size, unsigned char** img)
{
    FILE* outStream = fopen(fileName, "wb");
    if (outStream == NULL)
    {
        fprintf(stderr, "there is no file!: %s\n", fileName);
        return;
    }
    for (int i = 0; i < size; fwrite(img[i++], sizeof(unsigned char), size, outStream));
    fclose(outStream);
    return 0;
}

int convolution(unsigned char** img, int imgSz, int row, int col, filter* filter)
{
    int res = 0;
    for(int i = 0; i < FILTER_SZ; i++)
        for (int j = 0; j < FILTER_SZ; j++)
        {
            res += img[symCilp(row + i - (FILTER_SZ >> 1), 0, imgSz-1)][symCilp(col + j - (FILTER_SZ >> 1), 0, imgSz -1)] * filter->filterdata[i][j];
        }
    res += filter->denominator >> 1;    // round
    res /= filter->denominator;
    return res;
}

int convolution_double(unsigned char** img, int imgSz, int row, int col, double* filter)
{
    double res = 0;
    for (int i = 0; i < FILTER_SZ; i++)
        for (int j = 0; j < FILTER_SZ; j++)
        {
            res += (img[symCilp(row + i - (FILTER_SZ >> 1), 0, imgSz - 1)][symCilp(col + j - (FILTER_SZ >> 1), 0, imgSz - 1)] * filter[i * FILTER_SZ + j]);
        }
    if (res > 256) return 255;
    else if (res < 0) return 0;
    else
    return (unsigned char)res;
}

Mat_t** MatMul(Mat_t** A, int aRow, int aCol, Mat_t** B, int bRow, int bCol)
{
    if (aCol != bRow) return NULL;
    Mat_t** res = alloc2DArrRec(sizeof(Mat_t), aRow, bCol);
    for (int i = 0; i < aRow; i++)
        for (int j = 0; j < bCol; j++)
            for (int k = 0; k < aCol; k++)
                res[i][j] = A[i][k] * B[k][j];
    return res;
}

Mat_t** transposeMat(Mat_t** obj, int row, int col)
{
    Mat_t** res = alloc2DArrRec(sizeof(Mat_t), col, row);
    for (int r = 0; r < row; r++)
        for (int c = 0; c < col; c++)
            res[c][r] = obj[r][c];
    return res;
}

Mat_t** Gause_Seidel_Method(Mat_t** coeffcient, int eSZ, Mat_t** constant, int cCol)
{
    Mat_t** X = alloc2DArrRec(sizeof(Mat_t), eSZ, cCol);
    Mat_t** T = alloc2DArrRec(sizeof(Mat_t), eSZ, eSZ);
    memcpy(&T[0][0], &coeffcient[0][0], sizeof(Mat_t) * eSZ * eSZ);
    for (int i = 0; i < eSZ; i++)
    {
        int tarCoef = T[i][i];
        T[i][i] = 0;
        for (int j = 0; j < eSZ; j++)
        {
            if (j == i) continue;
        }
    }
    // X = T*X - C

    return NULL;
}

** Gauss_Jordan_Method(Mat_t** coeffcient, int eSZ, Mat_t** constant, int cCol)
{
    Mat_t** res = alloc2DArrRec(sizeof(Mat_t), eSZ, cCol);

    return res;
}

double** Gauss_Jordan_Method_double(Mat_t** coeffcient, int eSZ, Mat_t** constant, int cCol)
{
    double** res;
    double** coef = alloc2DArrRec(sizeof(double), eSZ, eSZ + cCol);
    double* maxCoef = malloc(sizeof(double) * eSZ);
    double det = 1;
    memset(maxCoef, 0, sizeof(double) * eSZ);

    // duplicate the coefficent and constant array
    for (int i = 0; i < eSZ; i++)
    {
        for (int j = 0; j < eSZ; j++)
            coef[i][j] = coeffcient[i][j];
        for (int j = 0; j < cCol; j++)
            coef[i][eSZ + j] = constant[i][j];
    }

    printf("\n");
    //set maxCoef
    for (int i = 0; i < eSZ; i++)
    {
        for (int j = 0; j < eSZ; j++)
        {
            double tar = coef[i][j];
            tar *= tar < 0 ? -1 : 1; //get absolute
            if (tar > maxCoef[i]) maxCoef[i] = tar;
        }
    }

    //gauss elimination method start
    for (int i = 0; i < eSZ; i++)
    {
        // find pibot
        int pibot = 0;
        double maxRatio = 0;
        for (int j = i; j < eSZ; j++)
        {
            double tar = coef[j][i];
            double ratio;
            tar *= tar < 0 ? -1 : 1; //get absolute
            ratio = tar / maxCoef[j];
            if (maxRatio < ratio)
            {
                maxRatio = ratio;
                pibot = j;
            }
        }

        // swap rows; pibot row is always upper row
        if (i != pibot);
        { 
            double* tmpRow = coef[i];
            double tmpCoef = maxCoef[i];
            coef[i] = coef[pibot];
            coef[pibot] = tmpRow;
            maxCoef[i] = maxCoef[pibot];
            maxCoef[pibot] = tmpCoef;
            det *= -1;
        }

        // eliminate the pibot's column
        double pibotCoef = coef[i][i];
        for (int nowCol = i; nowCol < eSZ + cCol; nowCol++)
            coef[i][nowCol] /= pibotCoef;
        det *= pibotCoef;
        for (int nowRow = i + 1; nowRow < eSZ; nowRow++)
        {
            pibotCoef = coef[nowRow][i];
            for (int nowCol = 0; nowCol < eSZ + cCol; nowCol++)
                coef[nowRow][nowCol] -= coef[i][nowCol] * pibotCoef;
        }
    }
    free(maxCoef);

    // option: find and print determinant
    printf("dat = %lf\n", det);

    // find root
    res = alloc2DArrRec(sizeof(double), eSZ, cCol);
    for(int resCol = 0; resCol < cCol; resCol++)
        for (int row = eSZ - 1; row >= 0; row--)
        {
            res[row][resCol] = coef[row][eSZ+resCol];
            for (int col = eSZ - 1; col > row; col--)
                res[row][resCol] -= coef[row][col] * res[col][resCol];
            res[row][resCol] /= coef[row][row];
        }

    for (int i = 0; i < 3; i++)
    {
        double sum = 0;
        for (int j = 0; j < eSZ; j++) {
            sum += res[j][i];
            printf("%lf ", res[j][i]);
        }
            printf("sum = %lf\n", sum);
    }

    return res;
}

double evalInterpole(char** gtImg, char** procImg)
{
    //compute Sum of error
    long long SE = 0;
    int maxSig = -1;	// the maximum of signal
    for (int i = 0; i < 512; i++)
        for (int j = 0; j < 512; j++)
        {
            SE += (gtImg[i][j] - procImg[i][j]) * (gtImg[i][j] - procImg[i][j]);
            if (procImg[i][j] > maxSig) maxSig = procImg[i][j];
        }
    double MSE = (double)SE / (512 * 512);
    double PSNR = 20 * log10(maxSig) - 10 * log10(MSE);
    return PSNR;
}

int symCilp(int obj, int min, int max)
{
    if (obj >= min && obj <= max) return obj;
    else if (obj < min) return -obj;
    else return (max << 1) - obj;
}

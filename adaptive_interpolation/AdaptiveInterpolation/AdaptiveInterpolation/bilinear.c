#define _CRT_SECURE_NO_WARNINGS
#include"methods.h"

int main()
{
	unsigned char** oriImg = alloc2DArr(sizeof(unsigned char), 256);
	unsigned char** gtImg = alloc2DArr(sizeof(unsigned char), 512);
	unsigned char** procImg = alloc2DArr(sizeof(unsigned char), 512);

	getImage("dataset\\lr\\Couple_256x256_yuv400_8bit.raw", 256, oriImg);
	getImage("dataset\\gt\\Couple_512x512_yuv400_8bit.raw", 512, gtImg);

	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			procImg[i << 1][j << 1] = oriImg[i][j];

	for (int i = 0; i < 512; i += 2)
	{
		for (int j = 0; j < 510; j += 2)
			procImg[i][j + 1] = (unsigned char)(((int)procImg[i][j] + (int)procImg[i][j + 2]) >> 1);
		procImg[i][511] = procImg[i][510];
	}
	for (int j = 0; j < 512; j++)
	{
		for (int i = 0; i < 510; i += 2)
			procImg[i + 1][j] = (unsigned char)(((int)procImg[i][j] + (int)procImg[i + 2][j]) >> 1);
		procImg[511][j] = procImg[510][j];
	}

	double PSNR = evalInterpole(gtImg, procImg);
	printf("Bilinear PSNR is %lf\n", PSNR);

	saveImage("BilinearRes.raw", 512, procImg);

	free2DArr(oriImg);
	free2DArr(gtImg);
	free2DArr(procImg);

	return 0;
}
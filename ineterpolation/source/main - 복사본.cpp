#include<iostream>
#include<fstream>
#include<string>
#include<cmath>

using namespace std;

const string fileList[3] =
	{
		"Barbara",
		"Couple",
		"Lena"
	};

//string prefix = "dataset\\";
const string prefix = "C:\\Users\\Computer\\Documents\\Visual Studio 2022\\interpolation\\dataset\\";
const string lrPrefix = prefix + "lr\\";
const string gtPrefix = prefix + "gt\\";
const string resPrefix = prefix + "result\\";

const string bigSizePostfix = "_512x512_yuv400_8bit.raw";
const string smallSizePostfix = "_128x128_yuv400_8bit.raw";
//string Postfix = "_yuv400_8bit.raw";

const double LagrangeMat[4][4] =
{
	{(double)-1 / 384, (double) 1 / 128, (double)-1 / 128,(double) 1 / 384},
	{(double) 1 /  16, (double)-5 /  32, (double) 1 /   8,(double)-1 /  32},
	{(double)-11 / 24, (double) 3 /   4, (double)-3 /   8,(double) 1 /  12},
	{               1,                0,                0,               0}
};

const int sixTab[6] = {1, -5, 20, 20, -5, 1};	//scaled by *32, so, /32 is needed.

unsigned char* getBicublicPixel(int position[4], unsigned char imageVector[4])
{
	unsigned char* res = (unsigned char*)malloc(4 * sizeof(unsigned char));
	if (res == NULL) return NULL;
	double LI[4] = { 0, };
	double PLI[4] = { 0, };
	memset(res, 0, 4 * sizeof(unsigned char));
	int positionMat[4][4];
	positionMat[0][3] = 1;
	positionMat[1][3] = 1;
	positionMat[2][3] = 1;
	positionMat[3][3] = 1;
	
	//make position Matrix
	for (int col = 3; col > 0; col--)
		for (int row = 0; row < 4; row++)
			positionMat[row][col - 1] = positionMat[row][col] * position[row];

	//LagrangeMat * imageVector
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			LI[i] += LagrangeMat[i][j] * imageVector[j];

	//positionMat * (LagrangeMat * imageVector)
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			PLI[i] += positionMat[i][j] * LI[j];

	//get integer form
	for (int i = 0; i < 4; i++)
	{
		double cmpNum = round(PLI[i]);
		if (cmpNum > 255) res[i] = 255;
		else if (cmpNum < 0) res[i] = 0;
		else res[i] = (char)cmpNum;
	}
	return res;
}

unsigned char getSixTabConvolution(unsigned char colorVec[6])
{
	int res = 0;
	for (int i = 0; i < 6; i++) res += colorVec[i] * sixTab[i];
	res >>= 5;	// /32
	//check if it's saturated
	if (res > 255) return (unsigned char)255;
	else if (res < 0) return (unsigned char)0;
	else  return (unsigned char)res;
}
unsigned char getSixTabConvolution1(unsigned char colorVec[6])
{
	int res = 0;
	for (int i = 0; i < 6; i++) res += colorVec[i] * sixTab[i];
	res += 16;
	res >>= 5;	// /32
	//check if it's saturated
	if (res > 255) return (unsigned char)255;
	else if (res < 0) return (unsigned char)0;
	else  return (unsigned char)res;
}
unsigned char getSixTabConvolution2(unsigned char colorVec[6])
{
	int res = 0;
	for (int i = 0; i < 6; i++) res += colorVec[i] * sixTab[i];
	res += 512;
	res >>= 10;	// /32
	//check if it's saturated
	if (res > 255) return (unsigned char)255;
	else if (res < 0) return (unsigned char)0;
	else  return (unsigned char)res;
}

unsigned char oriImg[128][128];
unsigned char procImg[512][512];
unsigned char gtImg[512][512];

double evalInterpole(int fileNum)
{
	ifstream fileIn(gtPrefix + fileList[fileNum] + bigSizePostfix, ios::binary | ios::in);
	if (!fileIn.is_open())
	{
		cerr << "file " << (gtPrefix + fileList[fileNum] + bigSizePostfix) << " can't be opened!\n";
		return -1;
	}
	//read file
	for (int i = 0; i < 512; fileIn.read((char*)gtImg[i++], 512));
	fileIn.close();
	int SE = 0;
	int maxSig = -1;
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

void main()
{
	ifstream fileIn;
	ofstream fileOut;

	for (int fileNum = 0; fileNum < 3; fileNum++)
	{
		fileIn.open(lrPrefix + fileList[fileNum] + smallSizePostfix, ios::binary | ios::in);
		//fileOut.open((resPrefix + fileList[fileNum] + smallSizePostfix).c_str(), ios::binary || ios::trunc);
		if (!fileIn.is_open())
		{
			cerr << "file " << (lrPrefix + fileList[fileNum] + smallSizePostfix) << " can't be opened!\n";
			continue;
		}
		//read file
		for (int i = 0; i < 128; fileIn.read((char *)oriImg[i++], 128));
		fileIn.close();

		//Nearest Neighbor Interpolation
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				for (int row = 0; row < 128; row++)
					for (int col = 0; col < 128; col++)
						procImg[(row << 2) + i][(col << 2) + j] = oriImg[row][col];

		fileOut.open((resPrefix + fileList[fileNum] + "_NearestNighbor" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char *)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] <<" at Neareast Neighbor: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		memset(procImg, 0, 512 * 512 * sizeof(unsigned char));

		//bilinear Interpolation
		//interpolation row first
		for(int row = 0 ; row < 128 ; row ++)
		{
			int tmp = (int)oriImg[row][0];
			int nowRow = (row << 2) + 1;
			tmp += ((int)oriImg[row][0] - (int)oriImg[row][1]) >> 2;
			//saturated addition
			if (tmp < 0) procImg[nowRow][0] = 0;
			else if (tmp > 255) procImg[nowRow][0] = (unsigned char)255;
			else procImg[nowRow][0] = tmp;
			for (int col = 0; col < 127; col++)
			{
				procImg[nowRow][(col << 2) + 1] = oriImg[row][col];
				procImg[nowRow][(col << 2) + 3] = (unsigned char)(((int)oriImg[row][col] + (int)oriImg[row][col+1]) >> 1);
				procImg[nowRow][(col << 2) + 2] = (unsigned char)((((int)oriImg[row][col] << 1) + (int)oriImg[row][col] + (int)oriImg[row][col + 1]) >> 2);
				procImg[nowRow][(col << 2) + 4] = (unsigned char)((((int)oriImg[row][col + 1] << 1) + (int)oriImg[row][col + 1] + (int)oriImg[row][col]) >> 2);
			}
			tmp = procImg[nowRow][509] = oriImg[row][127];
			tmp += procImg[nowRow][509] - procImg[nowRow][508];
			if (tmp < 0)
			{
				procImg[nowRow][510] = 0;
				procImg[nowRow][511] = 0;
				continue;
			}
			else if (tmp > 255)
			{
				procImg[nowRow][510] = (unsigned char)255;
				procImg[nowRow][511] = (unsigned char)255;
				continue;
			}
			else procImg[nowRow][510] = tmp;

			tmp += procImg[nowRow][509] - procImg[nowRow][508];
			if (tmp < 0) procImg[nowRow][511] = 0;
			else if (tmp > 255) procImg[nowRow][511] = (unsigned char)255;
			else procImg[nowRow][511] = tmp;
		}
		//then, interpolation column
		for (int col = 0; col < 512; col++)
		{
			int tmp = (int)procImg[1][col];
			tmp += ((int)procImg[1][col] - (int)procImg[4][col]) >> 2;
			//saturated addition
			if (tmp < 0) procImg[0][col] = 0;
			else if (tmp > 255) procImg[0][col] = (unsigned char)255;
			else procImg[0][col] = tmp;
			for (int row = 0; row < 127; row++)
			{
				procImg[(row << 2) + 3][col] = (unsigned char)(((int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 1);
				procImg[(row << 2) + 2][col] = (unsigned char)((((int)procImg[(row << 2) + 1][col] << 1) + (int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 2);
				procImg[(row << 2) + 4][col] = (unsigned char)((((int)procImg[(row << 2) + 5][col] << 1) + (int)procImg[(row << 2) + 5][col] + (int)procImg[(row << 2) + 1][col]) >> 2);
			}
			tmp = procImg[509][col];
			tmp += procImg[509][col] - procImg[508][col];
			if (tmp < 0)
			{
				procImg[510][col] = 0;
				procImg[511][col] = 0;
				continue;
			}
			else if (tmp > 255)
			{
				procImg[510][col] = (unsigned char)255;
				procImg[511][col] = (unsigned char)255;
				continue;
			}
			else procImg[510][col] = tmp;
			tmp += procImg[509][col] - procImg[508][col];
			if (tmp < 0) procImg[511][col] = 0;
			else if (tmp > 255) procImg[511][col] = (unsigned char)255;
			else procImg[511][col] = tmp;
		}
		fileOut.open((resPrefix + fileList[fileNum] + "_bilinear" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at bilinear interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		memset(procImg, 0, 512 * 512 * sizeof(unsigned char));

		//bilinear Interpolation
		//interpolation row first
		for (int row = 0; row < 128; row++)
		{
			int nowRow = (row << 2) + 1;
			procImg[nowRow][0] = oriImg[row][0] >> 1;
			for (int col = 0; col < 127; col++)
			{
				procImg[nowRow][(col << 2) + 1] = oriImg[row][col];
				procImg[nowRow][(col << 2) + 3] = (unsigned char)(((int)oriImg[row][col] + (int)oriImg[row][col + 1]) >> 1);
				procImg[nowRow][(col << 2) + 2] = (unsigned char)((((int)oriImg[row][col] << 1) + (int)oriImg[row][col] + (int)oriImg[row][col + 1]) >> 2);
				procImg[nowRow][(col << 2) + 4] = (unsigned char)((((int)oriImg[row][col + 1] << 1) + (int)oriImg[row][col + 1] + (int)oriImg[row][col]) >> 2);
			}
			procImg[nowRow][509] = oriImg[row][127];
			procImg[nowRow][511] = (unsigned char)((int)oriImg[row][127] >> 1);
			procImg[nowRow][510] = (unsigned char)((((int)oriImg[row][127] << 1) + (int)oriImg[row][127]) >> 2);

		}
		//then, interpolation column
		for (int col = 0; col < 512; col++)
		{
			procImg[0][col] = procImg[1][col] >> 1;
			for (int row = 0; row < 127; row++)
			{
				procImg[(row << 2) + 3][col] = (unsigned char)(((int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 1);
				procImg[(row << 2) + 2][col] = (unsigned char)((((int)procImg[(row << 2) + 1][col] << 1) + (int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 2);
				procImg[(row << 2) + 4][col] = (unsigned char)((((int)procImg[(row << 2) + 5][col] << 1) + (int)procImg[(row << 2) + 5][col] + (int)procImg[(row << 2) + 1][col]) >> 2);
			}
			procImg[511][col] = (unsigned char)((int)procImg[509][col] >> 1);
			procImg[510][col] = (unsigned char)((((int)procImg[509][col] << 1) + (int)procImg[509][col]) >> 2);
		}
		fileOut.open((resPrefix + fileList[fileNum] + "_bilinear2" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at bilinear interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		memset(procImg, 0, 512 * 512 * sizeof(unsigned char));

		//bilinear Interpolation
		//interpolation row first
		for (int row = 0; row < 128; row++)
		{
			int nowRow = (row << 2) + 1;
			procImg[nowRow][0] = oriImg[row][0];
			for (int col = 0; col < 127; col++)
			{
				procImg[nowRow][(col << 2) + 1] = oriImg[row][col];
				procImg[nowRow][(col << 2) + 3] = (unsigned char)(((int)oriImg[row][col] + (int)oriImg[row][col + 1]) >> 1);
				procImg[nowRow][(col << 2) + 2] = (unsigned char)((((int)oriImg[row][col] << 1) + (int)oriImg[row][col] + (int)oriImg[row][col + 1]) >> 2);
				procImg[nowRow][(col << 2) + 4] = (unsigned char)((((int)oriImg[row][col + 1] << 1) + (int)oriImg[row][col + 1] + (int)oriImg[row][col]) >> 2);
			}
			procImg[nowRow][511] = procImg[nowRow][510] = procImg[nowRow][509] = oriImg[row][127];
		}
		//then, interpolation column
		for (int col = 0; col < 512; col++)
		{
			procImg[0][col] = procImg[1][col];
			for (int row = 0; row < 127; row++)
			{
				procImg[(row << 2) + 3][col] = (unsigned char)(((int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 1);
				procImg[(row << 2) + 2][col] = (unsigned char)((((int)procImg[(row << 2) + 1][col] << 1) + (int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 2);
				procImg[(row << 2) + 4][col] = (unsigned char)((((int)procImg[(row << 2) + 5][col] << 1) + (int)procImg[(row << 2) + 5][col] + (int)procImg[(row << 2) + 1][col]) >> 2);
			}
			procImg[511][col] = procImg[510][col] = procImg[509][col];
		}
		fileOut.open((resPrefix + fileList[fileNum] + "_bilinear3" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at bilinear interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		memset(procImg, 0, 512 * 512 * sizeof(unsigned char));

		//bicubic interpolation
		//set representative pixels at procImg
		for (int row = 0; row < 128; row++)
			for (int col = 0; col < 128; col++)
				procImg[(row << 2) + 1][(col << 2) + 1] = oriImg[row][col];

		//row interpolation first
		for (int row = 0; row < 128; row++)
		{
			int nowProcRow = (row << 2) + 1;
			int positions[4] = { 5,6,7,3 };
			{
				unsigned char target[4] = { oriImg[row][0],oriImg[row][0], oriImg[row][1], oriImg[row][2] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[nowProcRow][0] = interpolated[3];
				memcpy(&procImg[nowProcRow][2], interpolated, 3 * sizeof(unsigned char));
				free(interpolated);
			}
			for (int col = 0; col < 125; col++)
			{
				unsigned char* target = &oriImg[row][col];
				unsigned char* interpolated = getBicublicPixel(positions, target);
				memcpy(&procImg[nowProcRow][(col << 2) + 6], interpolated, 3 * sizeof(unsigned char));
				free(interpolated);
			}
			{
				unsigned char target[4] = { oriImg[row][125],oriImg[row][126], oriImg[row][127], oriImg[row][127] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				memcpy(&procImg[nowProcRow][506], interpolated, 3 * sizeof(unsigned char));
				free(interpolated);
			}
			{
				unsigned char target[4] = { oriImg[row][126],oriImg[row][127], oriImg[row][127], oriImg[row][127] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				memcpy(&procImg[nowProcRow][510], interpolated, 2 * sizeof(unsigned char));
				free(interpolated);
			}
		}

		//column interpolation second
		for (int col = 0; col < 512; col++)
		{
			int positions[4] = { 5,6,7,3 };
			{
				unsigned char target[4] = { procImg[1][col],procImg[1][col], procImg[5][col], procImg[9][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[0][col] = interpolated[3];
				procImg[2][col] = interpolated[0];
				procImg[3][col] = interpolated[1];
				procImg[4][col] = interpolated[2];
				free(interpolated);
			}
			for (int row = 1; row < 500; row += 4)
			{
				unsigned char target[4] = { procImg[row][col],procImg[row + 4][col], procImg[row + 8][col], procImg[row + 12][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[row+5][col] = interpolated[0];
				procImg[row+6][col] = interpolated[1];
				procImg[row+7][col] = interpolated[2];
				free(interpolated);
			}
			{
				unsigned char target[4] = { procImg[501][col],procImg[505][col], procImg[509][col], procImg[509][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[506][col] = interpolated[0];
				procImg[507][col] = interpolated[1];
				procImg[508][col] = interpolated[2];
				free(interpolated);
			}
			{
				unsigned char target[4] = { procImg[505][col],procImg[509][col], procImg[509][col], procImg[509][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[510][col] = interpolated[0];
				procImg[511][col] = interpolated[1];
				free(interpolated);
			}
		}
		fileOut.open((resPrefix + fileList[fileNum] + "_bicubic" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at bicubic interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		memset(procImg, 0, 512 * 512 * sizeof(unsigned char));

		//bicubic interpolation
		//set representative pixels at procImg
		for (int row = 0; row < 128; row++)
			for (int col = 0; col < 128; col++)
				procImg[(row << 2) + 1][(col << 2) + 1] = oriImg[row][col];

		//row interpolation first
		for (int row = 0; row < 128; row++)
		{
			int nowProcRow = (row << 2) + 1;
			int positions[4] = { 5,6,7,3 };
			{
				unsigned char target[4] = { oriImg[row][0],oriImg[row][0], oriImg[row][1], oriImg[row][2] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[nowProcRow][0] = interpolated[3];
				memcpy(&procImg[nowProcRow][2], interpolated, 3 * sizeof(unsigned char));
				free(interpolated);
			}
			for (int col = 0; col < 125; col++)
			{
				unsigned char* target = &oriImg[row][col];
				unsigned char* interpolated = getBicublicPixel(positions, target);
				memcpy(&procImg[nowProcRow][(col << 2) + 6], interpolated, 3 * sizeof(unsigned char));
				free(interpolated);
			}
			{
				unsigned char target[4] = { oriImg[row][125],oriImg[row][126], oriImg[row][127], oriImg[row][127] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				memcpy(&procImg[nowProcRow][506], interpolated, 3 * sizeof(unsigned char));
				free(interpolated);
			}
			{
				unsigned char target[4] = { oriImg[row][126],oriImg[row][127], oriImg[row][127], oriImg[row][126] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				memcpy(&procImg[nowProcRow][510], interpolated, 2 * sizeof(unsigned char));
				free(interpolated);
			}
		}

		//column interpolation second
		for (int col = 0; col < 512; col++)
		{
			int positions[4] = { 5,6,7,3 };
			{
				unsigned char target[4] = { procImg[1][col],procImg[1][col], procImg[5][col], procImg[9][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[0][col] = interpolated[3];
				procImg[2][col] = interpolated[0];
				procImg[3][col] = interpolated[1];
				procImg[4][col] = interpolated[2];
				free(interpolated);
			}
			for (int row = 1; row < 500; row += 4)
			{
				unsigned char target[4] = { procImg[row][col],procImg[row + 4][col], procImg[row + 8][col], procImg[row + 12][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[row + 5][col] = interpolated[0];
				procImg[row + 6][col] = interpolated[1];
				procImg[row + 7][col] = interpolated[2];
				free(interpolated);
			}
			{
				unsigned char target[4] = { procImg[501][col],procImg[505][col], procImg[509][col], procImg[509][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[506][col] = interpolated[0];
				procImg[507][col] = interpolated[1];
				procImg[508][col] = interpolated[2];
				free(interpolated);
			}
			{
				unsigned char target[4] = { procImg[505][col],procImg[509][col], procImg[509][col], procImg[505][col] };
				unsigned char* interpolated = getBicublicPixel(positions, target);
				procImg[510][col] = interpolated[0];
				procImg[511][col] = interpolated[1];
				free(interpolated);
			}
		}
		fileOut.open((resPrefix + fileList[fileNum] + "_bicubic2" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at bicubic interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		memset(procImg, 0, 512 * 512 * sizeof(unsigned char));

		//six-tab interpolation
		//set representative pixels at procImg
		for (int row = 0; row < 128; row++)
			for (int col = 0; col < 128; col++)
				procImg[(row << 2) + 1][(col << 2) + 1] = oriImg[row][col];
		for (int row = 0; row < 128; row++)
		{
			int nowRow = (row << 2) + 1;
			{
				unsigned char target[6] = { oriImg[row][1], oriImg[row][0], oriImg[row][0], oriImg[row][1], oriImg[row][2], oriImg[row][3] };
				procImg[nowRow][3] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { oriImg[row][0], oriImg[row][0], oriImg[row][1], oriImg[row][2], oriImg[row][3], oriImg[row][4] };
				procImg[nowRow][7] = getSixTabConvolution(target);
			}
			for (int col = 0; col < 123; col++)
			{
				procImg[nowRow][(col << 2) + 11] = getSixTabConvolution(&oriImg[row][col]);
			}
			{
				unsigned char target[6] = { oriImg[row][123], oriImg[row][124], oriImg[row][125], oriImg[row][126], oriImg[row][127], oriImg[row][127] };
				procImg[nowRow][503] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { oriImg[row][124], oriImg[row][125], oriImg[row][126], oriImg[row][127], oriImg[row][127], oriImg[row][126] };
				procImg[nowRow][507] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { oriImg[row][125], oriImg[row][126], oriImg[row][127], oriImg[row][127], oriImg[row][126], oriImg[row][125] };
				procImg[nowRow][511] = getSixTabConvolution(target);
			}
		}
		for (int col = 1; col < 512; col += 2)
		{
			{
				unsigned char target[6] = { procImg[5][col], procImg[1][col], procImg[1][col], procImg[5][col], procImg[9][col], procImg[13][col] };
				procImg[3][col] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[1][col], procImg[1][col], procImg[5][col], procImg[9][col], procImg[13][col], procImg[17][col] };
				procImg[7][col] = getSixTabConvolution(target);
			}
			for (int row = 1; row < 492; row += 4)
			{
				unsigned char target[6] = { procImg[row][col], procImg[row + 4][col], procImg[row + 8][col], procImg[row + 12][col], procImg[row + 16][col], procImg[row + 20][col] };
				procImg[row + 10][col] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[493][col], procImg[497][col], procImg[501][col], procImg[505][col], procImg[509][col], procImg[509][col] };
				procImg[503][col] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[497][col], procImg[501][col], procImg[505][col], procImg[509][col], procImg[509][col], procImg[505][col] };
				procImg[507][col] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[501][col], procImg[505][col], procImg[509][col], procImg[509][col], procImg[505][col], procImg[501][col] };
				procImg[511][col] = getSixTabConvolution(target);
			}
		}
		
		for (int row = 3; row < 512; row += 4)
		{
			{
				unsigned char target[6] = { procImg[row][5], procImg[row][1], procImg[row][1], procImg[row][5], procImg[row][9], procImg[row][13] };
				procImg[row][3] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[row][1], procImg[row][1], procImg[row][5], procImg[row][9], procImg[row][13], procImg[row][17] };
				procImg[row][7] = getSixTabConvolution(target);
			}
			for (int col = 1; col < 492; col += 4)
			{
				unsigned char target[6] = { procImg[row][col], procImg[row][col + 4], procImg[row][col + 8], procImg[row][col + 12], procImg[row][col + 16], procImg[row][col + 20] };
				procImg[row][col + 10] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[row][493], procImg[row][497], procImg[row][501], procImg[row][505], procImg[row][509], procImg[row][509] };
				procImg[row][503] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[row][497], procImg[row][501], procImg[row][505], procImg[row][509], procImg[row][509], procImg[row][505] };
				procImg[row][507] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[row][501], procImg[row][505], procImg[row][509], procImg[row][509], procImg[row][505], procImg[row][501] };
				procImg[row][511] = getSixTabConvolution(target);
			}
		}
		
		for (int row = 1; row < 512; row += 2)
		{
			procImg[row][0] = procImg[row][1];
			for (int col = 2; col < 512; col += 2)
				procImg[row][col] = (procImg[row][col - 1] + procImg[row][col + 1]) >> 1;
		}
		for (int col = 0; col < 512; col++)
		{
			procImg[0][col] = procImg[1][col];
			for (int row = 2; row < 512; row += 2)
				procImg[row][col] = (procImg[row - 1][col] + procImg[row + 1][col]) >> 1;
		}
		fileOut.open((resPrefix + fileList[fileNum] + "_sixtab" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at six-tab interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
	}
	//six-tab interpolation
//set representative pixels at procImg
	for (int row = 0; row < 128; row++)
		for (int col = 0; col < 128; col++)
			procImg[(row << 2) + 1][(col << 2) + 1] = oriImg[row][col];
	for (int row = 0; row < 128; row++)
	{
		int nowRow = (row << 2) + 1;
		{
			unsigned char target[6] = { oriImg[row][0], oriImg[row][0], oriImg[row][0], oriImg[row][1], oriImg[row][2], oriImg[row][3] };
			procImg[nowRow][3] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { oriImg[row][0], oriImg[row][0], oriImg[row][1], oriImg[row][2], oriImg[row][3], oriImg[row][4] };
			procImg[nowRow][7] = getSixTabConvolution(target);
		}
		for (int col = 0; col < 123; col++)
		{
			procImg[nowRow][(col << 2) + 11] = getSixTabConvolution(&oriImg[row][col]);
		}
		{
			unsigned char target[6] = { oriImg[row][123], oriImg[row][124], oriImg[row][125], oriImg[row][126], oriImg[row][127], oriImg[row][127] };
			procImg[nowRow][503] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { oriImg[row][124], oriImg[row][125], oriImg[row][126], oriImg[row][127], oriImg[row][127], oriImg[row][127] };
			procImg[nowRow][507] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { oriImg[row][125], oriImg[row][126], oriImg[row][127], oriImg[row][127], oriImg[row][127], oriImg[row][127] };
			procImg[nowRow][511] = getSixTabConvolution(target);
		}
	}
	for (int col = 1; col < 512; col += 2)
	{
		{
			unsigned char target[6] = { procImg[1][col], procImg[1][col], procImg[1][col], procImg[5][col], procImg[9][col], procImg[13][col] };
			procImg[3][col] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[1][col], procImg[1][col], procImg[5][col], procImg[9][col], procImg[13][col], procImg[17][col] };
			procImg[7][col] = getSixTabConvolution(target);
		}
		for (int row = 1; row < 492; row += 4)
		{
			unsigned char target[6] = { procImg[row][col], procImg[row + 4][col], procImg[row + 8][col], procImg[row + 12][col], procImg[row + 16][col], procImg[row + 20][col] };
			procImg[row + 10][col] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[493][col], procImg[497][col], procImg[501][col], procImg[505][col], procImg[509][col], procImg[509][col] };
			procImg[503][col] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[497][col], procImg[501][col], procImg[505][col], procImg[509][col], procImg[509][col], procImg[509][col] };
			procImg[507][col] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[501][col], procImg[505][col], procImg[509][col], procImg[509][col], procImg[509][col], procImg[509][col] };
			procImg[511][col] = getSixTabConvolution(target);
		}
	}

	for (int row = 3; row < 512; row += 4)
	{
		{
			unsigned char target[6] = { procImg[row][1], procImg[row][1], procImg[row][1], procImg[row][5], procImg[row][9], procImg[row][13] };
			procImg[row][3] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[row][1], procImg[row][1], procImg[row][5], procImg[row][9], procImg[row][13], procImg[row][17] };
			procImg[row][7] = getSixTabConvolution(target);
		}
		for (int col = 1; col < 492; col += 4)
		{
			unsigned char target[6] = { procImg[row][col], procImg[row][col + 4], procImg[row][col + 8], procImg[row][col + 12], procImg[row][col + 16], procImg[row][col + 20] };
			procImg[row][col + 10] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[row][493], procImg[row][497], procImg[row][501], procImg[row][505], procImg[row][509], procImg[row][509] };
			procImg[row][503] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[row][497], procImg[row][501], procImg[row][505], procImg[row][509], procImg[row][509], procImg[row][509] };
			procImg[row][507] = getSixTabConvolution(target);
		}
		{
			unsigned char target[6] = { procImg[row][501], procImg[row][505], procImg[row][509], procImg[row][509], procImg[row][509], procImg[row][509] };
			procImg[row][511] = getSixTabConvolution(target);
		}
	}

	for (int row = 1; row < 512; row += 2)
	{
		procImg[row][0] = procImg[row][1];
		for (int col = 2; col < 512; col += 2)
			procImg[row][col] = (procImg[row][col - 1] + procImg[row][col + 1]) >> 1;
	}
	for (int col = 0; col < 512; col++)
	{
		procImg[0][col] = procImg[1][col];
		for (int row = 2; row < 512; row += 2)
			procImg[row][col] = (procImg[row - 1][col] + procImg[row + 1][col]) >> 1;
	}
	fileOut.open((resPrefix + fileList[fileNum] + "_sixtab2" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
	for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
	fileOut.close();
	cout << fileList[fileNum] << " at six-tab interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
}
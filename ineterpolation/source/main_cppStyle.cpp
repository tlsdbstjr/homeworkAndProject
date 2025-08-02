#include<stdio.h>
#include<string.h>
#include<math.h>
/*
#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
*/
/* CHANGE FOLLOWINGS: THE FILE NAME */
const string fileList[3] =
	{
		"Barbara",
		"Couple",
		"Lena"
	};

/* CHANGE FOLLOWINGS: THE DIRECTORY */
const string prefix = "dataset\\";	// relative address at windows style
//const string prefix = "E:\\NM\\HW02\\dataset\\";	// absolute address at windows style
// windows style folder name
const string lrPrefix = prefix + "lr\\";
const string gtPrefix = prefix + "gt\\";
const string resPrefix = prefix + "result\\";

/* CHANGE FOLLOWINGS: THE POSTFIX OF FILENAME */
// Postfixes
const string bigSizePostfix = "_512x512_yuv400_8bit.raw";
const string smallSizePostfix = "_128x128_yuv400_8bit.raw";
//string Postfix = "_yuv400_8bit.raw";

// Matrix for Lagrange interpolation
const double LagrangeMat[4][4] =
{
	{(double)-1 / 384, (double) 1 / 128, (double)-1 / 128, (double) 1 / 384},
	{(double) 1 /  16, (double)-5 /  32, (double) 1 /   8, (double)-1 /  32},
	{(double)-11 / 24, (double) 3 /   4, (double)-3 /   8, (double) 1 /  12},
	{               1,                0,                0,                0}
};

// six tab fliter for six tab interpolation
const int sixTab[6] = {1, -5, 20, 20, -5, 1};	//scaled by *32, so, /32 is needed.

// compute the matrix multiplication, which is pointMatrix * LagrangeMat * weights = interpolated point
unsigned char* getBicublicPixel(int* position, int positionCnt,  unsigned char imageVector[4])
{
	unsigned char* res = new unsigned char[positionCnt];
	double LI[4] = { 0, };
	double PLI[4] = { 0, };
	//memset(res, 0, 4 * sizeof(unsigned char));
	fill(res, res + positionCnt, 0);
	int positionMat[4][4];
	positionMat[0][3] = 1;
	positionMat[1][3] = 1;
	positionMat[2][3] = 1;
	positionMat[3][3] = 1;
	
	//make position Matrix
	for (int col = 3; col > 0; col--)
		for (int row = 0; row < positionCnt; row++)
			positionMat[row][col - 1] = positionMat[row][col] * position[row];

	//LagrangeMat * imageVector
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			LI[i] += LagrangeMat[i][j] * imageVector[j];

	//positionMat * (LagrangeMat * imageVector)
	for (int i = 0; i < positionCnt; i++)
		for (int j = 0; j < 4; j++)
			PLI[i] += positionMat[i][j] * LI[j];

	//get integer form and check if it's saturated
	for (int i = 0; i < positionCnt; i++)
	{
		double cmpNum = round(PLI[i]);
		if (cmpNum >= 255) res[i] = 255;
		else if (cmpNum <= 0) res[i] = 0;
		else res[i] = (unsigned char)cmpNum;
	}
	return res;
}

unsigned char getSixTabConvolution(unsigned char colorVec[6])
{
	int res = 0;
	for (int i = 0; i < 6; i++) res += colorVec[i] * sixTab[i];
	res += 47;	// bias to get more correct data for 32, and to round for 15
	res >>= 5;	// /32
	//check if it's saturated
	if (res > 255) return (unsigned char)255;
	else if (res < 0) return (unsigned char)0;
	else  return (unsigned char)res;
}

// images at data area
unsigned char oriImg[128][128];		// origianl image
unsigned char procImg[512][512];	// processed image
unsigned char gtImg[512][512];		// ground truth image

// get the PSNR
double evalInterpole(int fileNum)
{
	// open ground truth file
	ifstream fileIn((gtPrefix + fileList[fileNum] + bigSizePostfix).c_str(), ios::binary);
	if (!fileIn.is_open())
	{
		cerr << "file " << (gtPrefix + fileList[fileNum] + bigSizePostfix) << " can't be opened!\n";
		return -1;
	}
	//read file
	for (int i = 0; i < 512; fileIn.read((char*)gtImg[i++], 512));
	fileIn.close();

	//compute Sum of error
	int SE = 0;
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

int main()
{
	ifstream fileIn;
	ofstream fileOut;

	for (int fileNum = 0; fileNum < 3; fileNum++)
	{
		fileIn.open((lrPrefix + fileList[fileNum] + smallSizePostfix).c_str(), ios::binary);
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
		//store and compute the PSNR
		fileOut.open((resPrefix + fileList[fileNum] + "_NearestNighbor" + bigSizePostfix).c_str(), ios::binary | ios::trunc);
		for (int i = 0; i < 512; fileOut.write((char *)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] <<" at Neareast Neighbor: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		//memset(procImg, 0, 512 * 512 * sizeof(unsigned char));
		fill(&procImg[0][0], &procImg[511][511] + 1, 0);

		//bilinear Interpolation
		//interpolation row first
		for (int row = 0; row < 128; row++)
		{
			int nowRow = (row << 2) + 1;
			procImg[nowRow][0] = oriImg[row][0];	// side processing
			// main processing
			for (int col = 0; col < 127; col++)
			{
				procImg[nowRow][(col << 2) + 1] = oriImg[row][col];
				procImg[nowRow][(col << 2) + 3] = (unsigned char)(((int)oriImg[row][col] + (int)oriImg[row][col + 1]) >> 1);
				procImg[nowRow][(col << 2) + 2] = (unsigned char)((((int)oriImg[row][col] << 1) + (int)oriImg[row][col] + (int)oriImg[row][col + 1]) >> 2);
				procImg[nowRow][(col << 2) + 4] = (unsigned char)((((int)oriImg[row][col + 1] << 1) + (int)oriImg[row][col + 1] + (int)oriImg[row][col]) >> 2);
			}
			procImg[nowRow][511] = procImg[nowRow][510] = procImg[nowRow][509] = oriImg[row][127]; // side processing
		}
		//then, interpolation column
		for (int col = 0; col < 512; col++)
		{
			procImg[0][col] = procImg[1][col];	// side processing
			// main processing
			for (int row = 0; row < 127; row++)
			{
				procImg[(row << 2) + 3][col] = (unsigned char)(((int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 1);
				procImg[(row << 2) + 2][col] = (unsigned char)((((int)procImg[(row << 2) + 1][col] << 1) + (int)procImg[(row << 2) + 1][col] + (int)procImg[(row << 2) + 5][col]) >> 2);
				procImg[(row << 2) + 4][col] = (unsigned char)((((int)procImg[(row << 2) + 5][col] << 1) + (int)procImg[(row << 2) + 5][col] + (int)procImg[(row << 2) + 1][col]) >> 2);
			}
			procImg[511][col] = procImg[510][col] = procImg[509][col];	//side processing
		}
		// save the result
		fileOut.open((resPrefix + fileList[fileNum] + "_bilinear" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at bilinear interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		//memset(procImg, 0, 512 * 512 * sizeof(unsigned char));
		fill(&procImg[0][0], &procImg[511][511] + 1, 0);

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
			// side processing
			{
				unsigned char target[4] = { oriImg[row][0],oriImg[row][0], oriImg[row][1], oriImg[row][2] };
				unsigned char* interpolated = getBicublicPixel(positions, 4, target);
				procImg[nowProcRow][0] = interpolated[3];
				copy(interpolated, interpolated + 3, &procImg[nowProcRow][2]);
				delete[] interpolated;
			}
			// main processing
			for (int col = 0; col < 125; col++)
			{
				unsigned char* target = &oriImg[row][col];
				unsigned char* interpolated = getBicublicPixel(positions, 3, target);
				copy(interpolated, interpolated + 3, &procImg[nowProcRow][(col << 2) + 6]);
				delete[] interpolated;
			}
			// sdie processings
			{
				unsigned char target[4] = { oriImg[row][125],oriImg[row][126], oriImg[row][127], oriImg[row][127] };
				unsigned char* interpolated = getBicublicPixel(positions, 3, target);
				copy(interpolated, interpolated + 3, &procImg[nowProcRow][506]);
				delete[] interpolated;
			}
			{
				unsigned char target[4] = { oriImg[row][126],oriImg[row][127], oriImg[row][127], oriImg[row][126] };
				unsigned char* interpolated = getBicublicPixel(positions, 2, target);
				copy(interpolated, interpolated + 2, &procImg[nowProcRow][510]);
				delete[] interpolated;
			}
		}

		//column interpolation second
		for (int col = 0; col < 512; col++)
		{
			int positions[4] = { 5,6,7,3 };
			// side processing
			{
				unsigned char target[4] = { procImg[1][col],procImg[1][col], procImg[5][col], procImg[9][col] };
				unsigned char* interpolated = getBicublicPixel(positions, 4, target);
				procImg[0][col] = interpolated[3];
				procImg[2][col] = interpolated[0];
				procImg[3][col] = interpolated[1];
				procImg[4][col] = interpolated[2];
				delete[] interpolated;
			}
			// main processing
			for (int row = 1; row < 500; row += 4)
			{
				unsigned char target[4] = { procImg[row][col],procImg[row + 4][col], procImg[row + 8][col], procImg[row + 12][col] };
				unsigned char* interpolated = getBicublicPixel(positions, 3, target);
				procImg[row + 5][col] = interpolated[0];
				procImg[row + 6][col] = interpolated[1];
				procImg[row + 7][col] = interpolated[2];
				delete[] interpolated;
			}
			// side processing
			{
				unsigned char target[4] = { procImg[501][col],procImg[505][col], procImg[509][col], procImg[509][col] };
				unsigned char* interpolated = getBicublicPixel(positions, 3, target);
				procImg[506][col] = interpolated[0];
				procImg[507][col] = interpolated[1];
				procImg[508][col] = interpolated[2];
				delete[] interpolated;
			}
			{
				unsigned char target[4] = { procImg[505][col],procImg[509][col], procImg[509][col], procImg[505][col] };
				unsigned char* interpolated = getBicublicPixel(positions, 2, target);
				procImg[510][col] = interpolated[0];
				procImg[511][col] = interpolated[1];
				delete[] interpolated;
			}
		}
		// save result
		fileOut.open((resPrefix + fileList[fileNum] + "_bicubic" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at bicubic interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
		fill(&procImg[0][0], &procImg[511][511] + 1, 0);

		//six-tab interpolation
		//set representative pixels at procImg
		for (int row = 0; row < 128; row++)
			for (int col = 0; col < 128; col++)
				procImg[(row << 2) + 1][(col << 2) + 1] = oriImg[row][col];
		// row existing process first
		for (int row = 0; row < 128; row++)
		{
			int nowRow = (row << 2) + 1;
			// side processing
			{
				unsigned char target[6] = { oriImg[row][1], oriImg[row][0], oriImg[row][0], oriImg[row][1], oriImg[row][2], oriImg[row][3] };
				procImg[nowRow][3] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { oriImg[row][0], oriImg[row][0], oriImg[row][1], oriImg[row][2], oriImg[row][3], oriImg[row][4] };
				procImg[nowRow][7] = getSixTabConvolution(target);
			}
			// main processing
			for (int col = 0; col < 123; col++)
			{
				procImg[nowRow][(col << 2) + 11] = getSixTabConvolution(&oriImg[row][col]);
			}
			// side processing
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
		// column existing process second
		for (int col = 1; col < 512; col += 2)
		{
			// side processing
			{
				unsigned char target[6] = { procImg[5][col], procImg[1][col], procImg[1][col], procImg[5][col], procImg[9][col], procImg[13][col] };
				procImg[3][col] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[1][col], procImg[1][col], procImg[5][col], procImg[9][col], procImg[13][col], procImg[17][col] };
				procImg[7][col] = getSixTabConvolution(target);
			}
			// main processing
			for (int row = 1; row < 492; row += 4)
			{
				unsigned char target[6] = { procImg[row][col], procImg[row + 4][col], procImg[row + 8][col], procImg[row + 12][col], procImg[row + 16][col], procImg[row + 20][col] };
				procImg[row + 10][col] = getSixTabConvolution(target);
			}
			// side processing
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
		// row generated process third
		for (int row = 3; row < 512; row += 4)
		{
			// side processing
			{
				unsigned char target[6] = { procImg[row][5], procImg[row][1], procImg[row][1], procImg[row][5], procImg[row][9], procImg[row][13] };
				procImg[row][3] = getSixTabConvolution(target);
			}
			{
				unsigned char target[6] = { procImg[row][1], procImg[row][1], procImg[row][5], procImg[row][9], procImg[row][13], procImg[row][17] };
				procImg[row][7] = getSixTabConvolution(target);
			}
			// main processing
			for (int col = 1; col < 492; col += 4)
			{
				unsigned char target[6] = { procImg[row][col], procImg[row][col + 4], procImg[row][col + 8], procImg[row][col + 12], procImg[row][col + 16], procImg[row][col + 20] };
				procImg[row][col + 10] = getSixTabConvolution(target);
			}
			// side processing
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
		// filling empty pixels
		// row filling first
		for (int row = 1; row < 512; row += 2)
		{
			procImg[row][0] = procImg[row][1];	// side processing
			// main processing
			for (int col = 2; col < 512; col += 2)
				procImg[row][col] = (unsigned char)(((unsigned int)procImg[row][col - 1] + procImg[row][col + 1]) >> 1);
		}
		// column filling second
		for (int col = 0; col < 512; col++)
		{
			procImg[0][col] = procImg[1][col];	// side processing
			// main processing
			for (int row = 2; row < 512; row += 2)
				procImg[row][col] = (unsigned char)(((unsigned int)procImg[row - 1][col] + procImg[row + 1][col]) >> 1);
		}
		// save the result
		fileOut.open((resPrefix + fileList[fileNum] + "_sixtab" + bigSizePostfix).c_str(), ios::binary | ios::trunc | ios::out);
		for (int i = 0; i < 512; fileOut.write((char*)procImg[i++], 512));
		fileOut.close();
		cout << fileList[fileNum] << " at six-tab interpolation: the PSNR is " << evalInterpole(fileNum) << "dB.\n";
	}
	return 0;
}
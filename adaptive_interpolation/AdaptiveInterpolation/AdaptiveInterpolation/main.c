#define _CRT_SECURE_NO_WARNINGS
#include"methods.h"
#include"LMQuant.h"

// use just one
#define CLASSIFY_BY_VECTOR
//#define CLASSIFY_BY_STATICS

typedef struct _coordinate {
	unsigned char row;
	unsigned char col;
}coordinate;

const filter Vertical =
{
	{
		{1,2,2,0,-2,-2,-1},
		{1,2,3,0,-3,-2,-1},
		{1,3,4,0,-4,-3,-1},
		{1,4,5,0,-5,-4,-1},
		{1,3,4,0,-4,-3,-1},
		{1,2,3,0,-3,-2,-1},
		{1,2,2,0,-2,-2,-1}
	},
	1
};

const filter Horizontal =
{
	{
		{-1,-1,-1,-1,-1,-1,-1},
		{-2,-2,-3,-4,-3,-2,-2},
		{-2,-3,-4,-5,-4,-3,-2},
		{ 0, 0, 0, 0, 0, 0, 0},
		{ 2, 3, 4, 5, 4, 3, 2},
		{ 2, 2, 3, 4, 3, 2, 2},
		{ 1, 1, 1, 1, 1, 1, 1}
	},
	1
};

const filter tenOclcok =
{
	{
		{ 0, 3, 2, 1, 1, 1, 1},
		{-3, 0, 4, 3, 2, 1, 1},
		{-2,-4, 0, 5, 4, 2, 1},
		{-1,-3,-5, 0, 5, 3, 1},
		{-1,-2,-4,-5, 0, 4, 2},
		{-1,-1,-2,-3,-4, 0, 3},
		{-1,-1,-1,-1,-2,-3, 0}
	},
	1
};

const filter twoOclcok =
{
	{
		{-1,-1,-1,-1,-2,-3, 0},
		{-1,-1,-2,-3,-4, 0, 3},
		{-1,-2,-4,-5, 0, 4, 2},
		{-1,-3,-5, 0, 5, 3, 1},
		{-2,-4, 0, 5, 4, 2, 1},
		{-3, 0, 4, 3, 2, 1, 1},
		{ 0, 3, 2, 1, 1, 1, 1}
	},
	1
};

int main()
{
	unsigned char** oriImg = alloc2DArr(sizeof(unsigned char), 256);
	unsigned char** procImg;
	unsigned char** gtImg = alloc2DArr(sizeof(unsigned char), 512);
	double** filterComputed[25] = { 0, };
	// !!!!!!! change below to load other images and make sure lr and gt imanges have to be a set
	getImage("dataset\\lr\\Barbara_256x256_yuv400_8bit.raw", 256, oriImg);
	getImage("dataset\\gt\\Barbara_512x512_yuv400_8bit.raw", 512, gtImg);
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
			printf("%2x ", oriImg[i][j]);
		printf("\n");
	}

	unsigned char** activity;
	unsigned char** direction;
	qdata activityRange;
	int groupCnt[25] = { 0, };
	int regroupInfo[25] = { -1, -1, -1,-1,-1,-1, -1, -1,-1,-1, -1, -1, -1,-1,-1, -1, -1, -1,-1,-1, -1, -1, -1,-1,-1 };
	coordinate** groupIdx;
	//classify the pixels
	{
		int** verField = (int**)alloc2DArr(sizeof(int), 256);
		int** horField = (int**)alloc2DArr(sizeof(int), 256);
		int** twoField = (int**)alloc2DArr(sizeof(int), 256);
		int** tenField = (int**)alloc2DArr(sizeof(int), 256);



		// get convolutional data
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				verField[i][j] = convolution(oriImg, 256, i, j, &Vertical);
				horField[i][j] = convolution(oriImg, 256, i, j, &Horizontal);
				twoField[i][j] = convolution(oriImg, 256, i, j, &twoOclcok);
				tenField[i][j] = convolution(oriImg, 256, i, j, &tenOclcok);
				//printf("ver = %d, hor = %d, two = %d, ten = %d\n", verField[i][j], horField[i][j], twoField[i][j], tenField[i][j]);
			}


		// convolutional data processing
		unsigned long long** activityAUX = alloc2DArr(sizeof(long long), 256);
		activity = (char**)alloc2DArr(sizeof(char), 256);
		direction = (char**)alloc2DArr(sizeof(char), 256);
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				// find maximum type of the Field
				long long dirVal = verField[i][j];

				direction[i][j] = 1;
				if (dirVal < twoField[i][j])
				{
					dirVal = twoField[i][j];
					direction[i][j] = 2;
				}
				if (dirVal < horField[i][j])
				{
					dirVal = horField[i][j];
					direction[i][j] = 3;
				}
				if (dirVal < tenField[i][j])
				{
					direction[i][j] = 4;
				}

				// get activity
				activityAUX[i][j] = verField[i][j] * verField[i][j] + horField[i][j] * horField[i][j];	//square of euclidian distance
			}

		activityRange = Qunatize(&(activityAUX[0][0]), 256 * 256, 5, 100);
		printf("res of qunatize = %lld~%lld~%lld~%lld~%lld~%lld\nthe representative values are %lld, %lld, %lld, %lld, %lld\n", activityRange.range[0], activityRange.range[1], activityRange.range[2], activityRange.range[3], activityRange.range[4], activityRange.range[5], activityRange.rep[0], activityRange.rep[1], activityRange.rep[2], activityRange.rep[3], activityRange.rep[4]);
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
#ifdef CLASSIFY_BY_STATICS
				long long ver = verField[i][j] < 0 ? -verField[i][j] : verField[i][j];
				long long hor = horField[i][j] < 0 ? -horField[i][j] : horField[i][j];
				long long ten = tenField[i][j] < 0 ? -tenField[i][j] : tenField[i][j];
				long long two = twoField[i][j] < 0 ? -twoField[i][j] : twoField[i][j];
				long long mean = (ver + hor + two + ten + 1) >> 2;	//+1 is bias for rounding
				long long stddev = (long long)(sqrt((ver - mean) * (ver - mean)
					+ (hor - mean) * (hor - mean)
					+ (two - mean) * (two - mean)
					+ (ten - mean) * (ten - mean)) + 0.5);	//+0.5 is bias for rounding
				long long target = direction[i][j] == 1 ? verField[i][j] :
					direction[i][j] == 2 ? twoField[i][j] :
					direction[i][j] == 3 ? horField[i][j] : tenField[i][j];
				if (target < 0) target = -target;
				//printf("[i][j]:mean %lld, stddev %lld, target %lld\n", mean, stddev, target);
				if (target < mean + (stddev >> 1))
					direction[i][j] = 0;
#endif
#ifdef CLASSIFY_BY_VECTOR		
				long long target = direction[i][j] == 1 ? verField[i][j] :
					direction[i][j] == 2 ? twoField[i][j] :
					direction[i][j] == 3 ? horField[i][j] : tenField[i][j];
				target *= target*2;	// modify coefficent
				if (target <= activityAUX[i][j])
					direction[i][j] = 0;
#endif
				for (int k = 0; k < 5; k++)
				{
					if (activityAUX[i][j] <= activityRange.range[k + 1])
					{
						activity[i][j] = k;
						break;
					}
				}

			}
		// set group
		int reGroup[5] = { 0, };
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				//printf("[%d][%d]:%d %d %d\n",i,j, activity[i][j], direction[i][j], activity[i][j] * 5 + direction[i][j]);
				groupCnt[activity[i][j] * 5 + direction[i][j]]++;
			}
		printf("group statics:\n");
		for (int i = 0; i < 25; i++)
		{
			printf("%d group is %d\n", i, groupCnt[i]);
			if (groupCnt[i] < FILTER_SZ * FILTER_SZ)
			{
				reGroup[i % 5]++;
				printf("!! %d group has to be regroupped!\n", i);
			}
		}
		// regroup
		for (int dir = 0; dir < 5; dir++)
		{
			if (!reGroup[dir]) continue;
			//act 4 check first
			if (groupCnt[20 + dir] < FILTER_SZ * FILTER_SZ)
			{
				for (int i = 0; i < 256; i++)
					for (int j = 0; j < 256; j++)
						if (activity[i][j] == 4 && direction[i][j] == dir)
							activity[i][j] = 3;
				groupCnt[15 + dir] += groupCnt[20 + dir];
				groupCnt[20 + dir] = 0;
				regroupInfo[20 + dir] = 15 + dir;
			}
			//act 0 check second
			if (groupCnt[dir] < FILTER_SZ * FILTER_SZ)
			{
				for (int i = 0; i < 256; i++)
					for (int j = 0; j < 256; j++)
						if (activity[i][j] == 0 && direction[i][j] == dir)
							activity[i][j] = 1;
				groupCnt[5 + dir] += groupCnt[0 + dir];
				groupCnt[0 + dir] = 0;
				regroupInfo[0 + dir] = 5 + dir;
			}
			//act 3 check third
			if (groupCnt[15 + dir] < FILTER_SZ * FILTER_SZ)
			{
				for (int i = 0; i < 256; i++)
					for (int j = 0; j < 256; j++)
						if (activity[i][j] == 3 && direction[i][j] == dir)
							activity[i][j] = 2;
				groupCnt[10 + dir] += groupCnt[15 + dir];
				groupCnt[15 + dir] = 0;
				regroupInfo[15 + dir] = 10 + dir;
			}
			//act 1 check fourth
			if (groupCnt[5 + dir] < FILTER_SZ * FILTER_SZ)
			{
				for (int i = 0; i < 256; i++)
					for (int j = 0; j < 256; j++)
						if (activity[i][j] == 1 && direction[i][j] == dir)
							activity[i][j] = 2;
				groupCnt[10 + dir] += groupCnt[5 + dir];
				groupCnt[5 + dir] = 0;
				regroupInfo[5 + dir] = 10 + dir;
			}
			//act 2 check last
			if (groupCnt[10 + dir] < FILTER_SZ* FILTER_SZ)
			{
				int tarAct = groupCnt[5 + dir] > groupCnt[15 + dir] ? 1 : 3;
				for (int i = 0; i < 256; i++)
					for (int j = 0; j < 256; j++)
						if (activity[i][j] == 2 && direction[i][j] == dir)
							activity[i][j] = tarAct;
				groupCnt[tarAct * 5 + dir] += groupCnt[10 + dir];
				groupCnt[10 + dir] = 0;
				regroupInfo[10 + dir] = tarAct * 5 + dir;
			}
		}
		printf("group statics:\n");
		for (int i = 0,cnt = 0; i < 25; i++)
		{
			printf("%d group is %d, %d\n", i, groupCnt[i],cnt);
			cnt += groupCnt[i];
		}
		/*
		// now direction means group index
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
				direction[i][j] += activity[i][j] * 5;
				*/
		groupIdx = (coordinate**) malloc(sizeof(coordinate*)*25);
		groupIdx[0] = (coordinate*) malloc(sizeof(coordinate) * 256 * 256);
		for (int i = 1; i < 25; i++)
			groupIdx[i] = &groupIdx[i - 1][groupCnt[i-1]];
		for (int i = 0, gIdx[25] = { 0, }; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				int gNum = activity[i][j] * 5 + direction[i][j];
				groupIdx[gNum][gIdx[gNum]++] = (coordinate){ i , j };
			}

		free2DArr(activity);
		free2DArr(direction);
		free2DArr(verField);
		free2DArr(horField);
		free2DArr(twoField);
		free2DArr(tenField);
		free2DArr(activityAUX);
		free_qdata(&activityRange);
	}
	// get filters "F = (X^T X)^-1 X^TY => (X^T X)F = X^TY"
	// Y : ground truth image
	// X : nearby piexels matrix
	for (int group = 0; group < 25; group++)
	{
		if (groupCnt[group] == 0) continue;
		const coordinate gtBias[3] = { {0,1},{1,0},{1,1} };
		//Mat_t** X;
		//Mat_t** Y;
		//Mat_t** XT;
		Mat_t** XTX = alloc2DArr(sizeof(Mat_t),FILTER_SZ * FILTER_SZ);
		Mat_t** XTY = alloc2DArrRec(sizeof(Mat_t), FILTER_SZ*FILTER_SZ, 3);
		double** doubleF;
		
		//get XTX and XTY
		// X^T X[i][j] = sigma (X[k][i] * X[k][j]) where k from 0 to (number of pixel data) 
		// X^T Y[i][j] = sigma (X[k][i] * Y[k][j]) where k from 0 to (number of pixel data)
		for (int i = 0; i < FILTER_SZ * FILTER_SZ; i++)
		{
			for (int j = 0; j < FILTER_SZ * FILTER_SZ; j++)
			{
				XTX[i][j] = 0;
				if(j < 3) XTY[i][j] = 0;
				for (int k = 0; k < groupCnt[group]; k++)
				{
					coordinate tgt = groupIdx[group][k];
					coordinate tgtI = (coordinate){ symCilp(tgt.row + (i / FILTER_SZ - (FILTER_SZ >> 1)),0,255), symCilp(tgt.col + (i % FILTER_SZ - (FILTER_SZ >> 1)),0,255) };
					coordinate tgtJ = (coordinate){ symCilp(tgt.row + (j / FILTER_SZ - (FILTER_SZ >> 1)),0,255), symCilp(tgt.col + (j % FILTER_SZ - (FILTER_SZ >> 1)),0,255) };
					XTX[i][j] += oriImg[tgtI.row][tgtI.col] * oriImg[tgtJ.row][tgtJ.col];
					if (j < 3) XTY[i][j] += oriImg[tgtI.row][tgtI.col] * gtImg[(tgt.row << 1) + gtBias[j].row][(tgt.col << 1) + gtBias[j].col];
				}
			}
		}
		
		printf("\n");
		//free2DArr(X);
		//free2DArr(Y);
		doubleF = Gauss_Jordan_Method_double(XTX, FILTER_SZ * FILTER_SZ, XTY, 3);
		
		//free2DArr(XT);
		free2DArr(XTX);
		free2DArr(XTY); 
		
		// save the filter
		double** tmp = alloc2DArrRec(sizeof(double), 3, FILTER_SZ * FILTER_SZ);
		for (int r = 0; r < FILTER_SZ * FILTER_SZ; r++)
			for (int c = 0; c < 3; c++)
				tmp[c][r] = doubleF[r][c];
		filterComputed[group] = tmp;
		free2DArr(doubleF);
	}

	//filter file print routine
	for (int g = 0; g < 25; g++)
	{
		if (!groupCnt[g]) continue;
		if (filterComputed[g] == NULL) continue;
		char dirname[100];
		sprintf(dirname, "filter\\filter%d%c_49x49_yuv400_8bit.raw", g, 'H');
		unsigned char** tmp = alloc2DArr(sizeof(unsigned char), 49);
		for (int i = 0; i < 49; i += 7)
			for (int j = 0; j < 49; j += 7)
				for (int k = 0; k < 7; k++)
					for (int l = 0; l < 7; l++)
						tmp[i + k][j + l] = 127 + (filterComputed[g][0][i * 7 + j] * 127);
		saveImage(dirname, 49, tmp);
		sprintf(dirname, "filter\\filter%d%c_49x49_yuv400_8bit.raw", g, 'V');
		for (int i = 0; i < 49; i += 7)
			for (int j = 0; j < 49; j += 7)
				for (int k = 0; k < 7; k++)
					for (int l = 0; l < 7; l++)
						tmp[i + k][j + l] = 127 + (filterComputed[g][1][i * 7 + j] * 127);
		saveImage(dirname, 49, tmp);
		sprintf(dirname, "filter\\filter%d%c_49x49_yuv400_8bit.raw", g, 'D');
		for (int i = 0; i < 49; i += 7)
			for (int j = 0; j < 49; j += 7)
				for (int k = 0; k < 7; k++)
					for (int l = 0; l < 7; l++)
						tmp[i + k][j + l] = 127 + (filterComputed[g][2][i * 7 + j] * 127);
		saveImage(dirname, 49, tmp);
		free2DArr(tmp);
	}
	free2DArr(groupIdx);
	memset(groupCnt, 0, sizeof(int) * 25);

	// duplicate the filters that is merged group
	for (int i = 0; i < 25; i++)
	{
		if (regroupInfo[i] != -1) filterComputed[i] = filterComputed[regroupInfo[i]];
		if (regroupInfo[i] != -1) printf("filter[%d] is duplicated to filter[%d]\n", regroupInfo[i], i);
	}


	// if you want to change processing image to use other filter set, use getImage below:
	//getImage("dataset\\lr\\Lena_256x256_yuv400_8bit.raw", 256, oriImg);

	//classify the original image
		//classify the pixels
	{
		int** verField = (int**)alloc2DArr(sizeof(int), 256);
		int** horField = (int**)alloc2DArr(sizeof(int), 256);
		int** twoField = (int**)alloc2DArr(sizeof(int), 256);
		int** tenField = (int**)alloc2DArr(sizeof(int), 256);



		// get convolutional data
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				verField[i][j] = convolution(oriImg, 256, i, j, &Vertical);
				horField[i][j] = convolution(oriImg, 256, i, j, &Horizontal);
				twoField[i][j] = convolution(oriImg, 256, i, j, &twoOclcok);
				tenField[i][j] = convolution(oriImg, 256, i, j, &tenOclcok);
				//printf("ver = %d, hor = %d, two = %d, ten = %d\n", verField[i][j], horField[i][j], twoField[i][j], tenField[i][j]);
			}


		// convolutional data processing
		unsigned long long** activityAUX = alloc2DArr(sizeof(long long), 256);
		activity = (char**)alloc2DArr(sizeof(char), 256);
		direction = (char**)alloc2DArr(sizeof(char), 256);
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				// find maximum type of the Field
				long long dirVal = verField[i][j];

				direction[i][j] = 1;
				if (dirVal < twoField[i][j])
				{
					dirVal = twoField[i][j];
					direction[i][j] = 2;
				}
				if (dirVal < horField[i][j])
				{
					dirVal = horField[i][j];
					direction[i][j] = 3;
				}
				if (dirVal < tenField[i][j])
				{
					direction[i][j] = 4;
				}

				// get activity
				activityAUX[i][j] = verField[i][j] * verField[i][j] + horField[i][j] * horField[i][j];	//square of euclidian distance
			}

		activityRange = Qunatize(&(activityAUX[0][0]), 256 * 256, 5, 100);
		printf("res of qunatize = %lld~%lld~%lld~%lld~%lld~%lld\nthe representative values are %lld, %lld, %lld, %lld, %lld\n", activityRange.range[0], activityRange.range[1], activityRange.range[2], activityRange.range[3], activityRange.range[4], activityRange.range[5], activityRange.rep[0], activityRange.rep[1], activityRange.rep[2], activityRange.rep[3], activityRange.rep[4]);
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
#ifdef CLASSIFY_BY_STATICS
				long long ver = verField[i][j] < 0 ? -verField[i][j] : verField[i][j];
				long long hor = horField[i][j] < 0 ? -horField[i][j] : horField[i][j];
				long long ten = tenField[i][j] < 0 ? -tenField[i][j] : tenField[i][j];
				long long two = twoField[i][j] < 0 ? -twoField[i][j] : twoField[i][j];
				long long mean = (ver + hor + two + ten + 1) >> 2;	//+1 is bias for rounding
				long long stddev = (long long)(sqrt((ver - mean) * (ver - mean)
					+ (hor - mean) * (hor - mean)
					+ (two - mean) * (two - mean)
					+ (ten - mean) * (ten - mean)) + 0.5);	//+0.5 is bias for rounding
				long long target = direction[i][j] == 1 ? verField[i][j] :
					direction[i][j] == 2 ? twoField[i][j] :
					direction[i][j] == 3 ? horField[i][j] : tenField[i][j];
				if (target < 0) target = -target;
				//printf("[i][j]:mean %lld, stddev %lld, target %lld\n", mean, stddev, target);
				if (target < mean + (stddev >> 1))
					direction[i][j] = 0;
#endif
#ifdef CLASSIFY_BY_VECTOR		
				long long target = direction[i][j] == 1 ? verField[i][j] :
					direction[i][j] == 2 ? twoField[i][j] :
					direction[i][j] == 3 ? horField[i][j] : tenField[i][j];
				target *= target * 2;	// modify coefficent
				if (target <= activityAUX[i][j])
					direction[i][j] = 0;
#endif
				for (int k = 0; k < 5; k++)
				{
					if (activityAUX[i][j] <= activityRange.range[k + 1])
					{
						activity[i][j] = k;
						break;
					}
				}

			}
		// set group
		int reGroup[5] = { 0, };
		for (int i = 0; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				//printf("[%d][%d]:%d %d %d\n",i,j, activity[i][j], direction[i][j], activity[i][j] * 5 + direction[i][j]);
				groupCnt[activity[i][j] * 5 + direction[i][j]]++;
			}
		printf("group statics:\n");
		for (int i = 0; i < 25; i++)
		{
			printf("%d group is %d\n", i, groupCnt[i]);
			if (groupCnt[i] < FILTER_SZ * FILTER_SZ)
			{
				reGroup[i % 5]++;
			}
		}

		groupIdx = malloc(sizeof(coordinate*) * 25);
		groupIdx[0] = malloc(sizeof(coordinate) * 256 * 256);
		for (int i = 1; i < 25; i++)
			groupIdx[i] = &groupIdx[i - 1][groupCnt[i - 1]];
		for (int i = 0, gIdx[25] = { 0, }; i < 256; i++)
			for (int j = 0; j < 256; j++)
			{
				int gNum = activity[i][j] * 5 + direction[i][j];
				groupIdx[gNum][gIdx[gNum]++] = (coordinate){ i , j };
			}

		free2DArr(activity);
		free2DArr(direction);
		free2DArr(verField);
		free2DArr(horField);
		free2DArr(twoField);
		free2DArr(tenField);
		free2DArr(activityAUX);
		free_qdata(&activityRange);
	}
	
	// set filters
	procImg = alloc2DArr(sizeof(unsigned char), 512);
	for (int group = 0; group < 25; group++)
	{
		for (int idx = 0; idx < groupCnt[group]; idx++)
		{
			procImg[(groupIdx[group][idx].row) << 1][(groupIdx[group][idx].col) << 1] = oriImg[groupIdx[group][idx].row][groupIdx[group][idx].col];
			procImg[(groupIdx[group][idx].row) << 1][((groupIdx[group][idx].col) << 1) + 1] = convolution_double(oriImg, 256, groupIdx[group][idx].row, groupIdx[group][idx].col, filterComputed[group][0]);
			procImg[((groupIdx[group][idx].row) << 1) + 1][(groupIdx[group][idx].col) << 1] = convolution_double(oriImg, 256, groupIdx[group][idx].row, groupIdx[group][idx].col, filterComputed[group][1]);
			procImg[((groupIdx[group][idx].row) << 1) + 1][((groupIdx[group][idx].col) << 1) + 1] = convolution_double(oriImg, 256, groupIdx[group][idx].row, groupIdx[group][idx].col, filterComputed[group][2]);
		}
	}
	
	saveImage("res.raw", 512, procImg);
	
	printf("the PSNR is %lf\n", evalInterpole(gtImg, procImg));
	free2DArr(groupIdx);
	free2DArr(oriImg);
	free2DArr(procImg);
	free2DArr(gtImg);
	return 0;
}
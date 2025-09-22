#include "LMQuant.h"

int compare(const void* a, const void* b)
{
	if (*(Qtype*)a > *(Qtype*)b)return 1;
	else if (*(Qtype*)a < *(Qtype*)b)return -1;
	else return 0;
}

void free_qdata(qdata* obj)
{
	free(obj->range);
	free(obj->rep);
}

qdata Qunatize(Qtype* data, int dataSz, int setCnt, int iterate)
{
	Qtype* sortData = (Qtype*)malloc(sizeof(Qtype) * dataSz);
	int* rngIdx = (int*)malloc(sizeof(int) * (setCnt + 1));
	int* repIdx = (int*)malloc(sizeof(int) * setCnt);
	qdata res;
	res.size = setCnt;

	//sort the data
	memcpy(sortData, data, sizeof(Qtype) * dataSz);
	qsort(sortData, dataSz, sizeof(Qtype), compare);

	//initialize qdata struct
	res.size = setCnt;
	res.range = (Qtype*)malloc(sizeof(Qtype) * (setCnt + 1));
	res.rep = (Qtype*)malloc(sizeof(Qtype) * setCnt);
	rngIdx[0] = 0;
	rngIdx[setCnt] = dataSz - 1;
	for (int i = 1; i < setCnt; i++)
		rngIdx[i] = rngIdx[i - 1] + dataSz / setCnt;

	//start Lloyd max quantize
	for (int i = 0; i < iterate; i++)
	{
		if (i)
		{	//set new range
			for (int j = 1; j < setCnt; j++)
			{
				rngIdx[j] = (repIdx[j - 1] + repIdx[j]) >> 1;
			}
		}
		for (int setNo = 0, itemNo = 0; setNo < setCnt; setNo++)
		{	//find new represent number
			long long localSum = 0, localWeightedSum = 0;
			for (; itemNo <= rngIdx[setNo + 1]; itemNo++)
			{
				//printf("itemNo : %d, sortData[%d] = %d\n", itemNo, itemNo, sortData[itemNo]);
				localSum += sortData[itemNo];
				localWeightedSum += sortData[itemNo] * itemNo;
			}
			repIdx[setNo] = localWeightedSum / localSum;
		}
	}
	for (int i = 0; i < setCnt; i++)
		res.rep[i] = sortData[repIdx[i]];
	for (int i = 0; i <= setCnt; i++)
		res.range[i] = sortData[rngIdx[i]];
	free(repIdx);
	free(rngIdx);
	free(sortData);
	return res;
}
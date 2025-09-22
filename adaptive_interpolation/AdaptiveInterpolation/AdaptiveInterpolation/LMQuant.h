#ifndef __LM_QUNAT__
#define __LM_QUANT__

#include<stdlib.h>
#include<memory.h>

typedef long long Qtype;

typedef struct _QUANT_DATA
{
	int size;
	Qtype* range;
	Qtype* rep;
}qdata;

void free_qdata(qdata* obj);

qdata Qunatize(Qtype* data, int dataSz, int setCnt, int iterate);

#endif
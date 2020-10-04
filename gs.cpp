#include "gs.h"
#include "utils.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <climits>

GenSketch::GenSketch(int size, int num_hash) :
size(size), num_hash(num_hash)
{
	sprintf(name, "GenSketch");

	if (size <= 0 || num_hash <= 0)
	{
		panic("SIZE & NUM_HASH must be POSITIVE integers.");
	}
	cnt = new int[size];
	hash = new BOBHash32[num_hash];
	
	row_size = size / num_hash;
}

GenSketch::~GenSketch()
{
	if (cnt)
		delete [] cnt;
	if (hash)
		delete [] hash;
}

void
GenSketch::status()
{
	printf("bucket: %d   hash: %d\n", size, num_hash);
}

int
GenSketch::query_freq(int v)
{
	int ans = INT_MAX;
	int i = 0, base = 0;

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		ans = min(ans, cnt[pos]);
	}

	return ans;
}

#ifndef GENS_HEADER
#define GENS_HEADER

#include "sketch.h"
#include "BOBHash32.h"

class GenSketch : public Sketch
{
public:
	int size, num_hash, row_size;

	int *cnt;
	BOBHash32 *hash;

	GenSketch(int size, int num_hash);
	~GenSketch();
	int query_freq(int v);
	void status();
};

#endif
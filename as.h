#ifndef AS_HEADER
#define AS_HEADER

#include "sketch.h"
#include "BOBHash32.h"
#include <set>
using std::set;

class ASketch : public Sketch
{
private:
	int size, num_hash, num_filter, row_size;

	// filter
	int *new_cnt, *old_cnt, *fp;
	
	// cm sketch
	int *cm_cnt;
	BOBHash32 *hash;

public:
	ASketch(int size, int num_hash, int num_filter);
	~ASketch();
	void init();
	void insert(int v);
	void insert(int v, int x);
	int query_freq(int v);
	vector<PII> query_heavyhitter(int threshold);
	vector<PII> query_topk(int k);
	void status();	
};

#endif
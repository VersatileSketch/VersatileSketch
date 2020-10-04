#ifndef SBF_HEADER
#define SBF_HEADER

#include "sketch.h"
#include "gs.h"
#include "minheap.h"
#include "BOBHash32.h"
#include <set>
using std::set;

class VSketch : public Sketch
{
public:
	int size, num_hash, row_size;

	int *hi_cnt, *lo_cnt;
	int *fp;
	bool cu_ver;
	bool low_ver; // under-estimation
	BOBHash32 *hash;

	MinHeap hp;

	VSketch(int size, int num_hash, bool cu = false);
	~VSketch();
	void init();
	void insert(int v);
	bool query_exist(int v);
	int query_freq(int v);
	int query_freq_low(int v);
	vector<PII> query_topk(int k);
	vector<PII> query_heavyhitter(int threshold);
	void query_distribution(vector<double>& ans);
	void status();
	void set_lowver(bool flag) { low_ver = flag; }

	VSketch* compress();
	VSketch* merge(VSketch *q);
	VSketch* expand();

	// Generated Sketch
	GenSketch* generate_elephant();
	GenSketch* generate_mouse();	
};

#endif
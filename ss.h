#ifndef SS_HEADER
#define SS_HEADER

#include "sketch.h"
#include "BOBHash32.h"
#include <unordered_map>
using std::unordered_map;

#define mp make_pair
#define ft first
#define sc second

class Bucket;
class Element;

class Bucket
{
public:
	Bucket *prev, *next;
	Element *son;
	int value;
};

class Element
{
public:
	Bucket *parent;
	Element *prev, *next;
	int fp;
};


class SpaceSaving : public Sketch
{
private:
	int size;

	Bucket *tail, *bkt, *free_head;
	Element *ele;

	unordered_map<int, Element*> ele_idx;

public:
	SpaceSaving(int size);
	~SpaceSaving();
	void init();
	void insert(int v);
	int query_freq(int v);
	vector<PII> query_heavyhitter(int threshold);
	vector<PII> query_topk(int k);
	void status();	
};

#endif
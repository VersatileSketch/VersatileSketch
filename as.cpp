#include "as.h"
#include "utils.h"
#include "EMSFD.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <climits>
#include <cstdint>
#include <map>
using std::map;

ASketch::ASketch(int size, int num_hash, int num_filter) :
size(size), num_hash(num_hash), num_filter(num_filter)
{
	sprintf(name, "ASketch");

	if (size <= 0 || num_hash <= 0)
	{
		panic("SIZE & NUM_HASH must be POSITIVE integers.");
	}

	cm_cnt = new int[size];

	new_cnt = new int[num_filter];
	old_cnt = new int[num_filter];
	fp = new int[num_filter];

	hash = new BOBHash32[num_hash];

	row_size = size / num_hash;
}

ASketch::~ASketch()
{
	if (cm_cnt)
		delete [] cm_cnt;
	if (new_cnt)
		delete [] new_cnt;
	if (old_cnt)
		delete [] old_cnt;
	if (fp)
		delete [] fp;
	if (hash)
		delete [] hash;
}

void
ASketch::init()
{
	memset(cm_cnt, 0, size * sizeof(int));
	memset(new_cnt, 0, num_filter * sizeof(int));
	memset(old_cnt, 0, num_filter * sizeof(int));
	memset(fp, 0, num_filter * sizeof(int));

	for (int i = 0; i < num_hash; ++i)
	{
		hash[i].initialize(i+1);//rand() % MAX_PRIME32);
	}
}

void
ASketch::insert(int v)
{
	insert(v, 1);
}

void
ASketch::insert(int v, int x)
{
	bool flag = false;
	int i = 0, base = 0;

	// query in filter
	for (i = 0; i < num_filter; ++i)
		if (fp[i] == v)
		{
			new_cnt[i] += x;
			return;
		}

	int min_flt = INT_MAX, min_pos = -1;
	for (i = 0; i < num_filter; ++i)
	{
		if (fp[i] == 0)
		{
			new_cnt[i] = x;
			old_cnt[i] = 0;
			fp[i] = v;
			return;
		}
		else if (min_flt > new_cnt[i])
		{
			min_flt = new_cnt[i];
			min_pos = i;
		}
	}

	int est = INT_MAX;
	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		cm_cnt[pos] += x;
		est = min(est, cm_cnt[pos]);
	}

	if (est > min_flt)
	{
		int delta = new_cnt[min_pos] - old_cnt[min_pos];
		if (delta > 0)
		{	
			for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
			{
				int pos = hash[i].run((char*)&fp[min_pos], sizeof(int)) % row_size + base;
				cm_cnt[pos] += delta;
			}
		}

		fp[min_pos] = v;
		new_cnt[min_pos] = est;
		old_cnt[min_pos] = est;
	}
}

int
ASketch::query_freq(int v)
{
	int i = 0, base = 0;

	for (i = 0; i < num_filter; ++i)
		if (fp[i] == v)
			return new_cnt[i];

	int ans = INT_MAX;
	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		ans = min(ans, cm_cnt[pos]);
	}

	return ans;
}

vector<PII>
ASketch::query_heavyhitter(int threshold)
{
	vector<PII> tmp, ans;
	
	for (int i = 0; i < num_filter; ++i)
		if (new_cnt[i] > 0)
		{
			tmp.push_back(mp(new_cnt[i], fp[i]));
		}
	sort(tmp.begin(), tmp.end(), greater<PII>());

	int sz = tmp.size();
	for (int i = 0; i < sz; ++i)
	{
		if (tmp[i].first < threshold) break;
		ans.push_back(tmp[i]);
	}

	return ans;
}

vector<PII>
ASketch::query_topk(int k)
{
	vector<PII> tmp, ans;
	
	for (int i = 0; i < num_filter; ++i)
		if (new_cnt[i] > 0)
		{
			tmp.push_back(mp(new_cnt[i], fp[i]));
		}
	sort(tmp.begin(), tmp.end(), greater<PII>());

	if (tmp.size() < k)
		k = tmp.size();

	for (int i = 0; i < k; ++i)
		ans.push_back(tmp[i]);

	return ans;
}


void
ASketch::status()
{
	printf("bucket: %d   hash: %d   filter: %d\n", size, num_hash, num_filter);
}


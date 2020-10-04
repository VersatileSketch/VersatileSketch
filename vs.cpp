#include "vs.h"
#include "utils.h"
#include "EMSFD.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <climits>
#include <cstdint>
#include <map>
using std::map;

VSketch::VSketch(int size, int num_hash, bool cu) :
size(size), num_hash(num_hash), cu_ver(cu)
{
	sprintf(name, "VSketch");

	if (size <= 0 || num_hash <= 0)
	{
		panic("SIZE & NUM_HASH must be POSITIVE integers.");
	}
	hi_cnt = new int[size];
	lo_cnt = new int[size];
	fp = new int[size];
	hash = new BOBHash32[num_hash];

	row_size = size / num_hash;
	low_ver = false;
}

VSketch::~VSketch()
{
	if (hi_cnt)
		delete [] hi_cnt;
	if (lo_cnt)
		delete [] lo_cnt;
	if (fp)
		delete [] fp;
	if (hash)
		delete [] hash;
}

void
VSketch::init()
{
	memset(hi_cnt, 0, size * sizeof(int));
	memset(lo_cnt, 0, size * sizeof(int));
	memset(fp, 0, size * sizeof(int));

	for (int i = 0; i < num_hash; ++i)
	{
		hash[i].initialize(i + 1); //rand() % MAX_PRIME32);
	}

	hp.init();
}

void
VSketch::insert(int v)
{
	// bool flag = false;
	// int max_lo = INT_MIN;
	int min_est = INT_MAX;
	int i = 0, base = 0;
	static int sav_pos[10];

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		int est = (fp[pos] == v? lo_cnt[pos] : -lo_cnt[pos]);
		est = (est + hi_cnt[pos]) >> 1;
		// acc_est[i] = est;
		sav_pos[i] = pos;
		min_est = min_est > est? min_est : est;
	}

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = sav_pos[i];
		//int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;

		int est = (fp[pos] == v? lo_cnt[pos] : -lo_cnt[pos]);
		est = (est + hi_cnt[pos]) >> 1;
		
		// cu
		if (est > min_est && cu_ver)
			continue;

		// high part - cm sketch
		hi_cnt[pos]++;

		// low part
		if (fp[pos] == v)
		{
			lo_cnt[pos]++;
			// if (lo_cnt[pos] > 100)
			// 	flag = true;
		}
		else if (lo_cnt[pos] == 0)
		{
			fp[pos] = v;
			lo_cnt[pos]++;
		}
		else
		{
			lo_cnt[pos]--;
		}

		// if (fp[pos] == v)
		// 	max_lo = max(max_lo, lo_cnt[pos]);

		// if (v == -1327507284)
		// {
		// 	printf("%d %d %d\n", fp[pos]==v, lo_cnt[pos], hi_cnt[pos]);
		// }
	}

	// int est = query_freq(v);
	// if (flag)
	// {
	// 	// hp.insert(v, max_lo);
	// 	hp.insert(v, est);
	// }
}

bool
VSketch::query_exist(int v)
{
	bool ans = false;
	int i = 0, base = 0;

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		ans |= (fp[pos] == v);
	}

	return ans;
}

int
VSketch::query_freq(int v)
{
	if (low_ver)
		return query_freq_low(v);

	int ans = INT_MAX;
	int i = 0, base = 0;

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		int est = (fp[pos] == v? lo_cnt[pos] : -lo_cnt[pos]);
		est = (est + hi_cnt[pos]) >> 1;
		ans = min(ans, est);
	}

	return ans;
}

int
VSketch::query_freq_low(int v)
{
	int ans = INT_MIN;
	int i = 0, base = 0;

	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		int pos = hash[i].run((char*)&v, sizeof(int)) % row_size + base;
		int est = (fp[pos] == v? (hi_cnt[pos] + lo_cnt[pos])/2 : 0);
		ans = max(ans, est);
	}

	return ans;
}

vector<PII>
VSketch::query_topk(int k)
{
	vector<PII> tmp, ans;
	map<int, int> vis;
	for (int i = 0; i < size; ++i)
		vis[fp[i]] = false;

	for (int i = 0; i < size; ++i)
		if (hi_cnt[i] > 0)
		{
			int est = (hi_cnt[i] + lo_cnt[i]) >> 1;
			if (!vis[fp[i]])
			{
				vis[fp[i]] = true;
				tmp.push_back(mp(query_freq(fp[i]), fp[i]));
			}
		}

    sort(tmp.begin(), tmp.end(), greater<PII>());
    for (int i = 0; i < k && i < tmp.size(); ++i)
    	ans.push_back(tmp[i]);
	return ans;

	// return hp.topk(k);
}

void
VSketch::status()
{
	printf("bucket: %d   hash: %d    %s\n", size, num_hash, cu_ver?"[CU]":"");
}

vector<PII>
VSketch::query_heavyhitter(int threshold)
{
	vector<PII> ans;
	map<int, int> vis;
	for (int i = 0; i < size; ++i)
		vis[fp[i]] = false;

	for (int i = 0; i < size; ++i)
		if (hi_cnt[i] > 0)
		{
			int est = (hi_cnt[i] + lo_cnt[i]) >> 1;
			if (est >= threshold && !vis[fp[i]])
			{
				vis[fp[i]] = true;
				ans.push_back(mp(query_freq(fp[i]), fp[i]));
			}
		}

    sort(ans.begin(), ans.end(), greater<PII>());
	return ans;
}

void
VSketch::query_distribution(vector<double>& ans)
{
	if (num_hash != 1)
	{
		printf("The answer may be wrong because NUM_HASH is not equal to 1.\n");
	}

	// low part
	uint32_t *tmp_cnt;
	tmp_cnt = new uint32_t[size];

	for (int i = 0; i < size; ++i)
		tmp_cnt[i] = (hi_cnt[i] - lo_cnt[i]) >> 1;

	EMFSD *em_fsd_algo;
	em_fsd_algo = new EMFSD();
    em_fsd_algo->set_counters(size, tmp_cnt);

    for (int i = 0; i < 10; ++i)
    {
    	printf("epoch %d\n", i);
    	em_fsd_algo->next_epoch();
    }

    delete [] tmp_cnt;

    ans = em_fsd_algo->ns;

	map<int, int> vis;
	for (int i = 0; i < size; ++i)
		vis[fp[i]] = false;

	for (int i = 0; i < size; ++i)
		if (hi_cnt[i] > 0 && !vis[fp[i]])
		{	
			int est = query_freq(fp[i]);
			vis[fp[i]] = true;

			if (est + 1 > ans.size())
				ans.resize(est + 1);
			ans[est] += 1.;
		}
}

GenSketch*
VSketch::generate_elephant()
{
	GenSketch *gs = new GenSketch(size, num_hash);

	for (int i = 0; i < num_hash; ++i)
		gs->hash[i] = hash[i];
	for (int i = 0; i < size; ++i)
		gs->cnt[i] = (hi_cnt[i] + lo_cnt[i]) >> 1;

	return gs;
}

GenSketch*
VSketch::generate_mouse()
{
	GenSketch *gs = new GenSketch(size, num_hash);

	for (int i = 0; i < num_hash; ++i)
		gs->hash[i] = hash[i];
	for (int i = 0; i < size; ++i)
		gs->cnt[i] = (hi_cnt[i] - lo_cnt[i]) >> 1;

	return gs;
}

VSketch*
VSketch::compress()
{
	if (row_size % 2 != 0)
	{
		printf("error! row_size must be even.\n");
		return NULL;
	}

	VSketch * cpr = new VSketch(size/2, num_hash, cu_ver);
	cpr->init();

	int hlf = row_size / 2;
	int base, i, old_base;
	for (i = 0, base = 0, old_base = 0; i < num_hash; ++i, base += hlf, old_base += row_size)
	{
		for (int j = 0; j < hlf; ++j)
		{
			int old_pos = old_base + j;
			int pos = base + j;
			cpr->hi_cnt[pos] = max(hi_cnt[old_pos], hi_cnt[old_pos + hlf]);
			if (lo_cnt[old_pos] > lo_cnt[old_pos + hlf])
			{
				cpr->lo_cnt[pos] = lo_cnt[old_pos];
				cpr->fp[pos] = fp[old_pos];
			}
			else
			{
				cpr->lo_cnt[pos] = lo_cnt[old_pos + hlf];
				cpr->fp[pos] = fp[old_pos + hlf];
			}
		}
	}

	if (low_ver)
		cpr->low_ver = true;

	return cpr;
}

VSketch*
VSketch::merge(VSketch* q)
{
	if (size != q->size || num_hash != q->num_hash)
	{
		printf("error! size must be equal.\n");
		return NULL;
	}

	VSketch * mrg = new VSketch(size, num_hash, cu_ver);
	mrg->init();

	int base, i;
	for (i = 0, base = 0; i < num_hash; ++i, base += row_size)
	{
		for (int j = 0; j < row_size; ++j)
		{
			int pos = base + j;
			mrg->hi_cnt[pos] = hi_cnt[pos] + q->hi_cnt[pos];

			if (fp[pos] == q->fp[pos])
			{
				mrg->fp[pos] = fp[pos];
				mrg->lo_cnt[pos] = lo_cnt[pos] + q->lo_cnt[pos];
			}
			else if (lo_cnt[pos] > q->lo_cnt[pos])
			{
				mrg->lo_cnt[pos] = lo_cnt[pos];
				mrg->fp[pos] = fp[pos];
			}
			else
			{
				mrg->lo_cnt[pos] = q->lo_cnt[pos];
				mrg->fp[pos] = q->fp[pos];
			}
		}
	}

	if (low_ver)
		mrg->low_ver = true;

	return mrg;
}

VSketch*
VSketch::expand()
{
	VSketch * epd = new VSketch(size*2, num_hash, cu_ver);
	epd->init();

	int base, i, old_base;
	for (i = 0, base = 0, old_base = 0; i < num_hash; ++i, base += 2*row_size, old_base += row_size)
	{
		for (int j = 0; j < row_size; ++j)
		{
			int old_pos = old_base + j;
			int pos = base + j;

			epd->hi_cnt[pos] = epd->hi_cnt[pos+row_size] = hi_cnt[old_pos];
			epd->lo_cnt[pos] = epd->lo_cnt[pos+row_size] = lo_cnt[old_pos];
			epd->fp[pos] = epd->fp[pos+row_size] = fp[old_pos];
		}
	}

	for (i = 0, base = 0; i < num_hash; ++i, base += 2*row_size)
	{
		for (int j = 0; j < 2*row_size; ++j)
		{
			int pos = base + j;
			if (epd->fp[pos] == 0) continue;

			int real_pos = hash[i].run((char*)&epd->fp[pos], sizeof(int)) % (2*row_size) + base;
			
			if (real_pos != pos)
			{
				epd->fp[pos] = 0;
				epd->hi_cnt[pos] -= epd->lo_cnt[pos];
				epd->lo_cnt[pos] = 0;
			}
		}
	}

	if (low_ver)
		epd->low_ver = true;

	return epd;
}

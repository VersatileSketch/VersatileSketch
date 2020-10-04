class CU_sketch
{
private:
	int total_bucket_num,bucket_num;
	int hash_num;
	int max_count;
	int distinct_num;
	counter_t** bucket;
	counter_t* dist;
	BOBHash32** hash_func;
public:
	CU_sketch(int _total_bucket_num,int _hash_num,int _max_count)
	{
		total_bucket_num=_total_bucket_num;
		hash_num=_hash_num;
		max_count=_max_count;
		bucket_num=total_bucket_num/hash_num;
		bucket=new counter_t*[hash_num];
		distinct_num = 0;
		for (int i=0;i<hash_num;i++)
		{
			bucket[i]=new counter_t[bucket_num];
			memset(bucket[i],0,sizeof(int)*bucket_num);
		}
		dist=new counter_t[max_count+1];
		memset(dist,0,sizeof(counter_t)*(max_count+1));
		hash_func=new BOBHash32*[hash_num];
		for (int i=0;i<hash_num;i++)
			hash_func[i]=new BOBHash32(i*12+221);
	}
	~CU_sketch()
	{
		for (int i=0;i<hash_num;i++)
		{
			delete hash_func[i];
			delete []bucket[i];
		}
		delete []hash_func;
		delete []bucket;
		delete []dist;
	}
	counter_t query(const char *key)
	{
		counter_t ret=CNT_MAX;
		for (int i=0;i<hash_num;i++)
		{
			int index=hash_func[i]->run(key,FIVE_TUPLE_LEN)%bucket_num;
			ret=min(ret,bucket[i][index]);
		}
		return ret;
	}
	counter_t min(counter_t x,counter_t y)
	{
		if (x<y) return x;
		else return y;
	}
	void insert(const char* key)
	{
		counter_t previous_count=min(query(key),max_count);
		bool flag = false;
		for (int i=previous_count;i==previous_count;i--)
		if (dist[i]>0)
		{
			flag = true;
			dist[i]--;
			break;
		}
		if (!flag)
		{
			distinct_num++;
		}
		dist[min(previous_count+1,max_count)]++;
		for (int i=0;i<hash_num;i++)
		{
			int index=hash_func[i]->run(key,FIVE_TUPLE_LEN)%bucket_num;
			if (bucket[i][index]==previous_count)
				bucket[i][index]++;
		}
	}
	double calc_entropy()
	{
		int tot=0;
		double entr=0.0;
		for (int i=1;i<max_count;i++)
		{
			tot+=dist[i]*i;
			entr+=(double)dist[i]*(double)i*log(i)/log(2);
		}
		return -entr/(double)tot+log(tot)/log(2);
	}
	void printestimation()
	{
		for (int i=1;i<=2000;i++) printf("%d %d\n",i,dist[i]);
	}
};

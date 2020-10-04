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
		for (int i=previous_count;i>=0;i--)
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
	double calc_ARE(const distribution_record &distribution)
	{
		double sum=0.0;
		int cnt=0;
		for (distribution_record::const_iterator it=distribution.begin();it!=distribution.end();it++)
		{
			counter_t freq1=dist[it->first];
			counter_t freq2=it->second;
			int AE=(int)freq1-(int)freq2;
			double RE=(double)AE/(double)freq2;
			sum+=fabs(RE);cnt++;
		}
		return sum/(double)cnt;
	}
	double calc_AAE(const distribution_record &distribution)
	{
		double sum=0.0;
		int cnt=0;
		for (distribution_record::const_iterator it=distribution.begin();it!=distribution.end();it++)
		{
			counter_t freq1=dist[it->first];
			counter_t freq2=it->second;
			int AE=(int)freq1-(int)freq2;
			sum+=fabs(AE);cnt++;
		}
		return sum/(double)cnt;
	}
	double calc_WMRE(distribution_record &distribution)
	{
		double div1=0.0,div2=0.0;
		for (int i = 1; i < max_count; ++i)
		{
			counter_t freq1=dist[i];
			counter_t freq2=distribution.count(i)? distribution[i] : 0;
			double AE=fabs((double)freq1-(double)freq2);
			div1+=AE;div2+=freq1+freq2;
		}
		// for (distribution_record::const_iterator it=distribution.begin();it!=distribution.end();it++)
		// {
		// 	counter_t freq1=dist[it->first];
		// 	counter_t freq2=it->second;
		// 	double AE=fabs((double)freq1-(double)freq2);
		// 	div1+=AE;div2+=freq1+freq2;
		// }
		return 2*div1/div2;
	}
	int calc_DI()
	{
		int sum = 0;
		for (int i = 1; i < max_count; i++)
			sum += dist[i];
		return sum;
		return distinct_num;
	}
	void printestimation()
	{
		for (int i=1;i<=2000;i++) printf("%d %d\n",i,dist[i]);
	}
};

class versatile_sketch
{
private:
	int total_bucket_num,bucket_num;
	int hash_num;
	int max_count;
	counter_t** lowbound;
	counter_t** upbound;
	counter_t** purebound;
	counter_t** hashvalue;
	counter_t* dist;
	BOBHash32** hash_func;
public:
	versatile_sketch(int _total_bucket_num,int _hash_num,int _max_count)
	{
		total_bucket_num=_total_bucket_num;
		hash_num=_hash_num;
		max_count=_max_count;
		bucket_num=total_bucket_num/hash_num;
		lowbound=new counter_t*[hash_num];
		upbound=new counter_t*[hash_num];
		purebound=new counter_t*[hash_num];
		hashvalue=new counter_t*[hash_num];
		for (int i=0;i<hash_num;i++)
		{
			lowbound[i]=new counter_t[bucket_num];
			memset(lowbound[i],0,sizeof(counter_t)*bucket_num);
			upbound[i]=new counter_t[bucket_num];
			memset(upbound[i],0,sizeof(counter_t)*bucket_num);
			purebound[i]=new counter_t[bucket_num];
			memset(purebound[i],0,sizeof(counter_t)*bucket_num);
			hashvalue[i]=new counter_t[bucket_num];
		}
		dist=new counter_t[max_count+1];
		memset(dist,0,sizeof(int)*(max_count+1));
		hash_func=new BOBHash32*[hash_num+1];
		for (int i=0;i<=hash_num;i++)
			hash_func[i]=new BOBHash32(i*12+221);
	}
	~versatile_sketch()
	{
		for (int i=0;i<=hash_num;i++)
			delete hash_func[i];
		for (int i=0;i<hash_num;i++)
		{
			delete []lowbound[i];
			delete []upbound[i];
			delete []purebound[i];
			delete []hashvalue[i];
		}
		delete []hash_func;
		delete []lowbound;
		delete []upbound;
		delete []purebound;
		delete []hashvalue;
		delete []dist;
	}
	counter_t query(const char *key)
	{
		counter_t footprint=hash_func[hash_num]->run(key,FIVE_TUPLE_LEN);
		counter_t ret=CNT_MAX;
		for (int i=0;i<hash_num;i++)
		{
			int index=hash_func[i]->run(key,FIVE_TUPLE_LEN)%bucket_num;
			counter_t freq;
			if (hashvalue[i][index]==footprint)
				freq=(lowbound[i][index]+upbound[i][index])/2;
			else
				freq=(upbound[i][index]-lowbound[i][index])/2;
			ret=min(ret,freq);
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
		/* 
		for (int i=previous_count;i>=0;i--)
		if (dist[i]>0)
		{
			dist[i]--;
			break;
		}
		*/
		if (dist[previous_count]>0) dist[previous_count]--;
		dist[min(previous_count+1,max_count)]++;
		counter_t footprint=hash_func[hash_num]->run(key,FIVE_TUPLE_LEN);
		for (int i=0;i<hash_num;i++)
		{
			int index=hash_func[i]->run(key,FIVE_TUPLE_LEN)%bucket_num;
			if (hashvalue[i][index]==footprint)
			{
				lowbound[i][index]++;
				upbound[i][index]++;
				purebound[i][index]++;
			}
			else
			{
				if (lowbound[i][index]!=0)
				{
					lowbound[i][index]--;
					upbound[i][index]++;
				}
				else
				{
					hashvalue[i][index]=footprint;
					purebound[i][index]=0;
					lowbound[i][index]++;
					upbound[i][index]++;
					purebound[i][index]++;
				}
			}
		}
	}
	/*
	entropy=-sum(p*log2p)
	=-sum(f/n*log2f/n)
	=-1/nsum(f(log2f-log2n))
	=-1/n(sum(flog2f)-sum(flog2n))
	=-1/nsum(flog2f)+log2n
	*/
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
};

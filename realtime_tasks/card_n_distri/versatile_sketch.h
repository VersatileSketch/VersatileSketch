class versatile_sketch
{
private:
	int total_bucket_num,bucket_num;
	int hash_num;
	int max_count;
	int distinct_num;
	counter_t** lowbound;
	counter_t** upbound;
	counter_t** purebound;
	counter_t** hashvalue;
	counter_t* dist;
	BOBHash32** hash_func;
public:
	versatile_sketch(int _total_bucket_num,int _hash_num,int _max_count)
	{
		distinct_num = 0;
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
		bool flag = false;
		for (int i=previous_count;i>=0;i--)
		if (dist[i]>0)
		{
			dist[i]--;
			flag = true;
			break;
		}
		if (!flag)
		{
			distinct_num++;
		}

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
	double calc_ARE(distribution_record &distribution)
	{
		double sum=0.0;
		int cnt=0;
		for (int i = 1; i < max_count; ++i)
		{
			counter_t freq1=dist[i];
			counter_t freq2=distribution.count(i)? distribution[i] : 0;
			int AE=(int)freq1-(int)freq2;
			double RE=(double)AE/(double)freq2;
			sum+=fabs(RE);cnt++;
		}
		// for (distribution_record::const_iterator it=distribution.begin();it!=distribution.end();it++)
		// {
		// 	counter_t freq1=dist[it->first];
		// 	counter_t freq2=it->second;
		// 	int AE=(int)freq1-(int)freq2;
		// 	double RE=(double)AE/(double)freq2;
		// 	sum+=fabs(RE);cnt++;
		// }
		return sum/(double)cnt;
	}
	double calc_AAE(distribution_record &distribution)
	{
		double sum=0.0;
		int cnt=0;
		for (int i = 1; i < max_count; ++i)
		{
			counter_t freq1=dist[i];
			counter_t freq2=distribution.count(i)? distribution[i] : 0;
			int AE=(int)freq1-(int)freq2;
			sum+=fabs(AE);cnt++;
		}
		// for (distribution_record::const_iterator it=distribution.begin();it!=distribution.end();it++)
		// {
		// 	counter_t freq1=dist[it->first];
		// 	counter_t freq2=it->second;
		// 	int AE=(int)freq1-(int)freq2;
		// 	sum+=fabs(AE);cnt++;
		// }
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
	}
	void printestimation()
	{
		for (int i=1;i<=2000;i++) printf("%d %d\n",i,dist[i]);
	}
};

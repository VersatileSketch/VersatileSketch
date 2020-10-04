#include"BOBHash32.h"
#include"param.h"
#include"versatile_sketch.h"
#include "CM_sketch.h"
#include "CU_sketch.h"
TRACE trace;
TRACE old_trace;
FILE *fi;
//使用CAIDA2018，8位时间戳+8位有效载荷 
void ReadInData(int packet_num)
{
	if ((packet_num>MAX_PACKET_NUM)||(packet_num<=0))
	{
		printf("only %d packets in the dataset, but you require %d\n", MAX_PACKET_NUM, packet_num);
		exit(1);
	}
	trace.clear();
	old_trace.clear();
	FILE *fin=fopen("../../../data/formatted00.dat","rb");
	uint8_t buf[16];
	uint32_t packet_cnt=0;
	int start=60000;
	while (fread(buf,1,16,fin)==16)
	{
		FIVE_TUPLE tmp;
		memcpy(tmp.key,buf+8,FIVE_TUPLE_LEN);
		trace.push_back(tmp);
		packet_cnt++;
		if (packet_cnt==start) break;
	}
	packet_cnt=0;
	int delta = packet_num-start;
	while (fread(buf,1,16,fin)==16)
	{
		FIVE_TUPLE tmp;
		memcpy(tmp.key,buf+8,FIVE_TUPLE_LEN);
		old_trace.push_back(tmp);
		packet_cnt++;
		if (packet_cnt==3*delta) break;
	}
	for (int j = 0; j < 3; ++j)
	{
		int st = delta*j;
		for (int i = 0; i < (delta/3); ++i)
		{
			trace.push_back(old_trace[i+st]);
		}
	}
	fclose(fin);
}
const int my_bucket_num=1000*100; //1M memory 
const int cm_bucket_num=1000*250;
const int hash_num=2;
const int max_count=100000;
double real_entropy[100];
void calc_real_freq(int start,int times,int step)
{
	FREQ_RECORD real_freq;
	for (int i=0;i<start;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		real_freq[key]++;
	}
	for (int k=0;k<times;k++)
	{
		int packet_num=start+k*step;
		double sum=0.0;
		for (FREQ_RECORD::const_iterator it=real_freq.begin();it!=real_freq.end();it++)
		{
			counter_t freq=it->second;
			double px=(double)freq/(double)packet_num;
			sum+=px*log(px)/log(2);
		}
		real_entropy[k]=-sum;
		printf("%d %.5lf\n",packet_num,real_entropy[k]);
		// fprintf(fi, "%lf ", real_entropy[k]);
		for (int i=packet_num;i<=packet_num+step;i++)
		{
			string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
			real_freq[key]++;
		}
	}
}
void test_versatile_sketch(int start,int times,int step,int my_bucket_num)
{
	versatile_sketch sketch(my_bucket_num,hash_num,max_count);
	for (int i=0;i<start;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		sketch.insert(key.c_str());
	}
	for (int k=0;k<times;k++)
	{
		int packet_num=start+k*step;
		double entropy=sketch.calc_entropy();
		// if (k==times-1)
		printf("VS %d %.5lf %.5lf\n",packet_num,entropy,fabs(entropy-real_entropy[k])/real_entropy[k]);
		if (k==times-1)
		fprintf(fi, "%lf ", fabs(entropy-real_entropy[k])/real_entropy[k]);
		for (int i=packet_num;i<=packet_num+step;i++)
		{
			string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
			sketch.insert(key.c_str());
		}
	}
}

void test_CM_sketch(int start,int times,int step,int cm_bucket_num)
{
	CM_sketch sketch(cm_bucket_num,4,max_count);
	for (int i=0;i<start;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		sketch.insert(key.c_str());
	}
	for (int k=0;k<times;k++)
	{
		int packet_num=start+k*step;
		double entropy=sketch.calc_entropy();
		// if (k==times-1)
		printf("CM %d %.5lf %.5lf\n",packet_num,entropy,fabs(entropy-real_entropy[k])/real_entropy[k]);
		if (k==times-1)
		fprintf(fi, "%lf ", fabs(entropy-real_entropy[k])/real_entropy[k]);
		for (int i=packet_num;i<=packet_num+step;i++)
		{
			string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
			sketch.insert(key.c_str());
		}
	}
}

void test_CU_sketch(int start,int times,int step,int cm_bucket_num)
{
	CU_sketch sketch(cm_bucket_num,4,max_count);
	for (int i=0;i<start;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		sketch.insert(key.c_str());
	}
	for (int k=0;k<times;k++)
	{
		int packet_num=start+k*step;
		double entropy=sketch.calc_entropy();
		// if (k==times-1)
		printf("CU %d %.5lf %.5lf\n",packet_num,entropy,fabs(entropy-real_entropy[k])/real_entropy[k]);
		if (k==times-1)
		fprintf(fi, "%lf ", fabs(entropy-real_entropy[k])/real_entropy[k]);
		for (int i=packet_num;i<=packet_num+step;i++)
		{
			string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
			sketch.insert(key.c_str());
		}
	}
}

int main()
{
	ReadInData(300000);
	fi = fopen("log_etp_re", "w");
	// freopen("result.txt","w",stdout);
	for (int i = 0; i <= 10; ++i)
    {
        // memory - KB
        int mem = i * 200;
        if (i == 0) mem = 50;
        printf("Mem = %d KB\n", mem);

		int start=60000,times=10,step=20000;
		calc_real_freq(start,times,step);
		// fprintf(fi, "\n");
		test_versatile_sketch(start,times,step,mem*100);
		fprintf(fi, "\n");
		test_CM_sketch(start,times,step,mem*250);
		fprintf(fi, "\n");
		test_CU_sketch(start,times,step,mem*250); 

		fprintf(fi, "\n");
	}
	return 0; 
}

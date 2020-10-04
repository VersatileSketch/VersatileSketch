#include"BOBHash32.h"
#include"param.h"
#include"versatile_sketch.h"
#include"CM_sketch.h"
#include"CU_sketch.h"
TRACE trace;
FILE *fi;
int real_ans;

void ReadInData(int packet_num)
{
	if ((packet_num>MAX_PACKET_NUM)||(packet_num<=0))
	{
		printf("only %d packets in the dataset, but you require %d\n", MAX_PACKET_NUM, packet_num);
		exit(1);
	}
	trace.clear();
	FILE *fin=fopen("../../data/formatted00.dat","rb");
	uint8_t buf[16];
	uint32_t packet_cnt=0;
	while (fread(buf,1,16,fin)==16)
	{
		FIVE_TUPLE tmp;
		memcpy(tmp.key,buf+8,FIVE_TUPLE_LEN);
		trace.push_back(tmp);
		packet_cnt++;
		if (packet_cnt==packet_num) break;
	}
	fclose(fin);
}
void ReadInDataWP(int packet_num)
{
	if ((packet_num>MAX_PACKET_NUM)||(packet_num<=0))
	{
		printf("only %d packets in the dataset, but you require %d\n", MAX_PACKET_NUM, packet_num);
		exit(1);
	}
	trace.clear();
	FILE *fin=fopen("../../data/webdocs_form00.dat","rb");
	uint8_t buf[16];
	uint32_t packet_cnt=0;
	while (fread(buf,1,13,fin)==13)
	{
		FIVE_TUPLE tmp;
		memcpy(tmp.key,buf,FIVE_TUPLE_LEN);
		trace.push_back(tmp);
		packet_cnt++;
		if (packet_cnt==packet_num) break;
	}
	fclose(fin);
}
//const int bucket_num=65536; //1M memory 
const int hash_num=2;
const int max_count=100000;
const int packet_num=1000000;
void test_versatile_sketch(int bucket_num)
{
	versatile_sketch sketch(bucket_num,hash_num,max_count);
	FREQ_RECORD real_freq;
	for (int i=0;i<packet_num;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		real_freq[key]++;
		sketch.insert(key.c_str());
	}
	distribution_record distribution;
	for (FREQ_RECORD::const_iterator it=real_freq.begin();it!=real_freq.end();it++)
		distribution[it->second]++;
	// printf("versatile %.4lf %.4lf %.4lf %d\n",
	// sketch.calc_ARE(distribution),sketch.calc_AAE(distribution),sketch.calc_WMRE(distribution), sketch.calc_DI());
	printf("VS %d\n", sketch.calc_DI());
	fprintf(fi, "%f ", (double)(real_ans - sketch.calc_DI())/real_ans);
}
void test_CM_sketch(int bucket_num)
{
	CM_sketch sketch(bucket_num,4,max_count);
	FREQ_RECORD real_freq;
	for (int i=0;i<packet_num;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		real_freq[key]++;
		sketch.insert(key.c_str());
	}
	distribution_record distribution;
	for (FREQ_RECORD::const_iterator it=real_freq.begin();it!=real_freq.end();it++)
		distribution[it->second]++;
	// printf("CM %.4lf %.4lf %.4lf %d\n",
	// sketch.calc_ARE(distribution),sketch.calc_AAE(distribution),sketch.calc_WMRE(distribution), sketch.calc_DI());
	printf("CM %d\n", sketch.calc_DI());
	fprintf(fi, "%f ", (double)(real_ans - sketch.calc_DI())/real_ans);
}
void test_CU_sketch(int bucket_num)
{
	CU_sketch sketch(bucket_num,4,max_count);
	FREQ_RECORD real_freq;
	for (int i=0;i<packet_num;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		real_freq[key]++;
		sketch.insert(key.c_str());
	}
	distribution_record distribution;
	for (FREQ_RECORD::const_iterator it=real_freq.begin();it!=real_freq.end();it++)
		distribution[it->second]++;
	// printf("CU %.4lf %.4lf %.4lf %d\n",
	// sketch.calc_ARE(distribution),sketch.calc_AAE(distribution),sketch.calc_WMRE(distribution), sketch.calc_DI());
	printf("CU %d\n", sketch.calc_DI());
	fprintf(fi, "%f ", (double)(real_ans - sketch.calc_DI())/real_ans);
}

int get_real_dist()
{
	FREQ_RECORD real_freq;
	for (int i=0;i<packet_num;i++)
	{
		string key((const char*)trace[i].key,FIVE_TUPLE_LEN);
		real_freq[key] = 1;
	}
	printf("real %d\n", real_freq.size());
	return real_freq.size();
}

int main()
{
	fi = fopen("result.txt", "w");
	ReadInData(packet_num);
	// freopen("result.txt","w",stdout);
	for (int i = 0; i <= 10; ++i)
    {
        // memory - KB
        int mem = i * 200;
        if (i == 0) mem = 50;

        printf("\nMem = %d KB\n", mem);
        real_ans = get_real_dist();
		test_versatile_sketch(mem*100);
		test_CM_sketch(mem*250);
		test_CU_sketch(mem*250);
		fprintf(fi, "\n");
	}
	return 0; 
}

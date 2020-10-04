#include<vector>
#include<iostream>
#include<unordered_map>
#include<string>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<math.h>
#include<deque>

#define FIVE_TUPLE_LEN 8
#define MAX_PACKET_NUM 14720318
#define MAX_FLOW_NUM 334400
#define counter_t uint32_t
#define CNT_MAX 0xffffffff

struct FIVE_TUPLE{ uint8_t key[FIVE_TUPLE_LEN]; };
typedef vector<FIVE_TUPLE> TRACE;
typedef unordered_map<string, int> FREQ_RECORD;

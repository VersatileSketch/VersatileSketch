#include "utils.h"
#include "BOBHash32.h"
#include "cm.h"
#include "cu.h"
#include "vs.h"
#include "as.h"
#include "ss.h"

#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <stdint.h>
using namespace std;

#define ft first
#define sc second

vector<int> flow;
string file_name;

/*
 * CAIDA 2016 Dataset
 * ----------------------------------
 * Item consists of 16B and last 8B will be the identifier.
 *
 */
void LoadData_CAIDA(char file[])
{
    BOBHash32 hash_id;
    // hash_id.initialize(0);
    hash_id.initialize(rand()%MAX_PRIME32);
    
    ifstream is(file, ios::in | ios::binary);
    char buf[2000] = {0};

    for (int i = 1; i <= 1e7; i++)
    {
        if(!is.read(buf, 16))
        {
            panic("Data Loading Error.\n");
        }
        flow.push_back(hash_id.run(buf+8, 8));
    }

    cout << "Loading complete." << endl;
}

/*
 * Web Page Dataset
 * ----------------------------------
 * Item consists of 13B and first 4B will be the identifier.
 *
 */
void LoadData_WebPage(char file[])
{
    BOBHash32 hash_id;
    // hash_id.initialize(0);
    hash_id.initialize(rand()%MAX_PRIME32);
    
    ifstream is(file, ios::in | ios::binary);
    char buf[2000] = {0};

    for (int i = 1; i <= 1e7; i++)
    {
        if(!is.read(buf, 13))
        {
            panic("Data Loading Error.\n");
        }
        flow.push_back(hash_id.run(buf, 4));
    }

    cout << "Loading complete. " << flow.size() << endl;
}

/*
 * Frequency
 * ----------------------------------
 *
 */
void FreqEstTest(vector<Sketch*>& sk, vector<int>& flow, bool init_first = false)
{
    static map<int, int> flow_cnt;
    int n = flow.size(), n_sk = sk.size();
    
    flow_cnt.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ]++;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();

        for (int i = 0; i < n; ++i)
        {
            // if(id == 0)
            //     printf("insert %d\n", flow[i]);
            sk[id]->insert(flow[i]);
        }
    }

    printf("%d\n", flow_cnt.size());

    // summary
    printf("Frequency Estimation Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out | ios::app);
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[id]->name);
        
        int sz = flow_cnt.size();
        double pr = 0, are = 0, aae = 0;

        for (auto item: flow_cnt)
        {
            int est = sk[id]->query_freq(item.ft);
            int err = abs(est - item.sc);
            // printf("%d %d\n", est, item.sc);
            // printf("  %3d: %11d\t%5d\t%5d\t%5d\t%5d\n", i, my_res[i].sc, my_res[i].ft,
            //                                  sk[id]->query_freq(my_res[i].sc),
            //                                  flow_cnt[my_res[i].sc], int(err));
            are += (double)err / item.sc;
            aae += (double)err;
            pr += (est == item.sc);

            // if (id == 0)
            // {
            //     int lo_est = ((VSketch*)sk[id])->query_freq_low(item.ft);
            //     if (lo_est > item.sc || est < item.sc)
            //     {
            //         printf("bad!\n");
            //     }
            //     fout << item.sc << ' ' << est << ' ' << lo_est << endl;
            // }
        }

        are /= sz;
        aae /= sz;
        pr /= sz;

        sk[id]->status();
        printf("Pr: %.3lf  ARE: %1.3lf(%.3lf)  AAE: %1.3lf(%.3lf)\n", pr, are, log10(are), aae, log10(aae));
        fout << pr << ' ' << are << ' ' << log10(are) << ' ';
        fout << aae << ' ' << log10(aae) << endl;
    }
}

/*
 * Dominant
 * ----------------------------------
 * Test the accuracy of dominant items.
 *
 */
void DominantTest(vector<Sketch*>& sk, vector<int>& flow, bool init_first = false)
{
    static map<int, int> flow_cnt;
    static map<int, bool> is_dom;
    int n = flow.size(), n_sk = sk.size();
    
    flow_cnt.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ]++;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();

        for (int i = 0; i < n; ++i)
        {
            // if(id == 0)
            //     printf("insert %d\n", flow[i]);
            sk[id]->insert(flow[i]);
        }
    }

    // printf("%d\n", flow_cnt.size());

    // summary
    printf("Dominant Test\n");
    printf("-------------------------------------\n");

    // is_dom.clear();
    // for (auto item: flow_cnt)
    //     is_dom[item.ft] = sk[0]->query_exist(item.ft);

    fstream fout(file_name, ios::out | ios::app);
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[id]->name);
        
        int sz = flow_cnt.size();
        double pr = 0, are = 0, aae = 0;

        for (auto item: flow_cnt)
        {
            // not a dominant item
            if (!sk[id]->query_exist(item.ft))
            // if (!is_dom[item.ft])
            {
                sz--;
                continue;
            }

            int est = sk[id]->query_freq(item.ft);
            int err = abs(est - item.sc);
            // printf("%d %d\n", est, item.sc);
            // printf("  %3d: %11d\t%5d\t%5d\t%5d\t%5d\n", i, my_res[i].sc, my_res[i].ft,
            //                                  sk[id]->query_freq(my_res[i].sc),
            //                                  flow_cnt[my_res[i].sc], int(err));
            are += (double)err / item.sc;
            aae += (double)err;
            pr += (est == item.sc);
        }

        are /= sz;
        aae /= sz;
        pr /= sz;

        if (id == 0)
        {
            printf("size: %d\n", sz);
        }

        sk[id]->status();
        printf("Pr: %.3lf  ARE: %1.3lf(%.3lf)  AAE: %1.3lf(%.3lf)\n", pr, are, log10(are), aae, log10(aae));
        fout << pr << ' ' << are << ' ' << log10(are) << ' ';
        fout << aae << ' ' << log10(aae) << endl;
    }
}

/*
 * Generation (Heavy VS & Light VS)
 * ----------------------------------
 * Make sure sk[0] and sk[1] are VSketches or other sketches supporting generation,
 * or you can edit the test codes.
 */
void GenerateTest(vector<Sketch*>& sk, vector<int>& flow, bool init_first = false)
{
    static map<int, int> flow_cnt;
    int n = flow.size(), n_sk = sk.size();
    
    flow_cnt.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ]++;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();

        for (int i = 0; i < n; ++i)
        {
            // if(id == 0)
            //     printf("insert %d\n", flow[i]);
            sk[id]->insert(flow[i]);
        }
    }

    sk.push_back(((VSketch*)sk[0])->generate_mouse());
    sk.push_back(((VSketch*)sk[1])->generate_mouse());
    n_sk += 2;
    sk[0] = ((VSketch*)sk[0])->generate_elephant();
    sk[1] = ((VSketch*)sk[1])->generate_elephant();

    // summary
    printf("Generation Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out | ios::app);
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[id]->name);
        
        int sz = flow_cnt.size();
        double pr = 0, are = 0, aae = 0;
        int r_sz = sz;

        for (auto item: flow_cnt)
        {
            if (item.sc > 500)
            {
                r_sz--;
                continue;
            }
            int est = sk[id]->query_freq(item.ft);
            int err = abs(est - item.sc);
            are += (double)err / item.sc;
            aae += (double)err;
            pr += (est == item.sc);
        }

        are /= r_sz;
        aae /= r_sz;
        pr /= r_sz;

        sk[id]->status();
        printf("Pr: %.3lf  ARE: %1.3lf(%.3lf)  AAE: %1.3lf(%.3lf)\n", pr, are, log10(are), aae, log10(aae));
        fout << pr << ' ' << are << ' ' << log10(are) << ' ';
        fout << aae << ' ' << log10(aae) << endl;
    }
}

/*
 * Top-k
 * ----------------------------------
 * 
 */
void TopkTest(vector<Sketch*>& sk, vector<int>& flow, int k, bool init_first = false)
{
    static map<int, int> flow_cnt;
    int n = flow.size(), n_sk = sk.size();
    
    flow_cnt.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ]++;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();

        for (int i = 0; i < n; ++i)
        {
            // printf("insert %d\n", flow[i]);
            sk[id]->insert(flow[i]);
        }
    }

    // summary
    static vector<PII> res;
    res.clear();
    map<int, int>::iterator it = flow_cnt.begin();
    while (it != flow_cnt.end())
    {
        res.push_back(mp((*it).sc, (*it).ft));
        it++;
    }
    sort(res.begin(), res.end(), greater<PII>());
    printf("Top-k Test\n");
    printf("-------------------------------------\n");
    printf("Real Result:\n");
    int sz = min(k, (int)res.size());
    for (int i = 0; i < sz; ++i)
    {
        printf("  %d: %d %d\n", i, res[i].ft, res[i].sc);
    }

    fstream fout(file_name, ios::out | ios::app);
    printf("\nOur Result:\n");
    for (int id = 0; id < n_sk; ++id)
    {
        vector<PII> my_res = sk[id]->query_topk(k);
        printf("%d) %s\n", id, sk[id]->name);
        
        int my_sz = my_res.size();
        double pr = 0, are = 0, aae = 0;

        for (int i = 0; i < my_sz; ++i)
        {
            for (int j = 0; j < sz; ++j)
                if (res[j].sc == my_res[i].sc)
                {
                    pr += 1;
                    break;
                }

            double err = fabs(sk[id]->query_freq(my_res[i].sc) - flow_cnt[my_res[i].sc]);
            are += err / flow_cnt[my_res[i].sc];
            aae += err;
        }

        pr /= sz;
        are /= my_sz;
        aae /= my_sz;

        sk[id]->status();
        printf("Pr: %1.3lf  ARE: %1.3lf(%.3lf)  AAE: %1.3lf(%.3lf)\n", pr, are, log10(are), aae, log10(aae));
        
        fout << pr << ' ' << are << ' ' << log10(are) << ' ';
        fout << aae << ' ' << log10(aae) << endl;
    }
}

/*
 * HeavyHitter
 * ----------------------------------
 * 
 */
void HeavyHitterTest(vector<Sketch*>& sk, vector<int>& flow, double threshold, bool init_first = false)
{
    static map<int, int> flow_cnt;
    int n = flow.size(), n_sk = sk.size();
    int pkt_cnt = (int)(threshold * n);
    
    flow_cnt.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ]++;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();

        for (int i = 0; i < n; ++i)
        {
            // printf("insert %d\n", flow[i]);
            sk[id]->insert(flow[i]);
        }
    }

    // summary
    static vector<PII> res;
    res.clear();
    map<int, int>::iterator it = flow_cnt.begin();
    while (it != flow_cnt.end())
    {
        res.push_back(mp((*it).sc, (*it).ft));
        it++;
    }
    sort(res.begin(), res.end(), greater<PII>());
    printf("HeavyHitter Test\n");
    printf("-------------------------------------\n");
    printf("Real Result:\n");
    int sz = res.size();
    for (int i = 0; i < sz; ++i)
    {
        if (res[i].ft < pkt_cnt)
        {
            sz = i;
            break;
        }
        printf("  %d: %d %d\n", i, res[i].ft, res[i].sc);
    }


    printf("\nOur Result:\n");
    for (int id = 0; id < n_sk; ++id)
    {
        vector<PII> my_res = sk[id]->query_heavyhitter(pkt_cnt);
        printf("%d) %s\n", id, sk[id]->name);
        
        int my_sz = my_res.size();
        double pr = 0, rr = 0, are = 0, aae = 0;

        for (int i = 0; i < my_sz; ++i)
        {
            for (int j = 0; j < sz; ++j)
                if (res[j].sc == my_res[i].sc)
                {
                    pr += 1;
                    rr += 1;
                    break;
                }

            double err = fabs(my_res[i].ft - flow_cnt[my_res[i].sc]);
            are += err / flow_cnt[my_res[i].sc];
            aae += err;
        }

        pr /= my_sz;
        rr /= sz;
        are /= my_sz;
        aae /= my_sz;

        double f1 = 2 * pr * rr / (pr + rr);

        sk[id]->status();
        printf("F1: %1.3lf  ARE: %1.6lf(%.3lf)  AAE: %1.6lf(%.3lf)\n", f1, are, log10(are), aae, log10(aae));
    }
}

/*
 * HeavyChange
 * ----------------------------------
 * Input sketch vectors must be corresponding.
 *
 */
void HeavyChangeTest(vector<Sketch*>& sk1, vector<Sketch*>& sk2,
                    vector<int>& flow, double threshold, bool init_first = false)
{
    static map<int, int> flow_cnt1, flow_cnt2;
    static vector<PII> res;
    int n = flow.size(), n_sk = sk1.size();
    int chg_pt = n / 2;
    int line = 0;

    if (sk1.size() != sk2.size())
    {
        printf("Wrong input format! (Sizes of two vectors must be equal)\n");
        return;
    }
    
    flow_cnt1.clear();
    flow_cnt2.clear();
    for (int i = 0; i < n; ++i)
    {
        flow_cnt1[ flow[i] ] = 0;
        flow_cnt2[ flow[i] ] = 0;
    }
    for (int i = 0; i < chg_pt; ++i)
        flow_cnt1[ flow[i] ]++;
    for (int i = chg_pt; i < n; ++i)
        flow_cnt2[ flow[i] ]++;

    res.clear();
    for (auto it : flow_cnt1)
    {
        int delta = abs(flow_cnt2[it.ft] - it.sc);
        line += delta;
        res.push_back(mp(delta, it.ft));
    }
    sort(res.begin(), res.end(), greater<PII>());

    line *= threshold;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
        {
            sk1[id]->init();
            sk2[id]->init();
        }

        for (int i = 0; i < chg_pt; ++i)
        {
            // printf("insert %d\n", flow[i]);
            sk1[id]->insert(flow[i]);
        }
        for (int i = chg_pt; i < n; ++i)
        {
            // printf("insert %d\n", flow[i]);
            sk2[id]->insert(flow[i]);
        }
    }

    // summary
    printf("HeavyChange Test\n");
    printf("-------------------------------------\n");
    printf("Real Result:\n");
    int sz = res.size();
    for (int i = 0; i < sz; ++i)
    {
        if (res[i].ft < line)
        {
            sz = i;
            break;
        }
        printf("  %d: %d %d\n", i, res[i].ft, res[i].sc);
    }


    fstream fout(file_name, ios::out | ios::app);
    printf("\nOur Result:\n");
    for (int id = 0; id < n_sk; ++id)
    {
        vector<PII> hh1 = sk1[id]->query_heavyhitter(line);
        vector<PII> hh2 = sk2[id]->query_heavyhitter(line);

        vector<PII> my_res;
        my_res.clear();
        for (int i = 0; i < hh1.size(); ++i)
            my_res.push_back(mp(hh1[i].ft - sk2[id]->query_freq(hh1[i].sc), hh1[i].sc));
        for (int i = 0; i < hh2.size(); ++i)
            my_res.push_back(mp(hh2[i].ft - sk1[id]->query_freq(hh2[i].sc), hh2[i].sc));
        sort(my_res.begin(), my_res.end(), greater<PII>());
        unique(my_res.begin(), my_res.end());

        printf("%d) %s\n", id, sk1[id]->name);
        
        int my_sz = my_res.size();
        for (int i = 0; i < my_sz; ++i)
            if (my_res[i].ft < line)
            {
                my_sz = i;
                break;
            }

        double pr = 0, rr = 0, are = 0, aae = 0;

        for (int i = 0; i < my_sz; ++i)
        {
            for (int j = 0; j < sz; ++j)
                if (res[j].sc == my_res[i].sc)
                {
                    pr += 1;
                    rr += 1;
                    break;
                }

            int fp = my_res[i].sc;
            int real_dlt = abs(flow_cnt1[fp] - flow_cnt2[fp]);
            double err = fabs(my_res[i].ft - real_dlt);
            are += err / real_dlt;
            aae += err;
        }

        pr /= my_sz;
        rr /= sz;
        are /= my_sz;
        aae /= my_sz;

        double f1 = 2 * pr * rr / (pr + rr);

        sk1[id]->status();
        printf("F1: %1.3lf  PR: %1.3lf  RR: %1.3lf  ARE: %1.6lf(%.3lf)  AAE: %1.6lf(%.3lf)\n",
                f1, pr, rr, are, log10(are), aae, log10(aae));

        fout << f1 << ' ' << pr << ' ' << rr << ' ';
        fout << are << ' ' << aae << endl;
    }
}

/*
 * Compression
 * ----------------------------------
 * Make sure sk[0] is VSketch or other sketches supporting compression,
 * or you can edit the test codes.
 */
void CompressTest(vector<Sketch*>& sk, vector<int>& flow, bool init_first = false)
{
    static map<int, int> flow_cnt;
    int n = flow.size(), n_sk = sk.size();
    
    flow_cnt.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ]++;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();

        for (int i = 0; i < n; ++i)
        {
            // if(id == 0)
            //     printf("insert %d\n", flow[i]);
            sk[id]->insert(flow[i]);
        }
    }

    sk.push_back(((VSketch*)sk[0])->compress());
    n_sk++;

    // summary
    printf("Compress Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out | ios::app);
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[id]->name);
        
        int sz = flow_cnt.size();
        double pr = 0, are = 0, aae = 0;
        int r_sz = sz;

        for (auto item: flow_cnt)
        {
            int est = sk[id]->query_freq(item.ft);
            int err = abs(est - item.sc);
            // printf("%d %d\n", est, item.sc);
            // printf("  %3d: %11d\t%5d\t%5d\t%5d\t%5d\n", i, my_res[i].sc, my_res[i].ft,
            //                                  sk[id]->query_freq(my_res[i].sc),
            //                                  flow_cnt[my_res[i].sc], int(err));
            are += (double)err / item.sc;
            aae += (double)err;
            pr += (est == item.sc);
        }

        are /= r_sz;
        aae /= r_sz;
        pr /= r_sz;

        sk[id]->status();
        printf("Pr: %.3lf  ARE: %1.3lf(%.3lf)  AAE: %1.3lf(%.3lf)\n", pr, are, log10(are), aae, log10(aae));
        fout << pr << ' ' << are << ' ' << log10(are) << ' ';
        fout << aae << ' ' << log10(aae) << endl;
    }
}

/*
 * Mergence
 * ----------------------------------
 * Make sure sk[0] and sk[1] are the same VSketch, or you can edit the test codes.
 *
 */
void MergeTest(vector<Sketch*>& sk, vector<int>& flow, bool init_first = false)
{
    static map<int, int> flow_cnt;
    int n = flow.size(), n_sk = sk.size();
    int hlf = n / 2;
    
    flow_cnt.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ]++;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();

        for (int i = 0; i < n; ++i)
        {
            // if(id == 0)
            //     printf("insert %d\n", flow[i]);
            if (i < hlf)
            {
                if (id != 1)
                    sk[id]->insert(flow[i]);
            }
            else
            {
                if (id != 0)
                    sk[id]->insert(flow[i]);
            }
        }
    }

    sk.push_back(((VSketch*)sk[0])->merge((VSketch*)sk[1]));
    n_sk++;

    // summary
    printf("Merge Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out | ios::app);
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s  %s\n", id, sk[id]->name, id<=1?"<invalid>":"");
        
        int sz = flow_cnt.size();
        double pr = 0, are = 0, aae = 0;
        int r_sz = sz;

        for (auto item: flow_cnt)
        {
            int est = sk[id]->query_freq(item.ft);
            int err = abs(est - item.sc);
            are += (double)err / item.sc;
            aae += (double)err;
            pr += (est == item.sc);
        }

        are /= r_sz;
        aae /= r_sz;
        pr /= r_sz;

        sk[id]->status();
        printf("Pr: %.3lf  ARE: %1.3lf(%.3lf)  AAE: %1.3lf(%.3lf)\n", pr, are, log10(are), aae, log10(aae));
        fout << pr << ' ' << are << ' ' << log10(are) << ' ';
        fout << aae << ' ' << log10(aae) << endl;
    }
}


/*
 * Expansion
 * ----------------------------------
 * Make sure sk[0] is VSketch or other sketches supporting expansion,
 * or you can edit the test codes.
 */
void ExpandTest(vector<Sketch*>& sk, vector<int>& flow, bool init_first = false)
{
    int epd_time = 2;
    int n_itr = 10;

    static map<int, int> flow_cnt;
    static map<int, bool> flow_vis;
    int n = flow.size(), n_sk = sk.size();
    int seg = n / ((epd_time+1) * n_itr);

    n = (n/seg) * seg;
    
    flow_cnt.clear();
    flow_vis.clear();
    for (int i = 0; i < n; ++i)
        flow_cnt[ flow[i] ] = 0;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();
    }


    // summary
    printf("Expand Test\n");
    printf("-------------------------------------\n");
    printf("seg: %d flow_size: %d\n", seg, n);

    fstream fout(file_name, ios::out | ios::app);
    fout << seg << endl;
    
    int now_flow = 0;
    for (int i = 0; i <= epd_time; ++i)
    {
        for (int j = 0; j < n_itr; ++j)
        {
            for (int k = 0; k < seg; ++k)
            {
                flow_cnt[ flow[now_flow] ]++;
                flow_vis[ flow[now_flow] ] = true;
                for (int id = 0; id < n_sk; ++id)
                    sk[id]->insert(flow[now_flow]);
                now_flow++;
            }

            printf("Exp: %d  ----   Iteration: %d\n", i, j+1);
            for (int id = 0; id < n_sk; ++id)
            {
                printf("%d) %s\n", id, sk[id]->name);
        
                int sz = now_flow;
                double pr = 0, are = 0, aae = 0;
                int r_sz = sz;

                for (auto item: flow_vis)
                {
                    int est = sk[id]->query_freq(item.ft);
                    int err = abs(est - flow_cnt[item.ft]);
                    are += (double)err / flow_cnt[item.ft];
                    aae += (double)err;
                    pr += (est == flow_cnt[item.ft]);
                }

                are /= r_sz;
                aae /= r_sz;
                pr /= r_sz;

                sk[id]->status();
                printf("Pr: %.3lf  ARE: %1.3lf(%.3lf)  AAE: %1.3lf(%.3lf)\n", pr, are, log10(are), aae, log10(aae));
                fout << pr << ' ' << are << ' ' << log10(are) << ' ';
                fout << aae << ' ' << log10(aae) << endl;
            }
        }

        sk[0] = ((VSketch*)sk[0])->expand();
    }
}

/*
 * ThroughputTest
 * ----------------------------------
 * RECOMMAND: Modify the code and remove additional parts to get maximum processing speed.
 *
 */
void ThroughputTest(vector<Sketch*>& sk, vector<int>& flow, bool init_first = false)
{
    int n = flow.size(), n_sk = sk.size();

    static map<int, bool> flow_vis;
    flow_vis.clear();

    for (int i = 0; i < n; ++i)
        flow_vis[ flow[i] ] = true;

    for (int id = 0; id < n_sk; ++id)
    {
        if (init_first)
            sk[id]->init();
    }

    // summary
    printf("Throughput Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out);
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[id]->name);

        Timer timer;
        double t1 =.0, t2=.0, t3=.0;
        double tp1, tp2, tp3;

        timer.Start();
        for (register int i = 0; i < n; ++i)
            sk[id]->insert(flow[i]);
        t1 = timer.Finish();

        timer.Start();
        for (register int i = 0; i < n; ++i)
        {
            int t = sk[id]->query_freq(flow[i]);
        }
        t2 = timer.Finish();

        timer.Start();
        for (auto item: flow_vis)
        {
            int t = sk[id]->query_freq(item.ft);
        }
        t3 = timer.Finish();

        tp1 = (double)n / 1e6 / t1;
        tp2 = (double)n / 1e6 / t2;
        tp3 = (double)flow_vis.size() / 1e6 / t3;

        sk[id]->status();
        printf("TP: (Insert)%.3lf(%.3lf)  (Query)%.3lf(%.3lf) %.3lf(%.3lf)\n", tp1, t1, tp2, t2, tp3, t3);
        fout << tp1 << ' ' << tp2 << ' ' << tp3 << ' ';
        fout << t1 << ' ' << t2 << ' ' << t3 << endl;
    }

}

int main(int argc, char *argv[])
{
    srand(2020);
    // parse args
    // ParseArg(argc, argv);

    // load data
    LoadData_CAIDA("data/formatted00.dat");
    // LoadData_WebPage("data/webdocs_form00.dat");
    
    // name of output log file
    file_name = string("log/log_cpr_caida");

    // normal test except HeavyChange
    if (true)
    {
        for (int i = 1; i <= 10; ++i)
        {
            // memory - KB
            int mem = i * 400;
            if (i == 0)
                mem = 50;

            vector<Sketch*> sk;
            CMSketch *cms = new CMSketch(mem * 250, 1);
            CUSketch *cus = new CUSketch(mem * 250, 4);
            VSketch *sbf[5], *sbf_low[5], *sbf_db[5];
            for (int j = 2; j <= 2; ++j)
            {
                sbf[j] = new VSketch(mem * 100, j, true);
                // sbf_db[j] = new VSketch(mem * 200, j, true);
                // sbf_low[j] = new VSketch(mem * 100, j, true);
                // sbf_low[j]->set_lowver(true);
                // sbf[j]->set_lowver(true);
                // sbf_db[j]->set_lowver(true);

                // sk.push_back(sbf_db[j]);
                sk.push_back(sbf[j]);
                // sk.push_back(sbf_low[j]);
            }
            ASketch *as = new ASketch(mem * 250, 4, 128);
            // SpaceSaving *ss = new SpaceSaving(mem * 38);

            // sk.push_back(cms);
            // sk.push_back(cus);
            // sk.push_back(new CUSketch(mem * 250, 4));
            // sk.push_back(new CUSketch(mem * 250, 4));
            // sk.push_back(as);
            // sk.push_back(ss);
            // sk.push_back(new VSketch(mem * 100, 2, true));

            // FreqEstTest(sk, flow, true);
            // DominantTest(sk, flow, true);
            // GenerateTest(sk, flow, true);
            // TopkTest(sk, flow, 200, true);
            // HeavyHitterTest(sk, flow, 1./1000, true);
            // DistributionTest(sk, flow, true);
            CompressTest(sk, flow, true);
            // MergeTest(sk, flow, true);
            // ThroughputTest(sk, flow, true);

            delete cms;
            delete cus;
            delete as;
            // delete ss;
            for (int j = 2; j <= 2; ++j)
            {
                delete sbf[j];
                // delete sbf_low[j];
                // delete sbf_db[j];
            }
        }
    }

    // HeavyChange
    if (false)
    {
        for (int i = 1; i <= 10; ++i)
        {
            // memory - KB
            int mem = i * 10;

            vector<Sketch*> sk1, sk2;
            VSketch *sbf[10];
            for (int j = 1; j <= 2; ++j)
            {
                sbf[j*2-2] = new VSketch(mem * 100, j, true);
                sbf[j*2-1] = new VSketch(mem * 100, j, true);
                sk1.push_back(sbf[j*2-2]);
                sk2.push_back(sbf[j*2-1]);
            }

            CMSketch *cms1 = new CMSketch(mem * 250, 4);
            CMSketch *cms2 = new CMSketch(mem * 250, 4);
            ASketch *as1 = new ASketch(mem * 250, 4, 256);
            ASketch *as2 = new ASketch(mem * 250, 4, 256);
            SpaceSaving *ss1 = new SpaceSaving(mem * 38);
            SpaceSaving *ss2 = new SpaceSaving(mem * 38);
            sk1.push_back(cms1);
            sk2.push_back(cms2);
            sk1.push_back(as1);
            sk2.push_back(as2);
            sk1.push_back(ss1);
            sk2.push_back(ss2);

            HeavyChangeTest(sk1, sk2, flow, 2./10000, true);

            delete cms1;
            delete cms2;
            delete as1;
            delete as2;
            delete ss1;
            delete ss2;
            for (int j = 0; j < 2; ++j)
            {
                delete sbf[j*2];
                delete sbf[j*2+1];
            }
        }
    }

    return 0;
}

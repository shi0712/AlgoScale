#include <bits/stdc++.h>
#ifndef __CAVE_H__
#define __CAVE_H__

#ifdef __cplusplus
extern "C" {
#endif

int tryCombination(int S[]);
void answer(int S[], int D[]);
void exploreCave(int N);

#ifdef __cplusplus
}
#endif

#endif /* __CAVE_H__ */

#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <vector>
using namespace std;


#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

namespace cave_policy {
constexpr int levels=12;
constexpr std::array<int,13> ratios{260,320,350,375,397,410,420,430,440,459,480,500,550};
using Counts=std::array<int,levels+2>;
struct Policy {
    int origin=0;
    std::vector<std::array<int,2>> cap;
    int needed(int count,int known_open) const {
        if(!count)return 0;
        for(int i=0;i<int(cap.size());++i)if(cap[i][known_open]>=count)return origin+i;
        assert(false);return 0;
    }
};
struct Plan {
    int origin=0,ratio=0;
    Counts zero{},stop{};
    std::vector<Counts> rows;
    int capacity(int B,int t) const {
        if(B<origin)return 0;
        return rows[std::min(B-origin,int(rows.size())-1)][t];
    }
};
inline Plan plan(int k,int ratio,const Policy& future) {
    Plan s;s.ratio=ratio;
    int z=k-1,other=0;
    s.stop[0]=1+future.needed(k-1,0);
    for(int t=1;t<=levels+1;++t) {
        s.zero[t]=z;
        s.stop[t]=std::max(future.needed(z,0),future.needed(other,1));
        int new_zero=z*ratio/1000;
        other=std::max((other+1)/2,z-new_zero);z=new_zero;
    }
    s.origin=*std::min_element(s.stop.begin(),s.stop.end());
    Counts prev{};
    for(int B=s.origin;;++B) {
        Counts c{};
        c[0]=std::min(k,std::max(int(B>=s.stop[0]),prev[0]+prev[1]));
        for(int t=1;t<=levels+1;t++) {
            int count=std::max(int(B>=s.stop[t]),2*prev[t]);
            if(t<=levels)
                count=std::max(count,std::min(prev[t],s.zero[t]-s.zero[t+1])+std::min(prev[t+1],s.zero[t+1]+1));
            c[t]=std::min(k,count);
        }
        s.rows.push_back(c);
        if(c[0]==k && c[1]==k)break;
        prev=c;
    }
    return s;
}
inline std::vector<Policy> precompute(int n) {
    std::vector<Policy> stages(n+1);
    stages[1].cap.push_back({1,1});
    for(int k=2;k<=n;k++) {
        std::vector<Plan> plans;
        int first=1000000,last=0;
        for(int ratio:ratios) {
            plans.push_back(plan(k,ratio,stages[k-1]));
            const Plan& p=plans.back();
            first=std::min(first,p.origin);
            last=std::max(last,p.origin+int(p.rows.size())-1);
        }
        auto &s=stages[k];s.origin=first;
        for(int B=first;B<=last;B++) {
            std::array<int,2> c{};
            for(const Plan& p:plans)for(int t=0;t<2;t++)c[t]=std::max(c[t],p.capacity(B,t));
            c[1]=std::max(c[1],c[0]);s.cap.push_back(c);
            if(c[0]==k && c[1]==k)break;
        }
    }
    return stages;
}
}

using namespace cave_policy;

struct Observation {vector<int> switches;int reply;};
static bool open_at(int reply,int door) {return reply<0 || reply>door;}
void exploreCave(int n) {
    auto stages=precompute(n);
    vector<int> unused(n),answer_state(n),answer_door(n);
    iota(unused.begin(),unused.end(),0);
    vector<int> zero(n);
    vector<Observation> seed{{zero,tryCombination(zero.data())}};
    for(int door=0;door<n;door++) {
        int k=unused.size();
        const Observation &base=seed[0];
        vector<int> candidates,required(n);
        for(int i:unused) {
            required[i]=base.switches[i]^!open_at(base.reply,door);
            if(all_of(seed.begin()+1,seed.end(),[&](const Observation &r){
                return (required[i]==r.switches[i])==open_at(r.reply,door);
            }))
                candidates.push_back(i);
        }
        auto open_seed=find_if(seed.begin(),seed.end(),[&](const Observation& o){return open_at(o.reply,door);});
        int m0=candidates.size();
        int budget=stages[k].needed(m0,open_seed!=seed.end());
        vector<Observation> opened;
        vector<int> groups(n);
        Plan stage;
        if(k>1) for(int ratio:ratios) {
            stage=plan(k,ratio,stages[k-1]);
            if(open_seed!=seed.end() && stage.capacity(budget,1)>=m0) {
                opened.push_back(*open_seed);break;
            }
            if(stage.capacity(budget,0)>=m0) break;
        }
        while(k>1) {
            int t=opened.size(),m=candidates.size();
            if(m==1 && budget>=stage.stop[t]) break;
            assert(m<=stage.capacity(budget,t));
            int fails=min(m,stage.capacity(budget-1,t));
            bool extend=t==0;
            if(t>=1 && t<=levels) {
                int balanced_fails=min(fails,stage.zero[t]-stage.zero[t+1]);
                if(m-balanced_fails<=min(stage.capacity(budget-1,t+1),1+stage.zero[t+1])) {
                    extend=true;fails=balanced_fails;
                }
            }
            assert(m-fails<=stage.capacity(budget-1,t+int(extend)));
            vector<int> query=t==0?base.switches:opened[0].switches;
            vector<char> is_candidate(n);
            for(int j=0;j<m;j++) {
                int sw=candidates[j];
                query[sw]=required[sw]^(j<fails);
                is_candidate[sw]=true;
            }
            if(t>=1 && extend) {
                vector<int> flips(1<<(t-1));
                for(int i:unused) ++flips[groups[i]];
                --flips[0]; // the true current switch must stay in group zero on success
                for(int& size:flips) size/=2;
                flips[0]=stage.zero[t]-stage.zero[t+1]-fails;
                for(int i:unused) if(!is_candidate[i] && flips[groups[i]]>0) {
                    query[i]^=1;--flips[groups[i]];
                }
                assert(all_of(flips.begin(),flips.end(),[](int left){return left==0;}));
            }
            int reply=tryCombination(query.data());--budget;
            if(open_at(reply,door)) {
                candidates.erase(candidates.begin(),candidates.begin()+fails);
                if(extend) {
                    if(t>=1) for(int i:unused)
                        if(query[i]!=opened[0].switches[i]) groups[i]|=1<<(t-1);
                    opened.push_back({move(query),reply});
                }
            } else candidates.resize(fails);
        }
        assert(candidates.size()==1);
        int sw=candidates.front();
        answer_door[sw]=door;answer_state[sw]=required[sw];
        unused.erase(find(unused.begin(),unused.end(),sw));
        if(k==1) break;
        if(opened.empty()) {
            if(open_at(base.reply,door)) opened.push_back(base);
            else {
                vector<int> query=base.switches;
                query[sw]=required[sw];
                int reply=tryCombination(query.data());
                opened.push_back({move(query),reply});
            }
        }
        seed=move(opened);
    }
    answer(answer_state.data(),answer_door.data());
}

static int public_n;
static std::string switch_string(const int* s){
    std::string bits(public_n,'0');
    for(int i=0;i<public_n;i++)bits[i]=char('0'+s[i]);
    return bits;
}
extern "C" int tryCombination(int s[]){
    std::cout<<"? "<<switch_string(s)<<std::endl;
    int response;if(!(std::cin>>response))std::exit(0);
    return response;
}
extern "C" void answer(int s[],int d[]){
    std::cout<<"! "<<switch_string(s);
    for(int i=0;i<public_n;i++)std::cout<<' '<<d[i];
    std::cout<<std::endl;std::exit(0);
}
int main(){
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    if(std::cin>>public_n)exploreCave(public_n);
}

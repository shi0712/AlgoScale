#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <set>
#include <vector>
using namespace std;
using Menu=vector<int>;
using Plan=vector<Menu>;

Plan cycles(int v,int r,const Menu& jumps) {
    Plan plan;
    for(int step:jumps) {
        int count=gcd(step,v);
        for(int root=0;root<count;++root) {
            Menu cycle;
            for(int a=root;;) {cycle.push_back(a);a=(a+step)%v;if(a==root)break;}
            plan.push_back(cycle);
            if(int(cycle.size())>r+1) {
                int w=min(r,int(cycle.size())/2);
                Menu wrap(cycle.end()-w,cycle.end());
                wrap.insert(wrap.end(),cycle.begin(),cycle.begin()+w);
                plan.push_back(wrap);
            }
        }
    }
    return plan;
}

Plan expand(const Plan& groups,int n,int v) {
    Plan out;
    for(const Menu& blocks:groups) {
        Menu line;
        for(int b:blocks)
            for(int i=b*n/v;i<(b+1)*n/v;++i) line.push_back(i);
        if(!line.empty())out.push_back(line);
    }
    return out;
}

int tastes(const Plan& p) {
    int total=0; for(const auto& row:p)total+=row.size();
    return total;
}

Plan trim(Plan p,int n,int k,int mode) {
    vector<vector<int>> counts(n,vector<int>(n));
    auto count=[&](int a,int b)->int& {return a<b?counts[a][b]:counts[b][a];};
    for(const Menu& row:p)for(int i=0;i<int(row.size());++i)
        for(int j=i+1;j<int(row.size())&&j<=i+k;++j)++count(row[i],row[j]);
    if(mode&1)reverse(p.begin(),p.end());
    if(mode&2)for(Menu& m:p)reverse(m.begin(),m.end());
    bool progress=true;
    while(progress) {
        progress=false;
        for(Menu& m:p) {
            for(int i=0;i<int(m.size());) {
                int a=max(0,i-k),b=min(int(m.size())-1,i+k);
                bool removable=true;
                for(int j=a;j<=b;++j) if(j!=i&&count(m[i],m[j])<=1){removable=false;break;}
                if(!removable){++i;continue;}
                for(int j=a;j<=b;++j) if(j!=i)--count(m[i],m[j]);
                for(int j=a;j<i&&j+k+1<int(m.size());++j)++count(m[j],m[j+k+1]);
                m.erase(m.begin()+i);
                progress=true;
            }
        }
    }
    p.erase(remove_if(p.begin(),p.end(),[](const Menu& x){return x.empty();}),p.end());
    return p;
}

Plan base_plan(int n,int k) {
    if(n==1)return {};
    int size=max(1,k/2),v=n/size;
    Plan groups;
    for(int center=0;center<v/2;++center) {
        Menu line;
        for(int i=0;i<v;++i) {
            int offset=(i%2==0?-(i/2):(i+1)/2);
            line.push_back((center+offset+v)%v);
        }
        groups.push_back(line);
    }
    return expand(groups,n,v);
}

Plan pack1024() {
    return {
        {21,7,13,2,14,19,12,5,20,0,1,4,27,16,30,22,8,18,28,17,3,10,29,25,23,11,15,9,31,24,26,6},
        {1,31,28,3,13,12,23,4,18,26,11,7,5,22,16,10,24,30,25,8,0,2,20,6,29,15,14,27,17,9,21,19},
        {29,8,31,5,26,30,21,14,28,10,11,27,0,19,23,15,16,22,4,2,3,24,9,20,13,18,6,25,1,12,7,17},
        {10,2,20,31,7,27,23,6,24,5,28,17,15,18,4,21,0,8,14,3,11,13,12,16,30,9,29,1,22,26,19,25},
        {3,5,27,26,13,12,10,15,8,1,2,23,17,11,30,20,6,22,21,16,14,25,31,18,4,19,29,24,28,7,0,9},
        {15,30,3,7,19,6,8,4,10,9,27,5,18,25,2,28,16,26,20,17,0,13,31,22,29,12,11,21,24,1,23,14},
        {3,21},
    };
}
Plan pack512() {
    return {
        {6,31,32,48,60,23,28,54,30,19,10,3,56,34,17,20,9,29,62,15,24,45,36,16,49,21,38,57,33,50,13,2,47,59,55,4,53,35,27,61,7,8,22,63,46,51,42,26,25,0,11,39,40,52,37,43,58,18,41,5,44,12,14,1},
        {10,57,55,41,32,49,52,22,25,24,4,20,23,51,44,45,38,61,59,0,35,3,11,2,46,29,14,54,37,13,8,26,31,56,62,1,60,30,40,5,27,42,47,28,33,15,34,63,53,39,50,43,36,19,21,6,12,17,58,7,16,18,9,48},
        {28,9,33,29,32,3,4,62,37,60,46,40,50,5,16,23,15,56,25,18,14,27,36,38,1,7,13,55,39,42,19,20,12,63,49,2,51,47,58,24,0,53,10,31,52,21,44,34,30,8,45,35,41,43,54,11,17,22,59,48,6,57,61,26},
        {22,5,2,62,4,18,0,17,33,23,55,26,12,45,10,28,46,48,35,47,56,7,21,3,61,39,36,31,30,51,16,14,53,52,57,20,27,11,13,19,60,44,29,15,40,43,54,6,38,63,41,25,9,58,50,8,32,42,24,34,1,49,59,37},
        {46,4,34,16,57,35,37,5,24,38,13,22,56,28,36,0,50,44,54,7,41,31,14,62,40,48,12,47,25,30,43,17,49,26,60,53,39,58,29,10,42,59,63,45,32,33,1,19,20,61,2,18,23,52,8,21,51,55,11,3,15,9,27,6},
        {51,12,56,27,9,37,23,2,10,38,36,40,20,46,58,15,33,57,22,1,21,41,3,26,48,24,34,54,18,4,31,59,50,25,60,45,19,52,17,35,5,63,44,39,47,32,61,14,11,49,43,28,55,16,62,13,42,53,30,6,7,29,0,8},
        {46,56,55,61,58,30,15,14,17,10,50,22,53,44,37,28,24,11,12,31,34,43,38,2,42,60,18,21,13,32,35,25,36,62,8,59,57,40,9,45,3,49,5,54,39,33,27,48,52,0,1,51,63,16,29,23,41,26,7,19,4,20,47,6},
        {54,20,16,32,45,22,27,58,29,49,48,56,44,42,11,23,57,53,36,40,41,33,60,61,62,43,5,51,34,6,37,55,24,19,14,0,9,21,13,30,25,46,52,7,59,12,39,38,8,4,15,17,1,31,2,47,28,3,10,18,63,35,50,26},
        {38,52,62,9,47,39,22,23,35,31,1,58,42,37,17,36,63,60,48,55,29,50,12,61,34,13,49,40,18,19,51,57,54,7,56,45,2,53,25,44,3,10,6,16,33,11,8,4,30,28,14,20,59,26,5,15,0,32,43,46,27,41,24,21},
        {34,41,0,47,20,45,37,21,2,15,48,53,5,19,8,31,46,49,6,23,62,14,13,63,3,58,11,38,1,28,29,25,17,57,39,27,16,44,59,33,43,56,24,7,32,60,10,51,35,9,61,42,4,54,52,36,12,26,40,22,18,55,30,50},
        {42,14,60,22,34,23,19,58,59,62,54,21,5,47,29,11,18,36,45,4,13,48,43,10,8,20,1,3,50,52,6,51,56,28,39,41,24,2,30,57,63,27,31,26,55,9,44,46,38,53,17,32,12,40,15,35,7,49,0,37,33,61,25,16},
        {20,29,33,35,31,14,6,45,39,18,46,53,1,54,55,9,25,5,36,34,7,11,27,62,50,10,44,49,58,4,56,21,40,63,28,24,61,52,17,13,15,51,41,59,42,48,37,30,38,32,26,19,2,47,16,8,60,12,0,57,3,22,43,23},
        {61,10,56,5,46,7,28,57,58,26,47,38,53,36},
        {30,12,33,51},
        {4,63,41},
        {55,20,48,31,60,9,43,6,1,2,40},
        {3,42},
        {24,50},
        {29,52},
    };
}

Plan make_plan(int n,int k) {
    if(n*n/k<8192||k==1)return base_plan(n,k);
    vector<Plan> options;
    if(n==128)options.push_back(expand(cycles(131,2,{2,3,7,8,10,12,15,17,18,19,22,23,25,26,27,28,29,31,32,37,39,40,42,43,45,48,49,55,59,60,61,63,65}),n,131));
    if(n==256) {
        Menu step;for(int a=1;a<=43;a+=2)step.push_back(a);
        options.push_back(expand(cycles(86,2,step),n,86));
    }
    if(n==512) options.push_back(expand(pack512(),n,64));
    if(n==1024) options.push_back(expand(pack1024(),n,32));
    Plan best=base_plan(n,k);
    for(const auto& p:options)for(int mode=0;mode<4;++mode) {
        Plan result=trim(p,n,k,mode);
        if(tastes(result)<tastes(best))best=std::move(result);
    }
    return best;
}
#ifndef COFFEE_CERT
int main() {
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n,k;if(!(cin>>n>>k))return 0;
    Plan plan=make_plan(n,k);
    vector<bool> live(n,true);
    for(const auto& menu:plan) {
        cout<<"R"<<endl;
        for(int id:menu) if(live[id]) {
            cout<<"? "<<id+1<<endl;
            char answer;if(!(cin>>answer)||answer=='E')return 0;
            if(answer=='Y')live[id]=false;
        }
    }
    cout<<"! "<<count(live.begin(),live.end(),true)<<endl;
}
#endif

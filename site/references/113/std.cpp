#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
using namespace std;
constexpr int small=8, max_questions=61;
array<long long,75> fib;

// State 2*t is a length-one run of truth flag t; 2*t+1 is a length-two run.
int next_state(int s,int truth) {
    if(s==4 || truth!=s/2)return 2*truth;
    return (s%2) ? -1 : s+1;
}
struct Edge {int a,b;unsigned char l,r;};
struct Endings {
    vector<unsigned char> cost;
    vector<pair<unsigned char,unsigned char>> choice;
    array<int,small+1> worst{};
    Endings(){
        int limit=2<<(2*small);
        cost.assign(limit,100);choice.resize(limit);
        vector<vector<Edge>> edges(limit);
        for(int m=0;m<=small;++m) {
            const int first=1<<(2*m);
            for(int code=first;code<2*first;++code) {
                if(m<=2){cost[code]=0;continue;}
                array<int,small> states{};
                int temp=code;
                for(int i=m-1;i>=0;--i){states[i]=temp%4;temp/=4;}
                for(int l=0;l<m;++l)for(int r=l;r<m;++r) {
                    int child[2]={1,1};
                    for(int i=0;i<m;++i)for(int reply=0;reply<=1;++reply){
                        int truth=((l<=i && i<=r)==bool(reply));
                        int s=next_state(states[i],truth);
                        if(s>=0)child[reply]=4*child[reply]+s;
                    }
                    edges[code].push_back({child[0],child[1],(unsigned char)l,(unsigned char)r});
                }
            }
        }
        int unresolved=0;
        for(int m=3;m<=small;++m)unresolved+=1<<(2*m);
        for(int depth=1;unresolved;++depth){
            int changed=0;
            for(int m=3;m<=small;++m) {
                const int first=1<<(2*m);
                for(int s=first;s<2*first;++s)if(cost[s]==100) {
                    for(auto e:edges[s])if(cost[e.a]<depth && cost[e.b]<depth) {
                        cost[s]=depth;choice[s]={e.l,e.r};++changed;break;
                    }
                }
            }
            if(!changed)abort();
            unresolved-=changed;
        }
        for(int m=3;m<=small;++m){
            int first=1<<(2*m);
            worst[m]=*max_element(cost.begin()+first,cost.begin()+2*first);
            worst[m]=max(worst[m],worst[m-1]);
        }
    }
};

int survivors(int n,int q) {
    long long v=2LL*n*fib[q+1];
    for(int h=q;h>0;--h)v=(v+(h==q?0:fib[h+1]))/2;
    return int(min<long long>(n,v));
}
struct Parameters {
    vector<int> rounds,total;
    Parameters(const Endings& tail):rounds(100001),total(100001,100) {
        // Initial sizes handled by q weighted queries plus at most m survivors.
        for(int q=1;q<=max_questions;++q)for(int m=2;m<=small;++m){
            int lo=0,hi=100000;
            while(lo<hi){int md=(lo+hi+1)/2;if(survivors(md,q)<=m)lo=md;else hi=md-1;}
            for(int n=3;n<=lo;++n)if(q+tail.worst[m]<total[n]){
                rounds[n]=q;total[n]=q+tail.worst[m];
            }
        }
    }
};
struct Candidate {int id,state;};
long long branch_weight(int s,int h,int truth) {
    int t=next_state(s,truth);
    return t<0 ? 0 : fib[h+(t%2?0:1)];
}

template<class Ask>
vector<Candidate> solve(int n,const Endings& tail,const Parameters& params,Ask ask) {
    vector<Candidate> a;
    for(int i=1;i<=n;++i)a.push_back({i,4});
    auto query=[&](int l,int r) {
        int reply=ask(a[l].id,a[r].id);
        vector<Candidate> next;
        next.reserve(a.size());
        for(int i=0;i<int(a.size());++i) {
            auto c=a[i];c.state=next_state(c.state,((l<=i && i<=r)==bool(reply)));
            if(c.state>=0)next.push_back(c);
        }
        a.swap(next);
    };
    for(int h=params.rounds[n];h>0 && a.size()>2;--h) {
        long long delta=0;
        for(auto c:a)delta+=branch_weight(c.state,h,0)-branch_weight(c.state,h,1);
        long long best=llabs(delta);
        int cut=a.size(); // full and empty prefixes have opposite imbalance
        for(int i=0;i<int(a.size());++i){
            delta+=2*(branch_weight(a[i].state,h,1)-branch_weight(a[i].state,h,0));
            if(llabs(delta)<best){best=llabs(delta);cut=i+1;}
        }
        query(0,cut-1);
    }
    while(a.size()>2) {
        assert(a.size()<=small);
        int s=1;for(auto c:a)s=4*s+c.state;
        auto [l,r]=tail.choice[s];
        query(l,r);
    }
    return a;
}

int main(int argc,char** argv) {
    fib[1]=1;for(int i=2;i<int(fib.size());++i)fib[i]=fib[i-1]+fib[i-2];
    Endings tail;Parameters params(tail);
    if(argc==2 && string(argv[1])=="--profiles") {
        for(int m=1;m<=small;++m){
            vector<int> profile(m+1);
            int first=1<<(2*m);
            for(int s=first;s<2*first;++s){
                int k=s,u=0;
                for(int i=0;i<m;++i){u+=(k%2==0);k/=4;}
                profile[u]=max(profile[u],int(tail.cost[s]));
            }
            cout<<m;for(int x:profile)cout<<' '<<x;cout<<'\n';
        }
        return 0;
    }
    if(argc==2 && string(argv[1])=="--certificate") {
        cout<<"tail";for(int m=2;m<=small;++m)cout<<' '<<m<<':'<<tail.worst[m];cout<<'\n';
        int max_q=0,first=0;
        for(int n=3;n<=100000;++n) {
            if(params.total[n]>max_q){max_q=params.total[n];first=n;}
            assert(params.total[n]<=ceil(log(double(n))/log(1.116))-1);
        }
        cout<<"all 99998 sizes; maximum "<<max_q<<" first at "<<first
            <<"; n=100000 uses "<<params.rounds.back()<<" weighted probes\n";
        return 0;
    }
    {
        int n;cin>>n;
        auto candidates=solve(n,tail,params,[](int l,int r){
            cout<<"? "<<l<<' '<<r<<endl;
            int z;if(!(cin>>z)||z<0)exit(0);return r-l+1-z;
        });
        for(auto c:candidates){
            cout<<"! "<<c.id<<endl;
            int ans;if(!(cin>>ans)||ans<0) return 0;
            if(ans)break;
        }
        cout<<"#"<<endl;
    }
}

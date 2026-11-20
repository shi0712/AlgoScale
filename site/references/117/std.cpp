#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
using namespace std;
namespace {
int height(int capacity){
    int h=0;for(int p=1;p<capacity;p*=2)++h;return h;
}
// Pair winners are ordered first. Insert losers in reversed Jacobsthal groups.
template<class Less>
vector<int> merge_insertion(const vector<int>& input,Less& less){
    int n=input.size();
    if(n<=1)return input;
    vector<int> winners;
    map<int,int> mate;
    for(int j=0;j+1<n;j+=2){
        int a=input[j],b=input[j+1];if(less(b,a))swap(a,b);
        winners.push_back(b);mate[b]=a;
    }
    winners=merge_insertion(winners,less);
    vector<int> chain={mate[winners[0]]};
    chain.insert(chain.end(),winners.begin(),winners.end());
    int pending=(n+1)/2;
    for(int previous=1,bound=3;previous<pending;){
        for(int index=min(bound,pending);index>previous;--index){
            int loser;
            auto end=chain.end();
            if(index>int(winners.size()))loser=input.back();
            else{
                int winner=winners[index-1];loser=mate[winner];
                end=find(chain.begin(),chain.end(),winner);
            }
            auto where=lower_bound(chain.begin(),end,loser,less);
            chain.insert(where,loser);
        }
        int next=bound+2*previous;previous=bound;bound=next;
    }
    return chain;
}
struct Accounting{
    vector<int> sort_cost,rest;
    explicit Accounting(int limit):sort_cost(limit+1),rest(limit+1){
        for(int n=2;n<=limit;n++){
            int m=(n+1)/2;
            sort_cost[n]=n/2+sort_cost[n/2];
            for(int prev=1,h=3;prev<m;){
                int last=min(h,m);
                sort_cost[n]+=(last-prev)*height(last+prev);
                int next=h+2*prev;prev=h;h=next;
            }
            if(n%2)rest[n]=rest[n/2]+rest[n/2+1];
            else rest[n]=height(n-1)+rest[n-1];
        }
    }
    int bound(int n) const{return n-1+sort_cost[n]+rest[n];}
};
template<class Less,class Add>
int combine(vector<int> order,Less& less,Add& add){
    if(order.size()==1)return order[0];
    if(order.size()%2==0){
        int sum=add(order.front(),order.back());
        order.erase(order.begin());order.pop_back();
        order.insert(lower_bound(order.begin(),order.end(),sum,less),sum);
        if(order.size()==1)return sum;
    }
    // For odd size, the alternating sum is between the two extreme values.
    vector<int> odd,even;
    for(int i=0;i<int(order.size());i++)(i%2?even:odd).push_back(order[i]);
    int a=combine(odd,less,add),b=combine(even,less,add);
    return add(a,b);
}
template<class Less,class Add>
void run(int n,Less less,Add add){
    vector<int> order(n);iota(order.begin(),order.end(),1);
    combine(merge_insertion(order,less),less,add);
}
}
int main(int argc,char** argv){
    Accounting a(1000);
    if(argc==2 && string(argv[1])=="--certificate"){
        int worst=0,where=0;
        for(int n=2;n<=1000;n++)if(a.bound(n)>worst){worst=a.bound(n);where=n;}
        mt19937 rng(279);int tests=0;
        auto check=[&](vector<int> val,int R){
            int n=val.size()-1,sum=accumulate(val.begin(),val.end(),0),used=0;
            assert(abs(sum)<=R);vector<bool> alive(n+1,true);
            auto less=[&](int i,int j){assert(i!=j && alive[i] && alive[j]);++used;return val[i]<val[j];};
            auto add=[&](int i,int j){
                assert(i!=j && alive[i] && alive[j]);++used;alive[i]=alive[j]=false;
                int x=val[i]+val[j];assert(abs(x)<=R);val.push_back(x);alive.push_back(true);
                return int(val.size())-1;
            };
            run(n,less,add);assert(used<=a.bound(n) && val.back()==sum);++tests;
        };
        for(int n=2;n<=7;n++){
            vector<int> val(n+1,-2);val[0]=0;
            while(true){
                if(abs(accumulate(val.begin(),val.end(),0))<=2)check(val,2);
                int i=n;while(i && val[i]==2){val[i]=-2;--i;}
                if(!i)break;++val[i];
            }
        }
        for(int test=0;test<400;test++){
            int n=2+rng()%999;vector<int> v(n+1);
            for(int i=1;i<=n;i++)v[i]=int(rng()%201)-100;
            shuffle(v.begin()+1,v.end(),rng);
            int sum=accumulate(v.begin(),v.end(),0);
            check(v,max(100,abs(sum)));
        }
        cout<<"max "<<worst<<" at N="<<where<<", initial sort "<<a.sort_cost[where]
            <<", subsequent comparisons "<<a.rest[where]<<", legal arrays tested "<<tests<<"\n";
        return 0;
    }
    int n;if(!(cin>>n))return 0;
    auto call=[](char op,int a,int b){
        cout<<op<<' '<<a<<' '<<b<<endl;
        int x;if(!(cin>>x)||x<0)exit(0);return x;
    };
    run(n,[&](int i,int j){return call('?',i,j)!=0;},[&](int i,int j){return call('+',i,j);});
    cout<<"!"<<endl;
}

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>
using namespace std;
constexpr long long mod=1000000007;
struct Sample {int p,q,weight;};

void push(vector<Sample>& out,int p,int q,int w){
    int g=gcd(p,q);p/=g;q/=g;
    assert(q<=1000);
    out.push_back({p,q,w});
}

vector<Sample> quadrature(const vector<int>& x,const vector<int>& freq) {
    vector<Sample> out;
    auto slab=[&](int i){push(out,x[i]+x[i+1],2,2*(x[i+1]-x[i]));};
    int l=0;
    for(int r=1;r<int(x.size());++r) {
        if(r<int(x.size())-1 && freq[r]==1)continue;
        const int nodes=r-l+1;
        if(nodes<=3) {for(int i=l;i<r;++i)slab(i);l=r;continue;}
        auto weight=[&](int i){
            return x[min(r,i+1)]-x[max(l,i-1)];
        };
        for(int i=l;i<=r;) {
            if(nodes%2==1 && i==l+2) {
                push(out,x[i],1,weight(i));++i;continue;
            }
            int a=weight(i),b=weight(i+1),p=a*x[i]+b*x[i+1],q=a+b;
            if(q/gcd(p,q)<=1000)push(out,p,q,q);
            else {
                push(out,2*x[i]+x[i+1],3,2*a-b);
                push(out,x[i]+2*x[i+1],3,2*b-a);
            }
            i+=2;
        }
        l=r;
    }
    vector<Sample> simple;
    if(all_of(freq.begin(),freq.end(),[](int f){return f==1;})) {
        for(int i=1;i+1<int(x.size());++i)push(simple,x[i],1,x[i+1]-x[i-1]);
        vector<Sample> paired;
        for(int i=1;i+1<int(x.size());i+=2){
            int a=x[i+1]-x[i-1];
            if(i+2==int(x.size())){push(paired,x[i],1,a);break;}
            int b=x[i+2]-x[i],p=a*x[i]+b*x[i+1],q=a+b;
            if(q/gcd(p,q)<=1000)push(paired,p,q,q);
            else {push(paired,x[i],1,a);push(paired,x[i+1],1,b);}
        }
        if(paired.size()<simple.size())simple.swap(paired);
    } else {
        for(int i=0;i+1<int(x.size());++i)
            push(simple,x[i]+x[i+1],2,2*(x[i+1]-x[i]));
    }
    return out.size()<simple.size()?out:simple;
}
long long residue(const string& s){
    long long v=0;for(char c:s)v=(10*v+c-'0')%mod;return v;
}
long long inverse(long long v){
    long long z=1;for(int p=mod-2;p;p/=2,v=v*v%mod)if(p%2)z=z*v%mod;
    return z;
}
int main(int argc,char** argv){
    if(argc==2 && string(argv[1])=="--certificate"){
        int maximum=0;
        for(int n=3;n<=1000;++n)for(int d=0;2*d+2<=n;++d) {
            int s=n-2*d-2,p=d+1;
            int cost=min({n-2,s+p,p+min(s,(s+p)/2)+1});
            maximum=max(maximum,cost);
        }
        cout<<"universal quadrature bound "<<maximum<<"\n";return 0;
    }
    {
        int n;cin>>n;vector<int> px(n),py(n);map<int,int> mult;
        for(int i=0;i<n;++i){cin>>px[i]>>py[i];++mult[px[i]];}
        vector<pair<int,int>> pts,hull;
        for(int i=0;i<n;++i)pts.push_back({px[i],py[i]});
        sort(pts.begin(),pts.end());
        auto cross=[](auto a,auto b,auto c){
            return (b.first-a.first)*(c.second-a.second)-(b.second-a.second)*(c.first-a.first);
        };
        for(auto p:pts){while(hull.size()>=2 && cross(hull[hull.size()-2],hull.back(),p)<=0)hull.pop_back();hull.push_back(p);}
        int floor_size=hull.size();
        for(int i=n-2;i>=0;--i){while(int(hull.size())>floor_size && cross(hull[hull.size()-2],hull.back(),pts[i])<=0)hull.pop_back();hull.push_back(pts[i]);}
        long long twice=0;
        if(n==3)twice=abs((px[1]-px[0])*(py[2]-py[0])-(px[2]-px[0])*(py[1]-py[0]));
        else if(int(hull.size())==n+1) {
            for(int i=0;i<n;++i)twice+=1LL*hull[i].first*hull[i+1].second-1LL*hull[i].second*hull[i+1].first;
        } else {
            vector<int> x,freq;
            for(auto [v,f]:mult){x.push_back(v);freq.push_back(f);}
            for(auto s:quadrature(x,freq)){
                cout<<"? "<<s.p<<" "<<s.q<<endl;
                string a,b;if(!(cin>>a>>b)||a.front()=='-')return 0;
                long long h=residue(a)*inverse(residue(b))%mod;
                twice=(twice+s.weight*h)%mod;
            }
            twice=(twice+mod)%mod;
        }
        cout<<"! "<<(twice%2?twice:twice/2)<<' '<<(twice%2?2:1)<<endl;
    }
}

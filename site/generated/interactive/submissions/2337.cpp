#include <bits/stdc++.h>
using namespace std;

map<pair<int,int>,int> cacheQ;

int ask(int a,int b){
    auto key=make_pair(a,b);
    auto it=cacheQ.find(key);
    if(it!=cacheQ.end()) return it->second;
    printf("? %d %d\n",a,b);
    fflush(stdout);
    int r;
    if(scanf("%d",&r)!=1) exit(0);
    if(r==-1) exit(0);
    cacheQ[key]=r;
    return r;
}
int addop(int a,int b){
    printf("+ %d %d\n",a,b);
    fflush(stdout);
    int r;
    if(scanf("%d",&r)!=1) exit(0);
    if(r==-1) exit(0);
    return r;
}

vector<int> fj(const vector<int>& v){
    int n=v.size();
    if(n<=1) return v;
    int m=n/2;
    unordered_map<int,int> part;
    vector<int> bigs;
    for(int i=0;i<m;i++){
        int a=v[2*i],b=v[2*i+1];
        int big,small;
        if(ask(b,a)){big=a;small=b;} else {big=b;small=a;}
        part[big]=small;
        bigs.push_back(big);
    }
    vector<int> sb=fj(bigs);
    vector<int> chain;
    chain.push_back(part[sb[0]]);
    for(int x:sb) chain.push_back(x);
    int odd=n%2;
    int total=m+odd; // b indices 1..total, b_1 already inserted
    // b_j for j in 2..m: element part[sb[j-1]], bound sb[j-1]; b_{m+1} = v[n-1], bound none
    auto insertB=[&](int j){
        int elem,bound;
        if(j<=m){elem=part[sb[j-1]];bound=sb[j-1];}
        else {elem=v[n-1];bound=-1;}
        int hi;
        if(bound==-1) hi=chain.size();
        else hi=find(chain.begin(),chain.end(),bound)-chain.begin();
        int lo=0;
        while(lo<hi){
            int mid=(lo+hi)/2;
            if(ask(elem,chain[mid])) hi=mid; else lo=mid+1;
        }
        chain.insert(chain.begin()+lo,elem);
    };
    if(total>=2){
        long long lastEnd=1,jp=1,jc=3;
        while(true){
            long long en=min<long long>(jc,total);
            for(long long j=en;j>lastEnd;j--) insertB((int)j);
            lastEnd=jc;
            if(lastEnd>=total) break;
            long long nx=jc+2*jp;
            jp=jc;jc=nx;
        }
    }
    return chain;
}

int main(){
    int N;
    if(scanf("%d",&N)!=1) return 0;
    if(N==1){printf("!\n");fflush(stdout);return 0;}
    if(N==2){
        addop(1,2);
        printf("!\n");fflush(stdout);return 0;
    }
    vector<int> init(N);
    for(int i=0;i<N;i++) init[i]=i+1;
    vector<int> O=fj(init);
    int ol=0,orr=(int)O.size()-1; // inclusive range
    vector<int> E;
    int cnt=N;
    while(cnt>1){
        int osz=orr-ol+1; if(osz<0) osz=0;
        if(cnt==2){
            vector<int> rem;
            for(int i=ol;i<=orr;i++) rem.push_back(O[i]);
            for(int x:E) rem.push_back(x);
            addop(rem[0],rem[1]);
            cnt=1;
            break;
        }
        int mn,mx; bool mnO,mxO;
        if(osz==0){mn=E.front();mnO=false;mx=E.back();mxO=false;}
        else if(E.empty()){mn=O[ol];mnO=true;mx=O[orr];mxO=true;}
        else{
            if(ask(O[ol],E.front())){mn=O[ol];mnO=true;} else {mn=E.front();mnO=false;}
            if(ask(O[orr],E.back())){mx=E.back();mxO=false;} else {mx=O[orr];mxO=true;}
        }
        int x=addop(mn,mx);
        if(mnO) ol++; else E.erase(E.begin());
        if(mxO) orr--; else E.pop_back();
        int lo=0,hi=E.size();
        while(lo<hi){
            int mid=(lo+hi)/2;
            if(ask(x,E[mid])) hi=mid; else lo=mid+1;
        }
        E.insert(E.begin()+lo,x);
        cnt--;
    }
    printf("!\n");
    fflush(stdout);
    return 0;
}

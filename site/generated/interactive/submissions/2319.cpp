#include <bits/stdc++.h>
using namespace std;

int n;
int ask(int i,int j){
    printf("? %d %d\n",i,j);
    fflush(stdout);
    int r;
    if(scanf("%d",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    vector<int> a(n,0);
    for(int i=0;i<n-1;i++) a[i]=ask(i,0);
    int X=0; for(int i=0;i<n;i++) X^=i;
    int K=0; for(int i=0;i<n-1;i++) K^=a[i];
    mt19937_64 rng(1234567);
    vector<unsigned long long> mul(n+1);
    for(auto&x:mul) x=rng()|1;
    // classes
    unordered_map<unsigned long long,int> mp;
    vector<int> repV, cnt, lastA;
    vector<vector<uint16_t>> rows;
    vector<int> p(n), b(n);
    for(int v=0;v<n;v++){
        bool ok=true;
        for(int i=0;i<n-1;i++) if((a[i]^v)>=n){ok=false;break;}
        if(!ok) continue;
        int XS = X ^ ((n&1)?v:0);
        int m = XS ^ K;
        if((m^v)>=n) continue;
        vector<int> aa=a; aa[n-1]=m;
        // check distinct (should be)
        for(int i=0;i<n;i++) p[i]=aa[i]^v;
        if(p[v]!=0) continue;
        vector<char> seen(n,0);
        for(int i=0;i<n;i++){ if(seen[p[i]]){ok=false;break;} seen[p[i]]=1; b[p[i]]=i;}
        if(!ok) continue;
        unsigned long long h = (unsigned long long)(m+1)*mul[n];
        vector<uint16_t> row(n);
        for(int j=0;j<n;j++){ row[j]=(uint16_t)(p[0]^b[j]); h += (unsigned long long)(row[j]+1)*mul[j]; h ^= h>>29; }
        auto it=mp.find(h);
        if(it==mp.end()){
            mp[h]=repV.size();
            repV.push_back(v); cnt.push_back(1); lastA.push_back(m); rows.push_back(row);
        } else cnt[it->second]++;
    }
    int M=repV.size();
    vector<int> alive(M); iota(alive.begin(),alive.end(),0);
    bool askedLast = false;
    vector<int> freq(16384,0), stamp(16384,-1);
    int st=0;
    vector<char> askedJ(n,0);
    while(alive.size()>1){
        int bestQ=-2; int bestMax=INT_MAX;
        // query type: j in 1..n-1 -> (0,j); -1 -> (n-1,0)
        auto eval=[&](int q)->int{
            st++;
            int mx=0;
            for(int c:alive){
                int val = (q==-1)? lastA[c] : rows[c][q];
                if(stamp[val]!=st){stamp[val]=st;freq[val]=0;}
                freq[val]++;
                mx=max(mx,freq[val]);
            }
            return mx;
        };
        if(!askedLast && n>=2){
            int mx=eval(-1);
            if(mx<bestMax){bestMax=mx;bestQ=-1;}
        }
        for(int j=1;j<n;j++){
            if(askedJ[j]) continue;
            int mx=eval(j);
            if(mx<bestMax){bestMax=mx;bestQ=j;}
            if(bestMax==1) break;
        }
        if(bestQ==-2 || bestMax==(int)alive.size()) break; // shouldn't happen
        int r;
        vector<int> na;
        if(bestQ==-1){
            askedLast=true;
            r=ask(n-1,0);
            for(int c:alive) if(lastA[c]==r) na.push_back(c);
        } else {
            askedJ[bestQ]=1;
            r=ask(0,bestQ);
            for(int c:alive) if(rows[c][bestQ]==r) na.push_back(c);
        }
        alive=na;
    }
    if(alive.empty()) return 0;
    int c=alive[0];
    int v=repV[c];
    vector<int> aa=a; aa[n-1]=lastA[c];
    printf("!\n%d\n",cnt[c]);
    for(int i=0;i<n;i++) printf("%d%c",aa[i]^v,i+1<n?' ':'\n');
    fflush(stdout);
    return 0;
}

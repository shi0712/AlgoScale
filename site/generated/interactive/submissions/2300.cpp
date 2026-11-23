#include <bits/stdc++.h>
using namespace std;
using ll = long long;

static int n, m;
static mt19937_64 rng(0x9E3779B97F4A7C15ULL);

static int doQuery(int v, const vector<char>& b) {
    string s; s.reserve(m + 16);
    s += "? "; s += to_string(v); s += ' ';
    for (int i = 0; i < m; ++i) s.push_back(char('0' + b[i]));
    s.push_back('\n');
    fwrite(s.data(), 1, s.size(), stdout); fflush(stdout);
    int x;
    if (scanf("%d", &x) != 1) exit(0);
    if (x < 0) exit(0);
    return x;
}
struct Block { vector<int> A, B; };

int main() {
    if (scanf("%d", &n) != 1) return 0;
    m = n - 1;
    if (n == 2) {
        vector<char> b(1, 0);
        int ans = doQuery(1, b);
        if (ans == 1) printf("! 1 2\n"); else printf("! 2 1\n");
        fflush(stdout); return 0;
    }
    vector<int> samples;
    { set<int> ss; ss.insert(1); int want = (n <= 8 ? 1 : min(n, 6));
      if (want > 1) { ss.insert(n); ss.insert((n + 1) / 2); }
      while ((int)ss.size() < want) ss.insert(1 + (int)(rng() % (unsigned long long)n));
      for (int x : ss) samples.push_back(x); }
    vector<char> zero(m, 0);
    int r = 1, P = 0, bestT = INT_MAX;
    for (int v : samples) { int pv = doQuery(v, zero); int t = min(pv, m - pv);
        if (t < bestT) { bestT = t; r = v; P = pv; } }
    vector<char> p0(m, 0);
    { function<void(int,int,int)> rec = [&](int l, int rr, int cnt) {
            if (cnt <= 0) { for (int i=l;i<rr;i++) p0[i]=0; return; }
            if (cnt >= rr-l) { for (int i=l;i<rr;i++) p0[i]=1; return; }
            int mid=(l+rr)>>1; vector<char> bb(m,0);
            for (int i=l;i<mid;i++) bb[i]=1;
            int ans=doQuery(r,bb); int cl=((mid-l)+P-ans)/2;
            rec(l,mid,cl); rec(mid,rr,cnt-cl); };
      rec(0,m,P); }
    vector<int> dst(n+1,0);
    for (int v=1;v<=n;v++) if (v!=r) dst[v]=doQuery(v,p0);
    vector<int> order; for (int v=1;v<=n;v++) if (v!=r) order.push_back(v);
    sort(order.begin(),order.end(),[&](int x,int y){return dst[x]<dst[y];});
    vector<int> par(n+1,0), eid(n+1,-1); vector<char> used(m,0);
    vector<int> knownList; knownList.reserve(n); knownList.push_back(r);
    vector<int> a(n+1,0); vector<char> C(m,0), bestC(m,0), b(m,0);
    vector<ll> hist(m+3,0);
    for (int v : order) {
        int d = dst[v]; vector<Block> blocks;
        { Block init;
          for (int u : knownList) if (dst[u]==d-1) init.A.push_back(u);
          for (int i=0;i<m;i++) if (!used[i]) init.B.push_back(i);
          blocks.push_back(move(init)); }
        ll total=(ll)blocks[0].A.size()*(ll)blocks[0].B.size();
        while (total>1) {
            if (blocks.size()==1 && blocks[0].A.size()==1) {
                fill(C.begin(),C.end(),0); vector<int> tmp=blocks[0].B;
                int half=(int)tmp.size()/2;
                for (int k=0;k<half;k++){ int t2=k+(int)(rng()%(unsigned long long)(tmp.size()-k));
                    swap(tmp[k],tmp[t2]); C[tmp[k]]=1; }
                bestC=C;
            } else {
                double bestH=-1.0; double cap=log((double)(d+1));
                for (int t=0;t<16;t++){
                    fill(C.begin(),C.end(),0); int mode=t&3;
                    if (mode==0){ for(int i=0;i<m;i++) if(used[i]) C[i]=(char)(rng()&1ULL); }
                    else if (mode==1){ for(int i=0;i<m;i++) if(used[i]) C[i]=(char)((rng()%100)<35); }
                    else if (mode==2){ int z=knownList[rng()%knownList.size()]; while(z!=r){C[eid[z]]=1;z=par[z];} }
                    else { int z=knownList[rng()%knownList.size()]; while(z!=r){C[eid[z]]^=1;z=par[z];}
                           z=knownList[rng()%knownList.size()]; while(z!=r){C[eid[z]]^=1;z=par[z];} }
                    for (auto&blk:blocks) for(int j:blk.B) C[j]=(char)(rng()&1ULL);
                    a[r]=0;
                    for (size_t k=1;k<knownList.size();k++){int u=knownList[k]; a[u]=a[par[u]]+C[eid[u]];}
                    fill(hist.begin(),hist.begin()+d+2,0);
                    for (auto&blk:blocks){ ll c0=0,c1=0;
                        for(int j:blk.B){ if(C[j]) c1++; else c0++; }
                        for(int u:blk.A){ hist[a[u]]+=c0; hist[a[u]+1]+=c1; } }
                    double H=0;
                    for(int x=0;x<=d+1;x++) if(hist[x]>0){ double pp=(double)hist[x]/(double)total; H-=pp*log(pp); }
                    if(H>bestH){bestH=H;bestC=C;}
                    if(bestH>=cap-0.01) break;
                }
                if(bestH<=1e-12){
                    fill(C.begin(),C.end(),0); bool ok=false;
                    for(auto&blk:blocks) if(blk.B.size()>=2){C[blk.B[0]]=1;ok=true;break;}
                    if(!ok) for(auto&blk:blocks) if(blk.A.size()>=2){int z=blk.A[0]; while(z!=r){C[eid[z]]=1;z=par[z];} ok=true; break;}
                    bestC=C;
                }
            }
            C=bestC; a[r]=0;
            for (size_t k=1;k<knownList.size();k++){int u=knownList[k]; a[u]=a[par[u]]+C[eid[u]];}
            int wC=0;
            for(int i=0;i<m;i++){ b[i]=(char)(p0[i]^C[i]); wC+=C[i]; }
            int ans=doQuery(v,b);
            int X=(wC+d-ans)/2;
            vector<Block> nb;
            for(auto&blk:blocks){
                Block b0,b1;
                for(int u:blk.A){ if(a[u]==X) b0.A.push_back(u); else if(a[u]==X-1) b1.A.push_back(u); }
                for(int j:blk.B){ if(C[j]) b1.B.push_back(j); else b0.B.push_back(j); }
                if(!b0.A.empty()&&!b0.B.empty()) nb.push_back(move(b0));
                if(!b1.A.empty()&&!b1.B.empty()) nb.push_back(move(b1));
            }
            blocks=move(nb); total=0;
            for(auto&blk:blocks) total+=(ll)blk.A.size()*(ll)blk.B.size();
            if(total<=0) return 0;
        }
        int pu=-1,pj=-1;
        for(auto&blk:blocks) if((ll)blk.A.size()*(ll)blk.B.size()==1){pu=blk.A[0];pj=blk.B[0];}
        if(pu<0||pj<0) return 0;
        par[v]=pu; eid[v]=pj; used[pj]=1; knownList.push_back(v);
    }
    vector<int> S(m,0),F(m,0);
    for(int v=1;v<=n;v++) if(v!=r){ int i=eid[v];
        if(p0[i]){S[i]=par[v];F[i]=v;} else {S[i]=v;F[i]=par[v];} }
    string out="!";
    for(int i=0;i<m;i++){ out+=' '; out+=to_string(S[i]); out+=' '; out+=to_string(F[i]); }
    out+='\n';
    fwrite(out.data(),1,out.size(),stdout); fflush(stdout);
    return 0;
}

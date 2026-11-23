#include <bits/stdc++.h>
using namespace std;

static long long ask(const vector<int>& s){
    string buf; buf.reserve(s.size()*4+16);
    buf += "? "; buf += to_string((int)s.size()); buf += '\n';
    for(size_t i=0;i<s.size();++i){ buf += to_string(s[i]); buf += (i+1==s.size())?'\n':' '; }
    fwrite(buf.data(),1,buf.size(),stdout); fflush(stdout);
    long long r;
    if(scanf("%lld",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}
static void emit(const char* tag,const vector<int>& v){
    string buf=string(tag)+" "+to_string((int)v.size())+"\n";
    for(size_t i=0;i<v.size();++i){ buf+=to_string(v[i]); buf+=(i+1==v.size())?'\n':' '; }
    fwrite(buf.data(),1,buf.size(),stdout); fflush(stdout);
}
int main(){
    int n; if(scanf("%d",&n)!=1) return 0;
    if(n<=2){ vector<int> one{1}; emit("Y",one); return 0; }
    vector<long long> e(n+1,0); vector<int> par(n+1,0);
    vector<int> s; s.reserve(n+2);
    for(int v=2;v<=n;v++){
        s.clear(); for(int i=1;i<=v;i++) s.push_back(i);
        long long q=ask(s); e[v]=q;
        int d=(int)(q-e[v-1]); if(d==0) continue;
        int lo=1,hi=v-1,dlo=0,dhi=d; bool first=true;
        while(lo<hi){
            if(dhi-dlo==hi-lo+1) break;
            int mid;
            if(first){ int len=hi-lo+1,D=dhi-dlo; int off=len/(D+1); mid=lo+off;
                if(mid>hi-1) mid=hi-1; if(mid<lo) mid=lo; first=false; }
            else mid=lo+(hi-lo)/2;
            s.clear(); for(int i=1;i<=mid;i++) s.push_back(i); s.push_back(v);
            long long qq=ask(s); int dm=(int)(qq-e[mid]);
            if(dm>dlo){hi=mid;dhi=dm;} else {lo=mid+1;dlo=dm;}
        }
        par[v]=lo;
    }
    vector<vector<int>> adj(n+1);
    for(int v=2;v<=n;v++) if(par[v]>=1){ adj[v].push_back(par[v]); adj[par[v]].push_back(v); }
    vector<int> comp(n+1),col(n+1),prt(n+1),A,B,stk;
    while(true){
        fill(comp.begin(),comp.end(),-1); int c=0;
        for(int r=1;r<=n;r++) if(comp[r]<0){
            comp[r]=c; col[r]=0; prt[r]=0; stk.clear(); stk.push_back(r);
            while(!stk.empty()){ int u=stk.back(); stk.pop_back();
                for(int w:adj[u]) if(comp[w]<0){ comp[w]=c; col[w]=col[u]^1; prt[w]=u; stk.push_back(w);} }
            ++c;
        }
        A.clear(); B.clear();
        for(int v=1;v<=n;v++) (col[v]?B:A).push_back(v);
        long long ea=(A.size()>=2)?ask(A):0;
        const vector<int>* Sp; long long cnt;
        if(ea>0){ Sp=&A; cnt=ea; }
        else{
            long long eb=(B.size()>=2)?ask(B):0;
            if(eb==0){ emit("Y",A); return 0; }
            Sp=&B; cnt=eb;
        }
        const vector<int>& S=*Sp; int m=(int)S.size();
        int lo=2,hi=m; long long ehi=cnt; bool first=true;
        while(lo<hi){
            int mid;
            if(first){ int g=(int)(m/sqrt((double)cnt)); if(g<lo)g=lo; if(g>hi-1)g=hi-1; mid=g; first=false; }
            else mid=lo+(hi-lo)/2;
            vector<int> t(S.begin(),S.begin()+mid);
            long long q=ask(t);
            if(q>0){hi=mid;ehi=q;} else lo=mid+1;
        }
        int j=lo, x=S[j-1], dx=(int)ehi;
        int lo2=1,hi2=j-1,dlo=0,dhi=dx; first=true;
        while(lo2<hi2){
            if(dhi-dlo==hi2-lo2+1) break;
            int mid;
            if(first){ int len=hi2-lo2+1,D=dhi-dlo; int off=len/(D+1); mid=lo2+off;
                if(mid>hi2-1) mid=hi2-1; if(mid<lo2) mid=lo2; first=false; }
            else mid=lo2+(hi2-lo2)/2;
            vector<int> t(S.begin(),S.begin()+mid); t.push_back(x);
            long long q=ask(t); int dm=(int)q;
            if(dm>dlo){hi2=mid;dhi=dm;} else {lo2=mid+1;dlo=dm;}
        }
        int y=S[lo2-1];
        if(comp[x]==comp[y]){
            vector<int> ax,ay;
            for(int u=x;u;u=prt[u]) ax.push_back(u);
            for(int u=y;u;u=prt[u]) ay.push_back(u);
            vector<int> pos(n+1,-1);
            for(int i=0;i<(int)ax.size();++i) pos[ax[i]]=i;
            int jj=0; while(pos[ay[jj]]<0) ++jj;
            int ii=pos[ay[jj]];
            vector<int> cyc;
            for(int t=0;t<=ii;++t) cyc.push_back(ax[t]);
            for(int t=jj-1;t>=0;--t) cyc.push_back(ay[t]);
            emit("N",cyc); return 0;
        }
        adj[x].push_back(y); adj[y].push_back(x);
    }
}

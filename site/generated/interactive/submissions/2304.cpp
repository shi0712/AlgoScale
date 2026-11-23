#include <bits/stdc++.h>
using namespace std;

static int n, K;
static vector<int> ansv;
static unordered_map<string,int> cache_;
static vector<int> capv;

static int ask(const vector<int>& S){
    vector<char> in(n,0);
    for(int p:S) in[p]=1;
    bool inK=true,outK=true; int si=0,so=0;
    for(int i=0;i<n;i++){
        if(in[i]){ if(ansv[i]<0) inK=false; else si+=ansv[i]; }
        else { if(ansv[i]<0) outK=false; else so+=ansv[i]; }
    }
    if(inK) return si;
    if(outK) return K-so;
    string s(n,'T');
    for(int p:S) s[p]='F';
    auto it=cache_.find(s);
    if(it!=cache_.end()) return it->second;
    cout<<s<<'\n'; cout.flush();
    int sc;
    if(!(cin>>sc)) exit(0);
    if(sc<0) exit(0);
    if(sc>=n) exit(0);
    int val=(K+(int)S.size()-sc)/2;
    cache_.emplace(s,val);
    return val;
}
struct Plan{ int r=0,type=0,last=-1; vector<int> cols,W,extra;
    Plan *A=nullptr,*B=nullptr,*sub=nullptr; vector<vector<int>> rows,rowIdx; bool gen=false; };
static Plan* build(const vector<int>& cols,int r){
    Plan* p=new Plan(); p->r=r; p->cols=cols; int m=(int)cols.size();
    if(r<=3){ p->type=0; return p; }
    int rs=(r-1)/2, sp=r-2*rs-1, capA=2*capv[rs]+rs+sp;
    if(m<=capA){ p->type=1; int nW=min(rs,m),rem=m-nW;
        int nU=min(capv[rs],(rem+1)/2), nV=min(capv[rs],rem-nU), lo=rem-nU-nV; int idx=0;
        vector<int> U(cols.begin(),cols.begin()+nU); idx+=nU;
        vector<int> V(cols.begin()+idx,cols.begin()+idx+nV); idx+=nV;
        for(int t=0;t<nW;t++) p->W.push_back(cols[idx++]);
        for(int t=0;t<lo;t++) p->extra.push_back(cols[idx++]);
        p->A=build(U,rs); p->B=build(V,rs);
    } else { p->type=2; vector<int> sb(cols.begin(),cols.end()-1);
        p->sub=build(sb,r-1); p->last=cols.back(); }
    return p;
}
static void genRows(Plan* p){
    if(p->gen) return; p->gen=true; p->rows.assign(p->r,{});
    if(p->type==0){ int m=(int)p->cols.size(); p->rowIdx.assign(p->r,{});
        if(p->r==1){ if(m>=1) p->rowIdx[0].push_back(0); }
        else if(p->r==2){ for(int i=0;i<min(2,m);i++) p->rowIdx[i].push_back(i); }
        else { static const int id[3][3]={{0,1,3},{0,2,3},{1,2,3}};
            for(int j=0;j<3&&j<p->r;j++) for(int t=0;t<3;t++) if(id[j][t]<m) p->rowIdx[j].push_back(id[j][t]); }
        for(int j=0;j<p->r;j++) for(int i:p->rowIdx[j]) p->rows[j].push_back(p->cols[i]);
    } else if(p->type==2){ genRows(p->sub);
        for(int i=0;i<p->r-1;i++) p->rows[i]=p->sub->rows[i];
        p->rows[p->r-1].push_back(p->last);
    } else { genRows(p->A); genRows(p->B); int rs=(p->r-1)/2;
        p->rows[0]=p->B->cols; vector<char> mark(n,0);
        for(int j=0;j<rs;j++){
            vector<int> r1=p->A->rows[j];
            r1.insert(r1.end(),p->B->rows[j].begin(),p->B->rows[j].end());
            if(j<(int)p->W.size()) r1.push_back(p->W[j]);
            p->rows[1+2*j]=move(r1);
            for(int q:p->B->rows[j]) mark[q]=1;
            vector<int> r2=p->A->rows[j];
            for(int q:p->B->cols) if(!mark[q]) r2.push_back(q);
            for(int q:p->B->rows[j]) mark[q]=0;
            p->rows[2+2*j]=move(r2);
        }
        int base=1+2*rs;
        for(int t=0;t<(int)p->extra.size();t++) if(base+t<p->r) p->rows[base+t].push_back(p->extra[t]);
    }
}
static void decode(Plan* p,const vector<int>& res){
    if(p->type==0){ int m=(int)p->cols.size();
        for(int mask=0;mask<(1<<m);mask++){ bool ok=true;
            for(int j=0;j<p->r&&ok;j++){ int s=0; for(int i:p->rowIdx[j]) s+=(mask>>i)&1; if(s!=res[j]) ok=false; }
            if(ok){ for(int i=0;i<m;i++) ansv[p->cols[i]]=(mask>>i)&1; return; } }
    } else if(p->type==2){ vector<int> sb(res.begin(),res.begin()+(p->r-1));
        decode(p->sub,sb); ansv[p->last]=res[p->r-1];
    } else { int rs=(p->r-1)/2, v=res[0]; vector<int> a(rs),b(rs);
        for(int j=0;j<rs;j++){ int y1=res[1+2*j],y2=res[2+2*j]; int t=y1+y2-v;
            if(t<0) t=0; a[j]=t/2; int w=t%2; b[j]=y1-a[j]-w;
            if(j<(int)p->W.size()) ansv[p->W[j]]=w; }
        decode(p->A,a); decode(p->B,b); int base=1+2*rs;
        for(int t=0;t<(int)p->extra.size();t++) if(base+t<p->r) ansv[p->extra[t]]=res[base+t];
    }
}
static void runPlan(Plan* p){ genRows(p); vector<int> res(p->r);
    for(int i=0;i<p->r;i++) res[i]=ask(p->rows[i]); decode(p,res); }
static void initCap(int mx){ capv.assign(mx+1,0);
    for(int r=1;r<=mx;r++){ int best;
        if(r==1) best=1; else if(r==2) best=2; else if(r==3) best=4;
        else { int rs=(r-1)/2, sp=r-2*rs-1; best=max(2*capv[rs]+rs+sp,capv[r-1]+1); }
        capv[r]=best; } }
static int minRows(int m){ for(int r=1;r<(int)capv.size();r++) if(capv[r]>=m) return r; return (int)capv.size()-1; }
static void solveBlock(const vector<int>& pos,int cnt){
    int sz=(int)pos.size(); if(sz==0) return;
    if(cnt==0){ for(int p:pos) ansv[p]=0; return; }
    if(cnt==sz){ for(int p:pos) ansv[p]=1; return; }
    if(sz==1){ ansv[pos[0]]=cnt; return; }
    if(sz==2){ int b=ask(vector<int>{pos[0]}); ansv[pos[0]]=b; ansv[pos[1]]=cnt-b; return; }
    int c=min(cnt,sz-cnt), R=minRows(sz);
    double est=(double)c*(log2((double)sz/(double)c)+2.0);
    if(sz<=4||est<(double)R){
        int mid=sz/2;
        vector<int> L(pos.begin(),pos.begin()+mid), Rt(pos.begin()+mid,pos.end());
        int cl=ask(L); solveBlock(L,cl); solveBlock(Rt,cnt-cl);
    } else { Plan* p=build(pos,R); runPlan(p); }
}
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if(!(cin>>n)) return 0;
    initCap(3000); ansv.assign(n,-1);
    string s(n,'T'); cout<<s<<'\n'; cout.flush();
    int sc; if(!(cin>>sc)) return 0;
    if(sc<0) return 0; if(sc>=n) return 0;
    K=sc; cache_.emplace(s,0);
    if(K==0){ string t(n,'F'); cout<<t<<'\n'; cout.flush(); int z; if(!(cin>>z)) return 0; return 0; }
    vector<int> all(n); for(int i=0;i<n;i++) all[i]=i;
    solveBlock(all,K);
    string fin(n,'F'); for(int i=0;i<n;i++) if(ansv[i]==1) fin[i]='T';
    cout<<fin<<'\n'; cout.flush();
    int z; if(!(cin>>z)) return 0;
    return 0;
}

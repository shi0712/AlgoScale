#include <bits/stdc++.h>
using namespace std;

static int pw3[10];

struct Hyp{ int code; unsigned char kind,len,init,last; double w; };

static void build(vector<Hyp>& hs){
    hs.clear(); hs.reserve(240000);
    for(int len=1;len<=3;len++){
        for(int c=0;c<pw3[len];c++){
            int d[3]; int x=c; bool same=true;
            for(int i=0;i<len;i++){ d[i]=x%3; x/=3; }
            for(int i=1;i<len;i++) if(d[i]!=d[0]) same=false;
            if(len>1&&same) continue;
            double w;
            if(len==1) w=1.0;
            else if(len==2) w=0.02;
            else{ bool diff=(d[0]!=d[1]&&d[1]!=d[2]&&d[0]!=d[2]); w=diff?(d[0]==0?1.0:0.25):0.02; }
            Hyp h; h.code=c;h.kind=0;h.len=(unsigned char)len;h.init=0;h.last=0;h.w=w; hs.push_back(h);
        }
    }
    for(int c=0;c<19683;c++){
        int g[9]; int x=c;
        for(int i=0;i<9;i++){ g[i]=x%3; x/=3; }
        bool ox=true,oy=true,cp=true,bt=true,ls=true;
        for(int a=0;a<3;a++)for(int b=0;b<3;b++){
            int v=g[a*3+b];
            if(v!=g[a*3])ox=false; if(v!=g[b])oy=false;
            if(v!=a)cp=false; if(v!=(a+1)%3)bt=false; if(v!=(a+2)%3)ls=false;
        }
        double base=1e-6+(ox?0.004:0.0)+(oy?0.002:0.0);
        for(int it=0;it<3;it++){
            double w=base;
            if(cp)w+=(it==0?1.0:0.25);
            if(bt)w+=(it==0?1.0:0.25);
            if(ls)w+=(it==0?0.5:0.15);
            Hyp h;h.code=c;h.kind=1;h.len=0;h.init=(unsigned char)it;h.last=0;h.w=w;hs.push_back(h);
        }
    }
    for(int c=0;c<19683;c++){
        int g[9]; int x=c;
        for(int i=0;i<9;i++){ g[i]=x%3; x/=3; }
        bool oa=true,ob=true;
        for(int a=0;a<3;a++)for(int b=0;b<3;b++){
            if(g[a*3+b]!=g[a*3])oa=false;
            if(g[a*3+b]!=g[b])ob=false;
        }
        double base=1e-7+(oa?1e-5:0.0)+(ob?3e-5:0.0);
        for(int it=0;it<9;it++){
            Hyp h;h.code=c;h.kind=2;h.len=0;h.init=(unsigned char)it;h.last=0;h.w=base;hs.push_back(h);
        }
    }
}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    pw3[0]=1; for(int i=1;i<=9;i++)pw3[i]=pw3[i-1]*3;
    vector<Hyp> hs; build(hs);
    vector<int> pred(hs.size(),0);
    vector<int> my;
    int n=(int)hs.size(), t=0, streak=0, rounds=0;
    while(rounds<200000){
        if(n==0){ build(hs); pred.assign(hs.size(),0); n=(int)hs.size(); my.clear(); t=0; streak=0; }
        double W[3]={0,0,0};
        for(int i=0;i<n;i++){
            const Hyp&h=hs[i]; int p;
            if(h.kind==0) p=(h.code/pw3[t%h.len])%3;
            else if(h.kind==1) p=(t==0)?(int)h.init:(h.code/pw3[my[t-1]*3+h.last])%3;
            else p=(t==0)?(h.init/3):((t==1)?(h.init%3):(h.code/pw3[my[t-1]*3+my[t-2]])%3);
            pred[i]=p; W[p]+=h.w;
        }
        double mx=max(W[0],max(W[1],W[2]));
        int v=0;
        if(mx>0){
            double thr=mx-mx*1e-4-1e-15;
            int cand[3],nc=0;
            for(int c=0;c<3;c++) if(W[c]>=thr) cand[nc++]=c;
            v=cand[0];
            if(nc>1){
                double best=-1; int myPrev=(t>=1?my[t-1]:0);
                for(int k=0;k<nc;k++){
                    int c=cand[k], m=(c+1)%3;
                    double WA[3]={0,0,0},WB[3]={0,0,0};
                    for(int i=0;i<n;i++){
                        const Hyp&h=hs[i]; int q;
                        if(h.kind==0) q=(h.code/pw3[(t+1)%h.len])%3;
                        else if(h.kind==1) q=(h.code/pw3[m*3+pred[i]])%3;
                        else q=(t==0)?(h.init%3):(h.code/pw3[m*3+myPrev])%3;
                        if(pred[i]==c) WA[q]+=h.w; else WB[q]+=h.w;
                    }
                    double sa=WA[0]+WA[1]+WA[2], sb=WB[0]+WB[1]+WB[2];
                    double ca=(sa>0)?max(WA[0],max(WA[1],WA[2]))/sa:1.0;
                    double cb=(sb>0)?max(WB[0],max(WB[1],WB[2]))/sb:1.0;
                    double sc=ca+cb;
                    if(sc>best+1e-12){ best=sc; v=c; }
                }
            }
        }
        int m=(v+1)%3;
        cout<<(char)("RPS"[m])<<endl;
        rounds++;
        string s;
        if(!(cin>>s)) return 0;
        if(s=="-1") return 0;
        bool win=(s=="player");
        int nn=0;
        for(int i=0;i<n;i++){
            if((pred[i]==v)==win){
                int pv=pred[i];
                if(nn!=i){ hs[nn]=hs[i]; pred[nn]=pv; }
                hs[nn].last=(unsigned char)pv;
                nn++;
            }
        }
        n=nn;
        my.push_back(m);
        t++;
        streak=win?streak+1:0;
        if(streak>=10) return 0;
    }
    return 0;
}

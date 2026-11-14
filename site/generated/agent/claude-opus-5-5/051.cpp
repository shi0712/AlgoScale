#include <bits/stdc++.h>
using namespace std;
typedef unsigned int u32;
const u32 MOD=1000000007;
static inline u32 ad(u32 a,u32 b){u32 c=a+b;return c>=MOD?c-MOD:c;}
int main(){
    int n,Q;scanf("%d %d",&n,&Q);
    static char buf[2005];scanf("%s",buf);string s(buf);
    int AM=n/2+2; // max A index
    int OFF=AM+2, W=2*OFF+1;
    // dp[l][A][D+OFF]
    vector<u32> cur(3*(size_t)(AM+1)*W,0), nxt(cur.size(),0);
    auto idx=[&](int l,int A,int D)->size_t{return ((size_t)l*(AM+1)+A)*W+D+OFF;};
    // position 0
    for(int l=0;l<3;l++){
        if(s[0]!='?'&&s[0]-'a'!=l)continue;
        int A=(l==0),D=(l==1)-(l==2);
        cur[idx(l,A,D)]=1;
    }
    for(int i=1;i<n;i++){
        int Amax=min((i+2)/2,AM);
        bool al[3];for(int l=0;l<3;l++)al[l]=(s[i]=='?'||s[i]-'a'==l);
        for(int A=0;A<=Amax;A++){
            int Dl=-(A+1),Dh=A+1;
            u32* na=&nxt[idx(0,A,0)];u32* nb=&nxt[idx(1,A,0)];u32* nc=&nxt[idx(2,A,0)];
            const u32* oa=&cur[idx(0,A,0)];const u32* ob=&cur[idx(1,A,0)];const u32* oc=&cur[idx(2,A,0)];
            if(al[0]&&A>0){
                const u32* pb=&cur[idx(1,A-1,0)];const u32* pc=&cur[idx(2,A-1,0)];
                for(int D=Dl;D<=Dh;D++)na[D]=ad(pb[D],pc[D]);
            } else for(int D=Dl;D<=Dh;D++)na[D]=0;
            if(al[1]) for(int D=Dl;D<=Dh;D++)nb[D]=ad(oa[D-1],oc[D-1]);
            else for(int D=Dl;D<=Dh;D++)nb[D]=0;
            if(al[2]) for(int D=Dl;D<=Dh;D++)nc[D]=ad(oa[D+1],ob[D+1]);
            else for(int D=Dl;D<=Dh;D++)nc[D]=0;
        }
        swap(cur,nxt);
    }
    int fa=0,fb=0,T=0;
    for(char ch:s){if(ch=='a')fa++;else if(ch=='b')fb++;else if(ch=='?')T++;}
    const int N=1001;
    vector<vector<u32>> cnt(N,vector<u32>(N,0));
    for(int A=0;A<=AM;A++)for(int D=-OFF;D<=OFF;D++){
        u32 v=0;for(int l=0;l<3;l++){size_t k=idx(l,A,D);v=ad(v,cur[k]);}
        if(!v)continue;
        int nonA=n-A; if((nonA+D)&1)continue;
        int B=(nonA+D)/2;
        int an=A-fa,bn=B-fb;
        if(an<0||bn<0||an>=N||bn>=N)continue;
        cnt[an][bn]=ad(cnt[an][bn],v);
    }
    // rowpref
    vector<vector<u32>> rp(N,vector<u32>(N)),P(N,vector<u32>(N)),Dg(N,vector<u32>(N));
    for(int A=0;A<N;A++){u32 r=0;for(int B=0;B<N;B++){r=ad(r,cnt[A][B]);rp[A][B]=r;}}
    for(int B=0;B<N;B++){u32 r=0;for(int A=0;A<N;A++){r=ad(r,rp[A][B]);P[A][B]=r;}}
    for(int sm=0;sm<N;sm++){u32 r=0;for(int A=0;A<N;A++){if(A<=sm)r=ad(r,rp[A][sm-A]);Dg[sm][A]=r;}}
    auto tri=[&](int x,int y,int sm)->u32{
        if(sm<0)return 0;
        int xp=min(x,sm);
        int m=min(xp,sm-y);
        u32 res=0;
        if(m>=0)res=P[m][y];
        int lo=max(m,-1);
        if(xp>lo){u32 t=Dg[sm][xp]; if(lo>=0)t=ad(t,MOD-Dg[sm][lo]); res=ad(res,t);}
        return res;
    };
    string out;
    while(Q--){
        int x,y,z;scanf("%d %d %d",&x,&y,&z);
        u32 r=P[x][y];
        r=ad(r,MOD-tri(x,y,T-z-1));
        out+=to_string(r);out+='\n';
    }
    fputs(out.c_str(),stdout);
}

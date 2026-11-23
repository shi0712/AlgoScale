#include <bits/stdc++.h>
using namespace std;

int n;
long long askq(const vector<pair<int,int>>& items){
    vector<int> lst;
    for(auto&p:items) for(int j=0;j<p.second;j++) lst.push_back(p.first);
    printf("? 1 %d",(int)lst.size());
    for(int x:lst) printf(" %d",x);
    printf("\n"); fflush(stdout);
    long long r;
    if(scanf("%lld",&r)!=1) exit(0);
    return r;
}
void tog(int u){ printf("? 2 %d\n",u); fflush(stdout); }

int G;
vector<int> val;

void queryChunkExact(const vector<int>& ch,int o){
    int sz=ch.size();
    vector<pair<int,int>> it;
    for(int i=0;i<sz;i++) it.push_back({ch[i],1<<i});
    long long S=askq(it);
    long long W=(1LL<<sz)-1;
    long long B=(S-o*W+W)/2;
    for(int i=0;i<sz;i++) val[ch[i]]=((B>>i)&1)?1:-1;
}
void processExact(const vector<int>& pool,int o){
    for(size_t i=0;i<pool.size();i+=G){
        vector<int> ch(pool.begin()+i,pool.begin()+min(pool.size(),i+G));
        queryChunkExact(ch,o);
    }
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    for(int i=0;i<n-1;i++){int a,b;scanf("%d %d",&a,&b);}
    val.assign(n+1,0);
    vector<int> L; for(int i=2;i<=n;i++) L.push_back(i);
    int N=L.size();
    int g0=0; while(g0+1<=N && (1<<(g0+2))-1<=n) g0++;
    G=0; while((1<<(G+1))-1<=n) G++;
    vector<pair<int,int>> it; it.push_back({1,1});
    for(int i=0;i<g0;i++) it.push_back({L[i],1<<(i+1)});
    long long S=askq(it);
    long long W=(1LL<<(g0+1))-2;
    int d=99; long long Bf=-1;
    for(int dd=-2;dd<=2&&d==99;dd++){
        for(long long B=0;B<(1LL<<g0);B++){
            if(dd*(W+1)+4*B-W==S){d=dd;Bf=B;break;}
        }
    }
    for(int i=0;i<g0;i++) val[L[i]]=((Bf>>i)&1)?1:-1;
    vector<int> rest(L.begin()+g0,L.end());
    int a1=0;
    if(d==1||d==-1){
        a1=d;
        processExact(rest,d);
    } else if(d==2||d==-2){
        if(N==1){ val[2]=d/2; a1=d/2; }
        else{
            tog(1); a1=-d/2;
            vector<int> pool;
            for(int i=0;i<g0;i++) if(val[L[i]]==-d/2) pool.push_back(L[i]);
            for(int x:rest) pool.push_back(x);
            processExact(pool,0);
        }
    } else {
        if(N==1){
            processExact({2},0);
            a1=-val[2];
        } else if(rest.empty()){
            bool uni=true; for(int x:L) if(val[x]!=val[L[0]]) uni=false;
            if(uni) a1=-val[L[0]];
            else{
                tog(1);
                long long r=askq({{1,1}});
                a1=(int)(r/2);
            }
        } else {
            vector<vector<int>> chunks;
            for(size_t i=0;i<rest.size();i+=G)
                chunks.push_back(vector<int>(rest.begin()+i,rest.begin()+min(rest.size(),i+G)));
            for(size_t c=0;c+1<chunks.size();c++) queryChunkExact(chunks[c],0);
            vector<int>& Lc=chunks.back();
            bool anyUni=true;
            {
                // check if all known leaves same value (can't shortcut since last chunk unknown) - skip
                (void)anyUni;
            }
            tog(1);
            int sz=Lc.size();
            vector<pair<int,int>> it2;
            for(int i=0;i<sz;i++) it2.push_back({Lc[i],1<<i});
            long long S2=askq(it2);
            long long W2=(1LL<<sz)-1;
            int s=0; long long Bs=-1;
            for(int ss=-1;ss<=1&&s==0;ss+=2){
                long long t=S2-2*ss*W2+W2;
                if(t>=0 && t%2==0 && t/2<(1LL<<sz)){s=ss;Bs=t/2;}
            }
            a1=s;
            set<int> inLc(Lc.begin(),Lc.end());
            vector<int> C;
            for(int i=0;i<sz;i++){
                int c=((Bs>>i)&1)?1:-1;
                val[Lc[i]]=c;
                if(c==-s) C.push_back(Lc[i]);
            }
            bool K=false;
            for(int x:L) if(!inLc.count(x) && val[x]==s) K=true;
            if(!C.empty()){
                if(!K && C.size()==1){ val[C[0]]=s; }
                else{
                    tog(1); a1=-s;
                    processExact(C,0);
                }
            }
        }
    }
    val[1]=a1;
    printf("!");
    for(int i=1;i<=n;i++) printf(" %d",val[i]);
    printf("\n"); fflush(stdout);
    return 0;
}

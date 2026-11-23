#include <bits/stdc++.h>
using namespace std;
int n; int val[5005]; vector<int> reps[5]; int cnt[5];
int Fi[5][5][5];
vector<int> G; vector<uint32_t> C; int nxt=1;
double p[5];
struct Opt{ int u; int kv[2]; int nk; int sl[3]; int j; double H;};
int main(){
    map<int,int> mp;
    for(int a=1;a<=4;a++)for(int b=1;b<=4;b++)for(int c=1;c<=4;c++){
        int f1=a+b+c,f2=-a+b+c,f3=a-b+c,f4=a+b-c;
        int v=0; if(f2>0&&f3>0&&f4>0) v=f1*f2*f3*f4;
        if(!mp.count(v)){int s=mp.size(); mp[v]=s;}
        Fi[a][b][c]=mp[v];
    }
    if(!(cin>>n)) return 0;
    C.push_back(0);
    while(true){
        if(G.empty() && nxt>n){
            printf("!");
            for(int i=1;i<=n;i++) printf(" %d",val[i]);
            printf("\n"); fflush(stdout); return 0;
        }
        int tot=cnt[1]+cnt[2]+cnt[3]+cnt[4];
        for(int v=1;v<=4;v++) p[v]=(cnt[v]+1.0)/(tot+4.0);
        int k=G.size();
        vector<double> wc(C.size());
        for(size_t i=0;i<C.size();i++){double w=1;for(int t=0;t<k;t++) w*=p[((C[i]>>(2*t))&3)+1]; wc[i]=w;}
        vector<double> wt[4];
        for(int j=0;j<=3;j++){
            int T=1<<(2*j); wt[j].assign(T,1.0);
            for(int t=0;t<T;t++){double w=1;for(int q=0;q<j;q++) w*=p[((t>>(2*q))&3)+1]; wt[j][t]=w;}
        }
        auto evalOpt=[&](Opt&o){
            double acc[16]={0};
            int T=1<<(2*o.j);
            int vals[3];
            for(size_t i=0;i<C.size();i++){
                for(int t=0;t<T;t++){
                    int c=0;
                    for(int q=0;q<o.nk;q++) vals[c++]=o.kv[q];
                    for(int q=0;q<o.u;q++){
                        int s=o.sl[q];
                        if(s<k) vals[c++]=((C[i]>>(2*s))&3)+1;
                        else vals[c++]=((t>>(2*(s-k)))&3)+1;
                    }
                    acc[Fi[vals[0]][vals[1]][vals[2]]]+=wc[i]*wt[o.j][t];
                }
            }
            double s=0; for(int q=0;q<16;q++) s+=acc[q];
            double H=0;
            for(int q=0;q<16;q++) if(acc[q]>0){double x=acc[q]/s; H-=x*log2(x);}
            o.H=H;
        };
        auto evaluate=[&](bool force)->Opt{
            Opt best; best.H=-1; best.u=0;
            int unt=n-nxt+1; int mmax=min(3,unt); int S=k+mmax;
            for(int u=3;u>=1;u--){
                vector<int> sl(u);
                // enumerate combos
                function<void(int,int)> rec=[&](int pos,int start){
                    if(pos==u){
                        int j=0; for(int q=0;q<u;q++) if(sl[q]>=k) j++;
                        if(j>0 && sl[u-1]!=k+j-1) return;
                        if(force){ if(j!=1 || k+1>16) return; }
                        else if(j>0 && (k+j>6 || (long long)C.size()*(1LL<<(2*j))>256)) return;
                        Opt o; o.u=u; o.j=j; for(int q=0;q<u;q++) o.sl[q]=sl[q];
                        if(u==3){ o.nk=0; evalOpt(o); if(o.H>best.H+1e-12) best=o; }
                        else if(u==2){
                            for(int v=1;v<=4;v++) if(reps[v].size()>=1){
                                o.nk=1;o.kv[0]=v; evalOpt(o); if(o.H>best.H+1e-12) best=o;
                            }
                        } else {
                            for(int v1=1;v1<=4;v1++)for(int v2=v1;v2<=4;v2++){
                                if(v1==v2){ if(reps[v1].size()<2) continue; }
                                else { if(reps[v1].empty()||reps[v2].empty()) continue; }
                                o.nk=2;o.kv[0]=v1;o.kv[1]=v2; evalOpt(o); if(o.H>best.H+1e-12) best=o;
                            }
                        }
                        return;
                    }
                    for(int s=start;s<S;s++){ sl[pos]=s; rec(pos+1,s+1); }
                };
                rec(0,0);
            }
            return best;
        };
        Opt best=evaluate(false);
        if(best.H<1e-9){
            if(nxt<=n){
                best=evaluate(true);
                if(best.u==0){ printf("! -1\n"); fflush(stdout); return 0; }
            } else {
                printf("! -1\n"); fflush(stdout); return 0;
            }
        }
        // perform query
        vector<int> idx;
        if(best.nk==1) idx.push_back(reps[best.kv[0]][0]);
        else if(best.nk==2){
            if(best.kv[0]==best.kv[1]){ idx.push_back(reps[best.kv[0]][0]); idx.push_back(reps[best.kv[0]][1]); }
            else { idx.push_back(reps[best.kv[0]][0]); idx.push_back(reps[best.kv[1]][0]); }
        }
        for(int q=0;q<best.u;q++){
            int s=best.sl[q];
            if(s<k) idx.push_back(G[s]); else idx.push_back(nxt+(s-k));
        }
        sort(idx.begin(),idx.end());
        printf("? %d %d %d\n",idx[0],idx[1],idx[2]); fflush(stdout);
        long long r; if(!(cin>>r)) return 0;
        if(r<0) return 0;
        if(!mp.count((int)r)) return 0;
        int ri=mp[(int)r];
        int j=best.j; int T=1<<(2*j);
        vector<uint32_t> NC;
        int vals[3];
        for(size_t i=0;i<C.size();i++){
            for(int t=0;t<T;t++){
                int c=0;
                for(int q=0;q<best.nk;q++) vals[c++]=best.kv[q];
                for(int q=0;q<best.u;q++){
                    int s=best.sl[q];
                    if(s<k) vals[c++]=((C[i]>>(2*s))&3)+1;
                    else vals[c++]=((t>>(2*(s-k)))&3)+1;
                }
                if(Fi[vals[0]][vals[1]][vals[2]]==ri) NC.push_back(C[i]|((uint32_t)t<<(2*k)));
            }
        }
        vector<int> NG=G; for(int q=0;q<j;q++) NG.push_back(nxt+q);
        nxt+=j;
        if(NC.empty()){ printf("! -1\n"); fflush(stdout); return 0; }
        int K=NG.size();
        vector<int> keep;
        for(int q=0;q<K;q++){
            uint32_t v0=(NC[0]>>(2*q))&3; bool same=true;
            for(auto c:NC) if(((c>>(2*q))&3)!=v0){same=false;break;}
            if(same){
                int id=NG[q]; val[id]=v0+1; cnt[v0+1]++;
                if(reps[v0+1].size()<2) reps[v0+1].push_back(id);
            } else keep.push_back(q);
        }
        vector<uint32_t> C2; C2.reserve(NC.size());
        for(auto c:NC){
            uint32_t nc=0;
            for(size_t pos=0;pos<keep.size();pos++) nc|=((c>>(2*keep[pos]))&3)<<(2*pos);
            C2.push_back(nc);
        }
        sort(C2.begin(),C2.end()); C2.erase(unique(C2.begin(),C2.end()),C2.end());
        vector<int> G2; for(int q:keep) G2.push_back(NG[q]);
        G=G2; C=C2;
    }
}

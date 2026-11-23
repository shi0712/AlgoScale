#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> sp;
vector<int> As, Bs;

int ask(const vector<int>& q){
    printf("? %d", (int)q.size());
    for(int x:q) printf(" %d", x);
    printf("\n");
    fflush(stdout);
    int r;
    if(scanf("%d",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}
void setsp(int x,int s){
    sp[x]=s;
    if(s==0) As.push_back(x); else Bs.push_back(x);
}
double f(double R,double m){
    if(R<=0) return 0;
    return -2*m+2*sqrt(m*m+R);
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    sp.assign(n,-1);
    setsp(0,0);
    int nxt=1;
    int py=-1,pz=-1;
    long long extraA=0;
    // Phase 1
    while(nxt<n){
        bool useA = As.size()>=Bs.size();
        vector<int>& L = useA?As:Bs;
        int s = useA?0:1;
        int m=L.size();
        double R=n-nxt;
        if(1+f(R-2.5,m+1.25) >= f(R,m)) break;
        vector<int> seq;
        bool hadPending = (py!=-1);
        int first;
        if(hadPending) first=py; else first=nxt++;
        seq.push_back(first); seq.push_back(L[0]);
        int slots=min(2,m-1);
        vector<int> I;
        for(int i=0;i<slots && nxt<n;i++){
            int u=nxt++;
            I.push_back(u);
            seq.push_back(u); seq.push_back(L[i+1]);
        }
        int r=ask(seq);
        int fs = (r&1)?1-s:s;
        setsp(first,fs);
        if(hadPending){ setsp(pz,1-fs); py=pz=-1; }
        int rest=r>>1;
        if(I.size()==1){
            setsp(I[0], rest?1-s:s);
        } else if(I.size()==2){
            if(rest==0){ setsp(I[0],s); setsp(I[1],s);}
            else if(rest==2){ setsp(I[0],1-s); setsp(I[1],1-s);}
            else { py=I[0]; pz=I[1]; }
        }
    }
    // Phase 2
    while(true){
        if(py==-1 && nxt>=n) break;
        if(py!=-1 && nxt>=n){ extraA+=1; py=pz=-1; break; }
        bool useA = As.size()>=Bs.size();
        vector<int>& L = useA?As:Bs;
        int s = useA?0:1;
        int m=L.size();
        vector<int> seq;
        bool hadPending=(py!=-1);
        int first = hadPending?py:nxt++;
        seq.push_back(first); seq.push_back(L[0]);
        int k=0;
        for(int i=1;i<m && nxt<n;i++){
            seq.push_back(nxt++); seq.push_back(L[i]);
            k++;
        }
        int r=ask(seq);
        int fs=(r&1)?1-s:s;
        int cnt=r>>1;
        long long interiorA = (s==0)?(k-cnt):cnt;
        extraA+=interiorA;
        setsp(first,fs);
        if(hadPending){ setsp(pz,1-fs); py=pz=-1; }
    }
    long long total = (long long)As.size() + extraA;
    printf("! %lld\n", total);
    fflush(stdout);
    return 0;
}

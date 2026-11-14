#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
int main(){
    long long n,P; cin>>n>>P;
    int N=n;
    // R[E][m] = G[m][E], Q[T][m] = G[m][T-m], valid for m+E <= n-1
    vector<vector<u64>> R(N+1), Q(N+1);
    for(int E=0;E<=N;E++) R[E].assign(N+1-E+1,0);
    for(int T=0;T<=N;T++) Q[T].assign(T+1,0);
    vector<u64> inv(N+2,1);
    for(int i=2;i<=N+1;i++) inv[i]=(P-(P/i)*inv[P%i]%P)%P;
    const u64 PP=(u64)P*P, LIM=8*PP;
    for(int E=0;E<N;E++){R[E][0]=1;Q[E][0]=1;}
    for(int m=1;m<=N-1;m++){
        const u64* R0=R[0].data();
        for(int E=0;m+E<=N-1;E++){
            const u64* RE=R[E].data();
            const u64* QT=Q[m-1+E].data();
            u64 acc=R0[m-1];
            for(int t=1;t<=m-2;t++){
                int k=m-t-1;
                u64 b=R0[k]+P-QT[k];
                acc+=RE[t]*b;
                if(acc>=LIM) acc-=LIM;
            }
            u64 v=acc%P*inv[m+E]%P;
            R[E][m]=v; Q[m+E][m]=v;
        }
    }
    u64 ans=R[0][N-1]%P;
    for(int i=1;i<=N-1;i++) ans=ans*i%P;
    cout<<ans<<endl;
}

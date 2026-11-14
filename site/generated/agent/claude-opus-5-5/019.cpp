#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64; typedef unsigned u32;
u64 M;
u64 pw(u64 b,u64 e){u64 r=1;b%=M;while(e){if(e&1)r=r*b%M;b=b*b%M;e>>=1;}return r;}
int main(){
    int n; scanf("%d %llu",&n,&M);
    int N=n+2;
    // binomials
    vector<vector<u32>> C(N+1,vector<u32>(N+1,0));
    for(int i=0;i<=N;i++){C[i][0]=1;for(int j=1;j<=i;j++)C[i][j]=(C[i-1][j-1]+(u64)C[i-1][j])%M;}
    // unsigned Stirling first kind c(i,j), i<=n+1
    vector<vector<u32>> S(n+2,vector<u32>(n+2,0));
    S[0][0]=1;
    for(int i=1;i<=n+1;i++)for(int j=1;j<=i;j++)S[i][j]=(S[i-1][j-1]+(u64)(i-1)*S[i-1][j])%M;
    // Eulerian A[p][k]
    vector<vector<u32>> E(n+1,vector<u32>(n+1,0));
    E[0][0]=1;
    for(int p=1;p<=n;p++)for(int k=0;k<p;k++){
        u64 v=(u64)(k+1)*E[p-1][k]%M;
        if(k>0)v+=(u64)(p-k)*E[p-1][k-1]%M;
        E[p][k]=v%M;
    }
    vector<u64> invf(n+2);{u64 f=1;for(int i=1;i<=n+1;i++)f=f*i%M;invf[n+1]=pw(f,M-2);for(int i=n+1;i>0;i--)invf[i-1]=invf[i]*i%M;}
    // U_p(t), V_p(s), degree < n kept
    vector<vector<u32>> U(n+1,vector<u32>(n,0)),V(n+1,vector<u32>(n,0));
    auto neg=[&](u64 x)->u64{return x?M-x:0;};
    for(int p=0;p<=n;p++){
        // U_p = (1-t)^n if p==0 else t*A_p(t)*(1-t)^{n-p}
        if(p==0){for(int d=0;d<n;d++)U[0][d]=(d&1)?neg(C[n][d]):C[n][d];}
        else{
            vector<u64> acc(n,0);
            for(int k=0;k<p;k++){ if(!E[p][k])continue; u64 a=E[p][k];
                for(int i=0;i<=n-p;i++){int d=k+1+i; if(d>=n)break;
                    u64 v=a*C[n-p][i]%M; acc[d]+= (i&1)?neg(v):v; if(acc[d]>=M)acc[d]-=M;} }
            for(int d=0;d<n;d++)U[p][d]=acc[d];
        }
        // W_p(s)=sum_{m=p}^n C(n,m) e_{m,p} s^{n-m}(1-s)^{m-p}, e_{m,p}=c(m+1,p+1)/m!
        vector<u64> W(n+1,0);
        for(int m=p;m<=n;m++){
            u64 al=(u64)C[n][m]*S[m+1][p+1]%M*invf[m]%M; if(!al)continue;
            for(int i=0;i<=m-p;i++){int j=n-m+i; u64 v=al*C[m-p][i]%M; W[j]+=(i&1)?neg(v):v; if(W[j]>=M)W[j]-=M;}
        }
        // V_p = A_p(s)*W_p(s)
        vector<u64> acc(n,0);
        int degA=(p==0?0:p-1);
        for(int k=0;k<=degA;k++){u64 a=E[p][k]; if(!a)continue;
            for(int j=0;j+k<n && j<=n;j++){ if(!W[j])continue; acc[j+k]=(acc[j+k]+a*W[j])%M; }}
        for(int d=0;d<n;d++)V[p][d]=acc[d];
    }
    // h[m][d] = sum_p V[p][m]*U[p][d]
    string out; out.reserve((size_t)n*n*11);
    vector<u64> acc(n);
    char buf[24];
    for(int m=0;m<n;m++){
        fill(acc.begin(),acc.end(),0);
        for(int p0=0;p0<=n;p0+=16){
            int p1=min(n+1,p0+16);
            for(int p=p0;p<p1;p++){u64 v=V[p][m]; if(!v)continue; const u32* up=U[p].data();
                for(int d=0;d<n;d++)acc[d]+=v*(u64)up[d];}
            for(int d=0;d<n;d++)acc[d]%=M;
        }
        for(int d=0;d<n;d++){int l=snprintf(buf,sizeof buf,"%llu",acc[d]);out.append(buf,l);out.push_back(d+1<n?' ':'\n');}
    }
    fwrite(out.data(),1,out.size(),stdout);
}

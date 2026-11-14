#include <bits/stdc++.h>
using namespace std;
// Dual of the TU LP: choose nonincreasing Y1,Y2 (one per family) minimizing
// sum e1*Y1 + e2*Y2 + max(0, w - [F1]Y1 - [F2]Y2).
static long long solveDual(int n,const vector<int>&c,const vector<int>&w,int K){
    int R=2*K+1;
    vector<long long> dp(R*R,0),nd(R*R);
    int cnt1=0,cnt2=0;
    for(int i=0;i<n;i++){
        bool f1=(c[i]!=0), f2=(c[i]!=1);
        int e1=0,e2=0;
        if(f1){cnt1++; e1=(cnt1%2==0);}
        if(f2){cnt2++; e2=(cnt2%2==0);}
        // suffix min over a'>=a, b'>=b
        for(int a=R-1;a>=0;a--)for(int b=R-1;b>=0;b--){
            long long v=dp[a*R+b];
            if(a+1<R) v=min(v,nd[(a+1)*R+b]);
            if(b+1<R) v=min(v,nd[a*R+b+1]);
            nd[a*R+b]=v;
        }
        for(int a=0;a<R;a++)for(int b=0;b<R;b++){
            int Y1=a-K,Y2=b-K;
            long long s=0;
            int t=w[i];
            if(f1){s+=e1*Y1; t-=Y1;}
            if(f2){s+=e2*Y2; t-=Y2;}
            if(t>0)s+=t;
            dp[a*R+b]=nd[a*R+b]+s;
        }
    }
    return *min_element(dp.begin(),dp.end());
}
int main(){
    int n; if(scanf("%d",&n)!=1)return 0;
    static char buf[600005]; if(scanf("%s",buf)!=1)return 0;
    vector<int> c(n),w(n),z(n,0);
    int closes=0,m1=0,m2=0;
    for(int i=0;i<n;i++){if(scanf("%d",&c[i])!=1)return 0; w[i]=buf[i]==')'?1:-1; closes+=buf[i]==')'; if(c[i]!=0)m1++; if(c[i]!=1)m2++;}
    if(m1%2||m2%2){puts("-1");return 0;}
    if(solveDual(n,c,z,1)<0){puts("-1");return 0;}
    long long D=solveDual(n,c,w,6);
    printf("%lld\n",closes-D);
}

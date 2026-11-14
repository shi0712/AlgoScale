#include <bits/stdc++.h>
using namespace std;

/*
An achievable set is a strongly connected induced subgraph.

For l < r, choose w[l][r] so that the total product of consecutive
gap weights, over all subsets X of {i: l<i<r, a[i]>l, b[i]<r}, is 1.
The empty subset contributes w[l][r], so shorter gaps determine it.

Consider a set whose minimum and maximum can reach each other. Its
minimum reaches every selected vertex, since every edge is an interval.
The vertices that can return form a strongly connected set T containing
both extremes. Every remaining vertex lies in a gap (l,r) of T and has
a[i]>l, b[i]<r. Conversely, arbitrary such gap subsets can be added.
Consequently, summing products of w over sets with mutually reachable
extremes counts each strongly connected T with weight exactly 1.

For sorted selected vertices, the minimum reaches the maximum iff each
next vertex is at most the maximum b among the preceding vertices.
The reverse reachability fails exactly when some gap (i,j) has a[k]>i
for every selected k>=j. Apply inclusion-exclusion to these latter gaps.
Only the left endpoint p of the latest marked gap matters for subsequent
vertices: they must have a>p. A layer stores (last vertex, maximum b).
Marks increase p, so completed earlier layers can be aggregated in sumf.

Time O(n^4), memory O(n^2).
*/
static constexpr int MOD=998244353;
static constexpr int N=405;
int n,a[N],b[N],w[N][N],f[N][N],sumf[N][N];
inline void addto(int &x,int y){x+=y;if(x>=MOD)x-=MOD;}
inline void subfrom(int &x,int y){x-=y;if(x<0)x+=MOD;}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    cin>>n;
    for(int i=1;i<=n;i++)cin>>a[i]>>b[i];
    // The sum of gap weights over subsets whose outgoing intervals stay
    // strictly between the two endpoints must equal one.
    int ids[N],d[N];
    for(int len=1;len<n;len++)for(int l=1;l+len<=n;l++){
        int r=l+len,m=0;
        for(int i=l+1;i<r;i++)if(a[i]>l && b[i]<r)ids[m++]=i;
        int total=0;
        for(int x=0;x<m;x++){
            int i=ids[x];
            unsigned long long s=w[l][i];
            int batch=0;
            for(int y=0;y<x;y++){
                s+=(unsigned long long)d[y]*w[ids[y]][i];
                if(++batch==16){s%=MOD;batch=0;}
            }
            d[x]=s%MOD;
            addto(total,(long long)d[x]*w[i][r]%MOD);
        }
        w[l][r]=1-total;
        if(w[l][r]<0)w[l][r]+=MOD;
    }
    int ans=0;
    // p is the left endpoint of the last marked backward-separating gap.
    // Marks implement inclusion-exclusion; subsequent vertices require a>p.
    for(int p=0;p<n;p++){
        memset(f,0,sizeof(f));
        if(p==0){
            for(int i=1;i<=n;i++)f[i][b[i]]=1;
        }else{
            // All earlier layers have contributed to sumf[p]. Start a new
            // marked gap after p, changing the inclusion-exclusion sign.
            for(int reach=p+1;reach<=n;reach++)if(sumf[p][reach]){
                int val=sumf[p][reach];
                for(int j=p+1;j<=reach;j++)if(a[j]>p && w[p][j]){
                    int z=(long long)val*w[p][j]%MOD;
                    subfrom(f[j][max(reach,b[j])],z);
                }
            }
        }
        for(int i=p+1;i<=n;i++)if(a[i]>p){
            // Lists avoid considering destinations forbidden by the latest mark.
            int cnt=0;
            for(int j=i+1;j<=n;j++)if(a[j]>p && w[i][j])ids[cnt++]=j;
            for(int reach=i;reach<=n;reach++)if(f[i][reach]){
                int val=f[i][reach];
                addto(ans,val);
                addto(sumf[i][reach],val);
                for(int t=0;t<cnt && ids[t]<=reach;t++){
                    int j=ids[t];
                    int z=(long long)val*w[i][j]%MOD;
                    addto(f[j][max(reach,b[j])],z);
                }
            }
        }
    }
    cout<<ans<<'\n';
}

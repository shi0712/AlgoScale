#include <bits/stdc++.h>
using namespace std;
using i64=long long;
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n,m;cin>>n>>m;vector<int>trump(n);for(int &x:trump)cin>>x;
    vector<vector<i64>>a(n,vector<i64>(m)),b=a;
    for(auto &r:a)for(auto &x:r)cin>>x;
    for(auto &r:b)for(auto &x:r)cin>>x;
    int lim=1<<m;vector<i64>surplus(lim),balance(lim),need(lim);vector<char>bad(lim);
    for(int i=0;i<n;i++){
        for(int s=1;s<lim;s++){
            int r=__builtin_ctz((unsigned)s),prev=s&(s-1);
            balance[s]=balance[prev]+b[i][r]-a[i][r];
            need[s]=max(need[prev],a[i][r]-balance[prev]);
            if(trump[i]){bad[s]|=need[s]!=0;surplus[s]+=balance[s];}
            else surplus[s]-=need[s];
        }
    }
    int safe=0;for(int s=0;s<lim;s++)if(!bad[s]&&surplus[s]>=0)safe|=s;
    i64 answer=0;for(int i=0;i<n;i++)for(int r=0;r<m;r++)if(!(safe>>r&1))answer+=a[i][r];
    cout<<answer<<'\n';
}

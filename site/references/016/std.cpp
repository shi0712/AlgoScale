#include <bits/stdc++.h>
using namespace std;
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int n;string s;cin>>n>>s;vector<int>c(n);array<int,3>cnt{},initial{};
    for(int i=0;i<n;i++){cin>>c[i];cnt[c[i]]++;initial[c[i]]+=s[i]=='(';}
    if((cnt[0]+cnt[2])%2||(cnt[1]+cnt[2])%2){cout<<-1<<'\n';return 0;}
    int h[2]={(cnt[0]+cnt[2])/2,(cnt[1]+cnt[2])/2};
    int lo=max({0,h[0]-cnt[0],h[1]-cnt[1]}),hi=min({cnt[2],h[0],h[1]});
    array<int,3>pref{};
    for(int i=0;i<n;i++){
        pref[c[i]]++;
        for(int j=0;j<2;j++){
            int required=(pref[j]+pref[2]+1)/2;
            lo=max(lo,required-pref[j]);hi=min(hi,h[j]+pref[2]-required);
        }
    }
    if(lo>hi){cout<<-1<<'\n';return 0;}
    auto cost=[&](int blue){
        array<int,3>target{h[0]-blue,h[1]-blue,blue},add{},remove{};
        int base=0;
        for(int j=0;j<3;j++){add[j]=max(0,target[j]-initial[j]);remove[j]=max(0,initial[j]-target[j]);base+=add[j]+remove[j];}
        string a=s;
        for(int i=0;i<n;i++)if(a[i]==')'&&add[c[i]]){a[i]='(';--add[c[i]];}
        for(int i=n-1;i>=0;i--)if(a[i]=='('&&remove[c[i]]){a[i]=')';--remove[c[i]];}
        array<int,3>seen{},cap{},closing{},right=target,balance{},lower{};int pairneed[2]={};
        for(int i=0;i<n;i++){
            int j=c[i];if(seen[j]++<target[j]&&a[i]==')')++cap[j];
        }
        for(int i=0;i<n;i++){
            int j=c[i];if(a[i]=='('){--right[j];++balance[j];}else{++closing[j];--balance[j];}
            for(int j=0;j<2;j++){
                int deficit=max(0,(-balance[j]-balance[2]+1)/2);
                int x=min(closing[j],right[j]),y=min(closing[2],right[2]);
                lower[j]=max(lower[j],deficit-y);lower[2]=max(lower[2],deficit-x);pairneed[j]=max(pairneed[j],deficit);
            }
        }
        int b=min(cap[2],max({lower[2],pairneed[0]-lower[0],pairneed[1]-lower[1]}));
        int l=max(lower[0],pairneed[0]-b),g=max(lower[1],pairneed[1]-b);
        return base+2*(b+l+g);
    };
    // The fixed-blue-count cost is a convex integral min-cost-flow value function.
    while(lo<hi){int mid=(lo+hi)/2;if(cost(mid)<=cost(mid+1))hi=mid;else lo=mid+1;}
    cout<<cost(lo)<<'\n';
}

#include <bits/stdc++.h>
using namespace std;
template<class Ask> vector<int> recover(int n, Ask ask){
    array<long long,5> counts{};map<array<int,3>,int> table;
    for(int i=0;i<5;++i)for(int j=i+1;j<5;++j)for(int k=j+1;k<5;++k){
        int r=ask(i,j,k);table[{i,j,k}]=r;
        for(int t:{i,j,k})counts[t]+=r;
    }
    vector<int> order={0,1,2,3,4};sort(order.begin(),order.end(),[&](int i,int j){return counts[i]<counts[j];});
    auto get=[&](int i,int j,int k){
        array<int,3> key{order[i],order[j],order[k]};sort(key.begin(),key.end());return table[key];
    };
    vector<int> a(n);int u=order[0],v=order[1],w=order[2],y=order[3],z=order[4];
    a[u]=(int)((1LL*get(0,1,2)+get(0,1,4)-get(2,3,4))/2);
    a[w]=get(0,1,2)-a[u];a[z]=get(0,1,4)-a[u];
    a[v]=get(1,2,4)-a[z];a[y]=get(0,2,3)-a[u];
    for(int i=5;i<n;++i){
        int j=0;
        for(int t=1;t<i-1;++t)
            if(a[order[t+1]]-a[order[t]]<a[order[j+1]]-a[order[j]])j=t;
        int l=order[j],r=order[j+1],response=ask(l,r,i);
        if(a[l]==a[r] || response>a[l]+a[r]) a[i]=response-a[l];
        else if(response<a[l]+a[r]) a[i]=response-a[r];
        else if(j>=2) a[i]=ask(order[0],order[1],i)-a[order[0]];
        else a[i]=ask(order[i-2],order[i-1],i)-a[order[i-1]];
        auto at=lower_bound(order.begin(),order.end(),a[i],[&](int pos,int val){return a[pos]<val;});
        order.insert(at,i);
    }
    return a;
}
#ifdef VERIFY
int main(){
    mt19937 gen(813923);int maximum=0;
    for(int t=0;t<1500;++t){
        int n=5+gen()%996,calls=0;vector<int> a(n);for(int& x:a)x=gen()%(t%3?1000000001:6);
        auto got=recover(n,[&](int i,int j,int k){++calls;return min({a[i],a[j],a[k]})+max({a[i],a[j],a[k]});});
        if(got!=a || calls>min(2*n,n+33))return 1;maximum=max(maximum,calls-n);
    }
    cout<<"1500 arrays verified, maximum observed calls minus n = "<<maximum<<'\n';
}
#else
int main(){int n;if(!(cin>>n))return 0;auto a=recover(n,[](int i,int j,int k){
    cout<<"? "<<i+1<<' '<<j+1<<' '<<k+1<<endl;int v;if(!(cin>>v)||v<0)exit(0);return v;});
    cout<<"!";for(int x:a)cout<<' '<<x;cout<<endl;
}
#endif

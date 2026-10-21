#include <bits/stdc++.h>
using namespace std;
using U=__uint128_t;
constexpr int LIMIT=24;
struct Counter{
    U c[LIMIT+1][LIMIT+1]{},a[LIMIT+1][LIMIT+1]{},e[LIMIT+1]{};
    Counter(){
        c[0][0]=a[0][0]=e[0]=1;
        for(int i=1;i<=LIMIT;i++){
            c[i][0]=c[i][i]=1;
            for(int j=1;j<i;j++)c[i][j]=c[i-1][j-1]+c[i-1][j];
            for(int j=1;j<=i;j++)a[i][j]=a[i][j-1]+a[i-1][i-j];
            e[i]=a[i][i];
        }
        // Computed at runtime; this threshold justifies the forced prefix.
        assert(e[23]>=U(1000000000000000000ULL));
    }
    U split(int l,int r)const{return c[l+r][l]*e[l]*e[r];}
    vector<int> unrank(int n,U k,bool fixed_even_valleys)const{
        assert(n<=LIMIT);
        vector<int>used(n+2),left(n+2),right(n+2),out;
        int parity=0;U total=e[n];
        if(!fixed_even_valleys){
            if(k>2*e[n])return {};
            for(int v=1;v<=n;v++){
                U count=split(v-1,n-v);
                if(k>count)k-=count;
                else{used[v]=1;out.push_back(v);parity=v%2;total=count;break;}
            }
        }
        while((int)out.size()<n){
            int remain=n-out.size();
            for(int i=1;i<=n;){
                if(used[i]){i++;continue;}
                int j=i;while(j<=n&&!used[j])j++;
                for(int v=i;v<j;v++){left[v]=i;right[v]=j-1;}i=j;
            }
            bool found=false;
            for(int v=1;v<=n;v++)if(!used[v]){
                if(v%2!=parity&&((v>1&&!used[v-1])||(v<n&&!used[v+1])))continue;
                int len=right[v]-left[v]+1;
                U numerator=(total/e[len])*split(v-left[v],right[v]-v)*len;
                assert(total%e[len]==0&&numerator%remain==0);
                U count=numerator/remain;
                if(k>count)k-=count;
                else{used[v]=1;out.push_back(v);total=count;found=true;break;}
            }
            assert(found);
        }
        return out;
    }
};
void solve(const Counter&counter,int n,unsigned long long k){
    bool first=true;
    auto emit=[&](int v){if(!first)cout<<' ';first=false;cout<<v;};
    if(n<=23){
        auto answer=counter.unrank(n,U(k),false);
        if(answer.empty()){cout<<-1<<'\n';return;}
        for(int v:answer)emit(v);
    }
    else{
        emit(1);int base=1;
        while(n-base>24){emit(base+2);emit(base+1);base+=2;}
        for(int v:counter.unrank(n-base,U(k),true))emit(base+v);
    }
    cout<<'\n';
}
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);Counter counter;int n;unsigned long long k;
#ifdef BATCH_TEST
    while(cin>>n>>k)solve(counter,n,k);
#else
    if(cin>>n>>k)solve(counter,n,k);
#endif
}

#include <bits/stdc++.h>
using namespace std;
int main(){
    long long cnt;
    if(!(cin>>cnt)) return 0;
    if(cnt<0) return 0;
    long long ans=0;
    long long r=cnt;
    int q=0;
    while(r>0){
        long long x=1LL<<q;
        cout<<"- "<<x<<"\n"<<flush;
        long long c;
        if(!(cin>>c)) return 0;
        if(c<0) return 0;
        long long p=c-(r-1);
        ans+=1LL<<p;
        r--;
        q=(int)p;
    }
    cout<<"! "<<ans<<"\n"<<flush;
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

bool take_weaker;
int better(int a,int b) {
    if(!a || !b) return a+b;
    cout<<"? "<<a<<' '<<b<<endl;
    int w; if(!(cin>>w) || (w!=a && w!=b)) exit(0);
    return take_weaker ? a+b-w : w;
}
int main() {
    int n,k; cin>>n>>k;
    take_weaker = n-k<k;
    int needed=min(k,n-k);
    vector<int> answer;
    if(n==k) {
        for(int i=1;i<=n;++i) answer.push_back(i);
    } else {
        int size=1;while(size<n)size*=2;
        vector<int> winner(2*size);
        for(int i=0;i<n;++i) winner[size+i]=i+1;
        for(int p=size-1;p>=1;--p) winner[p]=better(winner[p*2],winner[p*2+1]);
        for(int j=0;j<needed;++j) {
            int v=winner[1];answer.push_back(v);
            if(j+1==needed) break;
            int p=size+v-1;winner[p]=0;
            while((p/=2)!=0) winner[p]=better(winner[p*2],winner[p*2+1]);
        }
        if(take_weaker) {
            vector<bool> discarded(n+1);
            for(int v:answer) discarded[v]=true;
            answer.clear();
            for(int v=1;v<=n;++v) if(!discarded[v]) answer.push_back(v);
        }
    }
    cout<<"!";
    for(int v:answer) cout<<' '<<v;
    cout<<endl;
}

#include <bits/stdc++.h>
using namespace std;

int visible;
void flip(int l,int r) {
    cout<<l<<' '<<r<<endl;
    if(!(cin>>visible) || visible<0)exit(0);
}
int main() {
    int n;cin>>n>>visible;
    for(int i=1;i<=n && visible<n;++i) {
        for(int step=1;step<(1<<(n-i+1));++step) {
            int bit=__builtin_ctz((unsigned)step), before=visible;
            flip(i,i+bit);
            if(visible==n)return 0;
            if(bit==0 && before!=visible) {
                if(visible<before)flip(i,i);
                break;
            }
        }
    }
}

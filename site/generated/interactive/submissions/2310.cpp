#include <bits/stdc++.h>
using namespace std;

int n,k;
bool ask(int c){
    cout<<"? "<<c<<endl;
    string s;
    if(!(cin>>s)) exit(0);
    if(s=="E"||s=="-1") exit(0);
    return s[0]=='Y';
}
int main(){
    if(!(cin>>n>>k)) return 0;
    if(n==1){cout<<"! 1"<<endl;return 0;}
    int b=max(1,k/2);
    int m=n/b;
    vector<char> alive(n+1,1);
    for(int s=0;s<m/2;s++){
        cout<<"R"<<endl;
        for(int i=0;i<m;i++){
            int off = (i%2==1)? (i+1)/2 : -(i/2);
            int blk=((s+off)%m+m)%m;
            for(int t=0;t<b;t++){
                int c=blk*b+t+1;
                if(!alive[c]) continue;
                if(ask(c)) alive[c]=0;
            }
        }
    }
    int d=0;
    for(int i=1;i<=n;i++) if(alive[i]) d++;
    cout<<"! "<<d<<endl;
    return 0;
}

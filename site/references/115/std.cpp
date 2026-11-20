#include <bits/stdc++.h>
using namespace std;
int ask(int i,int j) {
    cout<<"? "<<i<<' '<<j<<endl;
    int t;if(!(cin>>t) || t<0)exit(0);return t;
}
int main(){
    int n;cin>>n;
    if(n==1){cout<<"!\n1\n0"<<endl;return 0;}
    vector<int> col(n);
    if(n%2==0){
        int total=0;
        for(int i=0;i<n-1;i++){col[i]=ask(i,0);total^=col[i];}
        for(int i=0;i<n;i++)total^=i;
        col[n-1]=total;
    }else{
        for(int i=0;i<n;i++)col[i]=ask(i,0);
    }
    vector<int> shifts;
    for(int s=0;s<n;s++){
        bool ok=true;for(int v:col)if((v^s)>=n){ok=false;break;}
        if(ok && col[s]==s)shifts.push_back(s);
    }
    vector<vector<int>> fingerprints;
    for(int s:shifts){
        vector<int> inverse(n);
        for(int i=0;i<n;i++)inverse[col[i]^s]=i;
        for(int& x:inverse)x^=col[0]^s;
        fingerprints.push_back(move(inverse));
    }
    vector<int> candidates(shifts.size()); iota(candidates.begin(),candidates.end(),0);
    for(int j=1;j<n-(n%2==0);j++){
        bool splits=false;
        for(int k:candidates)if(fingerprints[k][j]!=fingerprints[candidates[0]][j])splits=true;
        if(!splits)continue;
        int result=ask(0,j);
        candidates.erase(remove_if(candidates.begin(),candidates.end(),[&](int k){
            return fingerprints[k][j]!=result;}),candidates.end());
    }
    cout<<"!\n"<<candidates.size()<<'\n';
    for(int v:col)cout<<(v^shifts[candidates[0]])<<' ';
    cout<<endl;
}

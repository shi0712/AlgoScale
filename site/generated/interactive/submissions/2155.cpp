#include <bits/stdc++.h>
using namespace std;

int main(){
 ios::sync_with_stdio(false);
 cin.tie(nullptr);
 int n;
 if(!(cin>>n)) return 0;
 long long totalK=0;
 auto ask = [&](const vector<int>& v)->int{
    totalK += v.size();
    cout << "? " << v.size();
    for(int x: v) cout << ' ' << x;
    cout << '\n';
    cout.flush();
    int ans;
    if(!(cin>>ans)) exit(0);
    if(ans==-1) exit(0);
    return ans;
 };
 auto ceil_div = [](int a,int b)->int{ return (a + b - 1)/b; };
 auto ceil_log2 = [](int x)->int{ int r=0; while((1<<r) < x) ++r; return r; };
 int N=n-1;
 int bestQ=1,bestEst=1;
 if(N>1){
   bestQ=N; bestEst=INT_MAX;
   for(int q=2;q<=N;q++){
     int c=q/2;
     int rem=N-q;
     int batches = rem? ceil_div(rem,c):0;
     int est=q+batches;
     int sum=2*N+batches;
     if(sum>100000) continue;
     if(est<bestEst){ bestEst=est; bestQ=q; }
   }
 }
 vector<int> knownA, knownB;
 knownA.reserve(n); knownB.reserve(n);
 knownA.push_back(0);
 int totalA=1;
 int q=0, next=1;
 auto testOne = [&](){
    int i=next++;
    int ans=ask({0,i});
    if(ans==0){ knownA.push_back(i); totalA++; }
    else knownB.push_back(i);
    q++;
 };
 while(next < n){
    int rem = N - q;
    int a=(int)knownA.size(), b=(int)knownB.size();
    int t=max(a,b);
    if(rem==0) break;
    if(t<2){ testOne(); continue; }
    int c=t-1;
    int stopNow=ceil_div(rem,c);
    int contBest=1 + ceil_div(rem-1, c+1);
    int currentTotal=q+stopNow;
    if(q>=bestQ || (currentTotal <= bestEst && stopNow <= contBest)) break;
    testOne();
 }
 auto findOpposite = [&](const vector<int>& H, int sep)->int{
    vector<int> cur=H;
    while(cur.size()>1){
       int mid=cur.size()/2;
       vector<int> left(cur.begin(), cur.begin()+mid);
       vector<int> seq;
       seq.reserve(left.size()+1);
       seq.push_back(sep);
       seq.insert(seq.end(), left.begin(), left.end());
       int ans=ask(seq);
       if(ans==0) cur.assign(cur.begin()+mid, cur.end());
       else cur.assign(cur.begin(), cur.begin()+mid);
    }
    return cur[0];
 };
 vector<int> remList;
 remList.reserve(n);
 for(int i=next;i<n;i++) remList.push_back(i);
 size_t pos=0;
 while(pos < remList.size()){
    int a=(int)knownA.size(), b=(int)knownB.size();
    int t=max(a,b);
    if(t<2){
       int u=remList[pos++];
       int ans=ask({0,u});
       if(ans==0){ knownA.push_back(u); totalA++; }
       else knownB.push_back(u);
       continue;
    }
    bool useA = (a >= b);
    int c=t-1;
    int m = min((int)remList.size() - (int)pos, c);
    if(m <= 0){
       int u=remList[pos++];
       int ans=ask({0,u});
       if(ans==0){ knownA.push_back(u); totalA++; }
       else knownB.push_back(u);
       continue;
    }
    vector<int> G(remList.begin()+pos, remList.begin()+pos+m);
    const vector<int>* sepPtr = useA ? &knownA : &knownB;
    const vector<int>& sep = *sepPtr;
    vector<int> seq;
    seq.reserve(2*m+1);
    for(int j=0;j<m;j++){
       seq.push_back(sep[j]);
       seq.push_back(G[j]);
    }
    seq.push_back(sep[m]);
    int ans=ask(seq);
    int acnt,bcnt;
    if(useA){ bcnt=ans/2; acnt=m-bcnt; }
    else { acnt=ans/2; bcnt=m-acnt; }
    totalA += acnt;
    if(acnt == m){
       for(int u: G) knownA.push_back(u);
    } else if(bcnt == m){
       for(int u: G) knownB.push_back(u);
    } else {
       int remAfter = (int)remList.size() - (int)pos - m;
       if(remAfter > 0 && min(acnt,bcnt) == 1){
          int a2 = a + acnt;
          int b2 = b + bcnt;
          int c_new = max(a2,b2) - 1;
          bool majA = (acnt >= bcnt);
          int cost = ceil_log2(m);
          if(!majA && knownB.empty()) cost += ceil_log2(m);
          int saving = ceil_div(remAfter, c) - ceil_div(remAfter, c_new);
          if(saving >= cost && totalK + 2LL*m <= 90000){
             if(majA){
                int minLabel = findOpposite(G, knownA[0]);
                for(int u: G) if(u != minLabel) knownA.push_back(u);
                knownB.push_back(minLabel);
             } else {
                int sepB;
                bool sepInG = false;
                if(!knownB.empty()) sepB = knownB[0];
                else {
                   sepB = findOpposite(G, knownA[0]);
                   sepInG = true;
                }
                int minLabel;
                if(sepInG){
                   vector<int> G2;
                   G2.reserve(m-1);
                   for(int u: G) if(u != sepB) G2.push_back(u);
                   minLabel = findOpposite(G2, sepB);
                } else {
                   minLabel = findOpposite(G, sepB);
                }
                for(int u: G) if(u != minLabel) knownB.push_back(u);
                knownA.push_back(minLabel);
             }
          }
       }
    }
    pos += m;
 }
 cout << "! " << totalA << endl;
 return 0;
}

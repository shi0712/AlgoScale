#include <bits/stdc++.h>
using namespace std;
struct Node { int score, cnt; };
Node better(Node a, Node b) { return a.score!=b.score ? (a.score>b.score?a:b) : (a.cnt>b.cnt?a:b); }
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int T;cin>>T;while(T--){string s;int K;cin>>s>>K;int n=s.size();
 // An interval is a possible run after its single changed character is set.
 // A matching neighbor outside it must have been removed earlier.
 struct Option {int len; bool left,right;};
 vector<vector<Option>> opts(n);
 for(int i=0;i<n;i++)for(int len=3;len<=5&&i+len<=n;len++){
  char a=s[i], b=s[i+1], c=s[i+2];char x=(a==b||a==c)?a:b;
  int mismatch=0;for(int j=i;j<i+len;j++)mismatch+=(s[j]!=x);
  if(mismatch==1)opts[i].push_back({len,i>0&&s[i-1]==x,i+len<n&&s[i+len]==x});
 }
 // State 0: previous character remains; state 1: an interval ended here
 // without requiring its right neighbor; state 2: it requires that neighbor.
 // Opposite requirements across the same boundary would form a cycle.
 auto calc=[&](int penalty){
  const Node bad{-1000000000,-1000000000};
  vector<array<Node,3>> dp(n+1);for(auto &v:dp)v={bad,bad,bad};dp[0][0]={0,0};
  for(int i=0;i<n;i++){
   dp[i+1][0]=better(dp[i+1][0],better(dp[i][0],dp[i][1]));
   for(auto o:opts[i]){
    Node from=o.left?dp[i][1]:better(dp[i][0],better(dp[i][1],dp[i][2]));
    from.score+=o.len-penalty;from.cnt++;
    dp[i+o.len][1+o.right]=better(dp[i+o.len][1+o.right],from);
   }
  }
  return better(dp[n][0],dp[n][1]);
 };
 // Penalize each edit and prefer more edits on a tie.
 int lo=0,hi=6;while(lo<hi){int m=(lo+hi+1)/2;if(calc(m).cnt>=K)lo=m;else hi=m-1;}
 auto ans=calc(lo);cout<<ans.score+lo*min(K,ans.cnt)<<'\n';
 }
}

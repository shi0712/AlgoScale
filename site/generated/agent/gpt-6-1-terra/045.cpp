#include <bits/stdc++.h>
using namespace std;
struct Node { int l=0,r=0,rank=1,heap=0,h=0; };
vector<Node> a;
int meld(int x,int y){
 if(!x) return y; if(!y) return x;
 if(a[x].h<a[y].h) swap(x,y);
 a[x].r=meld(a[x].r,y);
 if(a[a[x].l].rank<a[a[x].r].rank) swap(a[x].l,a[x].r);
 a[x].rank=a[a[x].r].rank+1;
 return x;
}
int pop(int &heap){int x=heap;heap=meld(a[x].l,a[x].r);a[x].l=a[x].r=0;a[x].rank=1;return x;}
void insert(int &heap,int v){heap=meld(heap,v);}
void update(int v){a[v].h=a[v].heap ? a[a[v].heap].h+1:0;}
int mergeBranches(int x,int y){
 vector<int> st;
 while(true){
  int xx=0,yy=0;
  if(a[x].h){xx=pop(a[x].heap);yy=pop(a[y].heap);}
  a[x].heap=meld(a[x].heap,a[y].heap);
  st.push_back(x);
  if(!xx) break;
  x=xx;y=yy;
 }
 int cur=st.back();st.pop_back();update(cur);
 while(!st.empty()){int p=st.back();st.pop_back();insert(a[p].heap,cur);update(p);cur=p;}
 return cur;
}
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int T;cin>>T;while(T--){
  int n;cin>>n;vector<int> head(n+1,-1),to(2*max(0,n-1)),nxt(to.size());int e=0;
  auto add=[&](int u,int v){to[e]=v;nxt[e]=head[u];head[u]=e++;};
  for(int i=1,u,v;i<n;i++){cin>>u>>v;add(u,v);add(v,u);}
  a.assign(n+1,Node());a[0].rank=0;
  vector<int> parent(n+1),order;order.reserve(n);order.push_back(1);
  for(size_t i=0;i<order.size();i++){int u=order[i];for(int j=head[u];j!=-1;j=nxt[j])if(to[j]!=parent[u]){parent[to[j]]=u;order.push_back(to[j]);}}
  for(int i=n-1;i>0;i--){int v=order[i];update(v);insert(a[parent[v]].heap,v);}update(1);
  int root=1,ans=0;
  while(a[root].h){
   int top=a[root].heap;
   int sec=max(a[a[top].l].h+(a[top].l!=0),a[a[top].r].h+(a[top].r!=0));
   int H=a[top].h+1;
   if(H>sec+1){int v=pop(a[root].heap);update(root);insert(a[v].heap,root);update(v);root=v;continue;}
   if(H==sec){int x=pop(a[root].heap),y=pop(a[root].heap);int z=mergeBranches(x,y);insert(a[root].heap,z);update(root);}
   else {int v=pop(a[root].heap);update(root);root=mergeBranches(root,v);}
   ans++;
  }
  cout<<ans<<'\n';
 }
}

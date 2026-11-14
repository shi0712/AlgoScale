#include <bits/stdc++.h>
using namespace std;
using U = uint64_t;
struct Node { U key, hash; int a,b; };
vector<Node> tr(1);
U mix(U x){ x+=0x9e3779b97f4a7c15ULL; x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL; x=(x^(x>>27))*0x94d049bb133111ebULL; return x^(x>>31); }
int bit(U x){return 63-__builtin_clzll(x);}
int makeleaf(U k){tr.push_back({k,mix(k),1,0}); return tr.size()-1;}
int add(int p,U k){
 if(!p) return makeleaf(k);
 Node v=tr[p];
 if(!v.b && v.key==k){v.a++; v.hash+=mix(k); tr.push_back(v); return tr.size()-1;}
 int diff=v.key==k?-1:bit(v.key^k);
 int branch=v.b?bit(tr[v.a].key^tr[v.b].key):-1;
 if(diff>branch){int q=makeleaf(k); Node w; w.key=max(v.key,k); w.hash=v.hash+mix(k); if((k>>diff)&1) w.a=p,w.b=q; else w.a=q,w.b=p; tr.push_back(w); return tr.size()-1;}
 if((k>>branch)&1) v.b=add(v.b,k); else v.a=add(v.a,k);
 v.key=max(tr[v.a].key,tr[v.b].key); v.hash=tr[v.a].hash+tr[v.b].hash; tr.push_back(v); return tr.size()-1;
}
int upper(int p,int b){ if(!p)return 0; Node &v=tr[p]; if(v.b && bit(tr[v.a].key^tr[v.b].key)==b) return v.b; return ((v.key>>b)&1)?p:0; }
int lower(int p,int b){ if(!p)return 0; Node &v=tr[p]; if(v.b && bit(tr[v.a].key^tr[v.b].key)==b) return v.a; return ((v.key>>b)&1)?0:p; }
struct Value {int root; U extra;};
int cmp(Value x,Value y){
 int a=x.root,b=y.root; U ex=x.extra,ey=y.extra;
 if(a==b && ex==ey)return 0;
 U mx=max(max(tr[a].key,tr[b].key),max(ex,ey)); if(!mx)return 0;
 U wx=ex?mix(ex):0, wy=ey?mix(ey):0;
 for(int k=bit(mx);k>=0;--k){
  int au=upper(a,k),bu=upper(b,k);
  U hx=tr[au].hash+((ex && ((ex>>k)&1))?wx:0);
  U hy=tr[bu].hash+((ey && ((ey>>k)&1))?wy:0);
  if(hx!=hy){a=au;b=bu;if(ex && !((ex>>k)&1))ex=0;if(ey && !((ey>>k)&1))ey=0;}
  else {a=lower(a,k);b=lower(b,k);if(ex && ((ex>>k)&1))ex=0;if(ey && ((ey>>k)&1))ey=0;}
 }
 int cx=(a?tr[a].a:0)+(ex!=0), cy=(b?tr[b].a:0)+(ey!=0);
 return (cx>cy)-(cx<cy);
}
struct Candidate {int root; U start;};
struct Hull {
 int lo,hi; vector<int> tree; vector<Candidate> cand; vector<U>* xs;
 void init(int l,int r,vector<U>* p){lo=l;hi=r;xs=p;tree.assign(4*(r-l+1)+4,-1);cand.clear();}
 Value val(int id,int pos){U x=(*xs)[pos],s=cand[id].start;return {cand[id].root,x>s?x-s:0};}
 bool better(int a,int b,int pos){return cmp(val(a,pos),val(b,pos))>0;}
 void insert(Candidate c){cand.push_back(c); put(1,lo,hi,(int)cand.size()-1);}
 void put(int p,int l,int r,int id){if(tree[p]<0){tree[p]=id;return;}int mid=(l+r)/2;bool left=better(id,tree[p],l), middle=better(id,tree[p],mid);if(middle)swap(id,tree[p]);if(l==r)return;if(left!=middle)put(p*2,l,mid,id);else put(p*2+1,mid+1,r,id);}
 Value query(int pos){int p=1,l=lo,r=hi;Value best={0,0};while(true){if(tree[p]>=0){Value v=val(tree[p],pos);if(cmp(v,best)>0)best=v;}if(l==r)break;int m=(l+r)/2;if(pos<=m)p*=2,r=m;else p=p*2+1,l=m+1;}return best;}
};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);
 int T;cin>>T;while(T--){int n,m;cin>>n>>m;vector<pair<U,U>> a[2]; vector<U> xs;for(int c=0;c<2;c++){int z=c?m:n;a[c].resize(z);for(auto &p:a[c]){cin>>p.first>>p.second;++p.second;xs.push_back(p.first);xs.push_back(p.second);}}sort(xs.begin(),xs.end());xs.erase(unique(xs.begin(),xs.end()),xs.end());
 tr.clear();tr.push_back({0,0,0,0});int root=0,idx[2]={0,0};bool active[2]={false,false};Hull hull[2];
 for(int i=0;i<(int)xs.size();i++){
  Value best={root,0};for(int c=0;c<2;c++)if(active[c]){Value v=hull[c].query(i);if(cmp(v,best)>0)best=v;}
  root=best.extra?add(best.root,best.extra):best.root;
  for(int c=0;c<2;c++){
   if(active[c] && xs[i]==a[c][idx[c]].second){active[c]=false;idx[c]++;}
   if(idx[c]<(int)a[c].size() && xs[i]==a[c][idx[c]].first){active[c]=true;int e=lower_bound(xs.begin(),xs.end(),a[c][idx[c]].second)-xs.begin();hull[c].init(i,e,&xs);}
   if(active[c])hull[c].insert({root,xs[i]});
  }
 }
 vector<pair<U,int>> ans;function<void(int)> dfs=[&](int p){if(!p)return;if(!tr[p].b){ans.push_back({tr[p].key,tr[p].a});return;}dfs(tr[p].b);dfs(tr[p].a);};dfs(root);long long count=0;for(auto [k,v]:ans)count+=v;cout<<count<<'\n';bool first=true;for(auto [k,v]:ans)while(v--){if(!first)cout<<' ';first=false;cout<<k;}cout<<'\n';
 }
}

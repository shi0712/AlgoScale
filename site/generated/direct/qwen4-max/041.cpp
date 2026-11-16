#include <bits/stdc++.h>
using namespace std;

int n;
struct Op{int p[4];int e[3][2];bool isPath;int q[4];};
static Op makePath(int x,int y,int z,int w,int a,int b,int c,int d){Op o;o.p[0]=x;o.p[1]=y;o.p[2]=z;o.p[3]=w;o.q[0]=a;o.q[1]=b;o.q[2]=c;o.q[3]=d;o.e[0][0]=a;o.e[0][1]=b;o.e[1][0]=b;o.e[1][1]=c;o.e[2][0]=c;o.e[2][1]=d;o.isPath=true;return o;}
static Op makeStar(int x,int y,int z,int w,int cen){Op o;o.p[0]=x;o.p[1]=y;o.p[2]=z;o.p[3]=w;int arr[4]={x,y,z,w};int k=0;for(int i=0;i<4;i++)if(arr[i]!=cen){o.e[k][0]=cen;o.e[k][1]=arr[i];++k;}o.isPath=false;o.q[0]=o.q[1]=o.q[2]=o.q[3]=0;return o;}
static Op invertOp(const Op&o){Op r;for(int i=0;i<4;i++){r.p[i]=o.q[i];r.q[i]=o.p[i];}r.e[0][0]=o.p[0];r.e[0][1]=o.p[1];r.e[1][0]=o.p[1];r.e[1][1]=o.p[2];r.e[2][0]=o.p[2];r.e[2][1]=o.p[3];r.isPath=true;return r;}
struct Tree{bool g[105][105];Tree(){memset(g,0,sizeof g);}void add(int a,int b){g[a][b]=g[b][a]=true;}void del(int a,int b){g[a][b]=g[b][a]=false;}
 void apply(const Op&o){del(o.p[0],o.p[1]);del(o.p[1],o.p[2]);del(o.p[2],o.p[3]);for(int i=0;i<3;i++)add(o.e[i][0],o.e[i][1]);}
 int deg(int v)const{int d=0;for(int u=1;u<=n;u++)if(g[v][u])++d;return d;}};
static void gather(Tree&t,int c,bool useStar,vector<Op>&ops){
 while(true){int par[105],dep[105];for(int i=1;i<=n;i++){par[i]=0;dep[i]=-1;}
  vector<int>ord;ord.reserve(n);dep[c]=0;ord.push_back(c);
  for(size_t i=0;i<ord.size();i++){int u=ord[i];for(int v=1;v<=n;v++)if(t.g[u][v]&&dep[v]<0){dep[v]=dep[u]+1;par[v]=u;ord.push_back(v);}}
  int maxd=0,cnt=0;for(int v=1;v<=n;v++){maxd=max(maxd,dep[v]);if(dep[v]>=2)cnt++;}
  if(maxd<=2&&cnt<=1)break;
  if(maxd>=3){int w=-1;for(int v=1;v<=n;v++)if(dep[v]==3){w=v;break;}int z=par[w],y=par[z];
   Op o=(useStar&&cnt>=3)?makeStar(c,y,z,w,c):makePath(c,y,z,w,w,c,y,z);t.apply(o);ops.push_back(o);
  }else if(useStar){int z=-1,y=-1,x=-1;for(int v=1;v<=n;v++)if(dep[v]==2){z=v;y=par[v];break;}
   for(int v=1;v<=n;v++)if(dep[v]==1&&v!=y){x=v;break;}
   Op o=makeStar(x,c,y,z,c);t.apply(o);ops.push_back(o);
  }else{int v=-1,u=-1;for(int s=1;s<=n;s++)if(dep[s]==2){if(v<0)v=s;else{u=s;break;}}
   int y=par[v],x=par[u];
   if(y!=x){Op o1=makePath(u,x,c,y,x,c,u,y);t.apply(o1);ops.push_back(o1);
            Op o2=makePath(c,u,y,v,v,c,u,y);t.apply(o2);ops.push_back(o2);}
   else{int xp=-1;for(int s=1;s<=n;s++)if(dep[s]==1&&s!=y){xp=s;break;}
        Op o1=makePath(v,y,c,xp,y,v,c,xp);t.apply(o1);ops.push_back(o1);
        Op o2=makePath(c,v,y,u,u,c,v,y);t.apply(o2);ops.push_back(o2);}}}}
static void findTail(const Tree&t,int c,int&t1,int&t2){t1=t2=-1;
 for(int v=1;v<=n&&t1<0;v++)if(t.g[c][v]){for(int u=1;u<=n;u++)if(t.g[v][u]&&u!=c){t1=v;t2=u;break;}}}
static int findLeafAt(const Tree&t,int c,int e1,int e2){for(int v=1;v<=n;v++)if(t.g[c][v]&&v!=e1&&v!=e2&&t.deg(v)==1)return v;return -1;}
static void relabel(Tree&t,int c,vector<Op>&ops){int t1,t2;findTail(t,c,t1,t2);
 if(t1!=2){Op o;if(t2==2){int x=findLeafAt(t,c,t1,t2);o=makePath(x,c,t1,t2,x,c,t2,t1);}else o=makePath(2,c,t1,t2,t2,2,c,t1);
  t.apply(o);ops.push_back(o);findTail(t,c,t1,t2);}
 if(t2!=3){Op o=makePath(3,c,t1,t2,t2,c,t1,3);t.apply(o);ops.push_back(o);}}
int main(){ios::sync_with_stdio(false);cin.tie(0);
 if(!(cin>>n))return 0;Tree S,T;
 for(int i=0;i<n-1;i++){int a,b;cin>>a>>b;S.add(a,b);}
 for(int i=0;i<n-1;i++){int a,b;cin>>a>>b;T.add(a,b);}
 bool same=true;for(int i=1;i<=n;i++)for(int j=1;j<=n;j++)if(S.g[i][j]!=T.g[i][j])same=false;
 if(same){cout<<"YES\n0\n";return 0;}
 int sStar=-1,tStar=-1;
 for(int v=1;v<=n;v++){if(S.deg(v)==n-1)sStar=v;if(T.deg(v)==n-1)tStar=v;}
 if(sStar>0){cout<<"NO\n";return 0;}
 vector<Op>res;
 if(tStar>0){Tree cur=S;gather(cur,tStar,true,res);int t1,t2;findTail(cur,tStar,t1,t2);
  int x=findLeafAt(cur,tStar,t1,t2);Op o=makeStar(x,tStar,t1,t2,tStar);cur.apply(o);res.push_back(o);
 }else{Tree cur=S;gather(cur,1,true,res);relabel(cur,1,res);
  Tree cur2=T;vector<Op>opsT;gather(cur2,1,false,opsT);relabel(cur2,1,opsT);
  for(int i=(int)opsT.size()-1;i>=0;i--)res.push_back(invertOp(opsT[i]));}
 cout<<"YES\n"<<res.size()<<"\n";
 for(auto&o:res){cout<<o.p[0]<<" "<<o.p[1]<<" "<<o.p[2]<<" "<<o.p[3]<<"\n";
  cout<<o.e[0][0]<<" "<<o.e[0][1]<<" "<<o.e[1][0]<<" "<<o.e[1][1]<<" "<<o.e[2][0]<<" "<<o.e[2][1]<<"\n";}
 return 0;}

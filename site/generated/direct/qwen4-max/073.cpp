#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int R,C,N;vector<string> g;
static const int DR[4]={-1,0,1,0},DC[4]={0,1,0,-1};
vector<int> dst[4],nxt[4],compOf,clsOf,compCenter,belief;vector<char> hopOf;
int ax=100,ay=100,adir=0;static bool vis[205][205];bool inNav=false;
int readObs(){int d;if(!(cin>>d))exit(0);return d;}
void sendCmd(const char*s){cout<<s<<'\n';cout.flush();}
void answerYes(int c){cout<<"yes "<<(c/C+1)<<" "<<(c%C+1)<<'\n';cout.flush();exit(0);}
void answerNo(){cout<<"no\n";cout.flush();exit(0);}
void doTurn(int dl);void doStep();
void navigateTo(int st,int tg){vector<int>par(4*N,-1);vector<unsigned char>ac(4*N,0);vector<int>q;q.reserve(4*N);q.push_back(st);par[st]=st;int gl=-1;
 for(size_t h=0;h<q.size();h++){int u=q[h];if((u>>2)==tg){gl=u;break;}int ce=u>>2,d=u&3;int v=(ce<<2)|((d+3)&3);if(par[v]<0){par[v]=u;ac[v]=0;q.push_back(v);}v=(ce<<2)|((d+1)&3);if(par[v]<0){par[v]=u;ac[v]=1;q.push_back(v);}if(dst[d][ce]>0){v=(nxt[d][ce]<<2)|d;if(par[v]<0){par[v]=u;ac[v]=2;q.push_back(v);}}}
 if(gl<0)answerNo();vector<int>as;for(int u=gl;u!=st;u=par[u])as.push_back(ac[u]);reverse(as.begin(),as.end());inNav=true;for(int a:as){if(a==0)doTurn(-1);else if(a==1)doTurn(1);else doStep();}inNav=false;}
void afterObs(int d){int m=0,c0=-1,k0=-1;bool sm=true,ah=true,sg=true;
 for(size_t k=0;k<belief.size();k++){int s=belief[k];if(dst[s&3][s>>2]!=d)continue;belief[m++]=s;int ce=s>>2;if(c0<0){c0=ce;k0=clsOf[s];}else{if(ce!=c0)sm=false;if(clsOf[s]!=k0)sg=false;}if(!hopOf[s])ah=false;}
 belief.resize(m);if(!m)exit(0);if(inNav)return;if(sm)answerYes(c0);if(ah)answerNo();
 if(sg){int tg=compCenter[compOf[c0]];if(tg<0)answerNo();navigateTo(belief[0],tg);answerYes(tg);}}
void doTurn(int dl){sendCmd(dl>0?"right":"left");adir=(adir+dl)&3;for(int&s:belief)s=(s&~3)|((s+dl)&3);afterObs(readObs());}
void doStep(){sendCmd("step");ax+=DR[adir];ay+=DC[adir];for(int&s:belief){int d=s&3;s=(nxt[d][s>>2]<<2)|d;}afterObs(readObs());}
void turnTo(int d){int f=(d-adir)&3;if(f==1)doTurn(1);else if(f==2){doTurn(1);doTurn(1);}else if(f==3)doTurn(-1);}
int commonDist(int d){int r=(d-adir)&3,v=-1;for(int s:belief){int dd=dst[(s+r)&3][s>>2];if(v<0)v=dd;else if(v!=dd)return -1;}return v;}
void eager(){int st=adir;for(int t=0;t<4;t++){int d=(st+t)&3;if(commonDist(d)<0)turnTo(d);}}
void explore(){struct F{int x,y,ad,idx;};vector<F> st;vis[ax][ay]=true;eager();st.push_back({ax,ay,adir,0});static const int off[4]={0,1,3,2};
 while(!st.empty()){int x=st.back().x,y=st.back().y,ad=st.back().ad,idx=st.back().idx;
  if(idx>=4){st.pop_back();if(st.empty())break;turnTo((ad+2)&3);doStep();continue;}
  st.back().idx++;int d=(ad+off[idx])&3,nx=x+DR[d],ny=y+DC[d];if(vis[nx][ny])continue;
  int v=commonDist(d);if(v<0){turnTo(d);v=commonDist(d);}if(v<=0)continue;turnTo(d);doStep();vis[nx][ny]=true;eager();st.push_back({nx,ny,d,0});}}
auto rotrel=[](int r,ll x,ll y)->pair<ll,ll>{r&=3;if(r==0)return make_pair(x,y);if(r==1)return make_pair(y,-x);if(r==2)return make_pair(-x,-y);return make_pair(-y,x);};
int main(){ios::sync_with_stdio(false);cin.tie(0);cin>>R>>C;g.resize(R);for(auto&s:g)cin>>s;N=R*C;
 for(int d=0;d<4;d++){dst[d].assign(N,0);nxt[d].assign(N,-1);}
 for(int i=0;i<R;i++)for(int j=0;j<C;j++){if(g[i][j]!='.')continue;int id=i*C+j;
  if(i==0||g[i-1][j]!='.'){dst[0][id]=0;}else{dst[0][id]=dst[0][id-C]+1;nxt[0][id]=id-C;}
  if(j==0||g[i][j-1]!='.'){dst[3][id]=0;}else{dst[3][id]=dst[3][id-1]+1;nxt[3][id]=id-1;}}
 for(int i=R-1;i>=0;i--)for(int j=C-1;j>=0;j--){if(g[i][j]!='.')continue;int id=i*C+j;
  if(i==R-1||g[i+1][j]!='.'){dst[2][id]=0;}else{dst[2][id]=dst[2][id+C]+1;nxt[2][id]=id+C;}
  if(j==C-1||g[i][j+1]!='.'){dst[1][id]=0;}else{dst[1][id]=dst[1][id+1]+1;nxt[1][id]=id+1;}}
 compOf.assign(N,-1);vector<vector<int>> comps;
 for(int id=0;id<N;id++){if(g[id/C][id%C]!='.'||compOf[id]>=0)continue;int ci=comps.size();comps.push_back({});vector<int> stk{id};compOf[id]=ci;
  while(!stk.empty()){int u=stk.back();stk.pop_back();comps[ci].push_back(u);int i=u/C,j=u%C;
   for(int d=0;d<4;d++){int ni=i+DR[d],nj=j+DC[d];if(ni<0||ni>=R||nj<0||nj>=C)continue;int v=ni*C+nj;if(g[ni][nj]=='.'&&compOf[v]<0){compOf[v]=ci;stk.push_back(v);}}}}
 int NC=comps.size();compCenter.assign(NC,-1);vector<int> ck(NC,1),cbr(NC,0),csh(NC,0),ccnt(NC,0);vector<ll> cCX(NC,0),cCY(NC,0);
 map<vector<pair<int,int>>,int> smap;vector<int> scnt;
 for(int ci=0;ci<NC;ci++){auto&cl=comps[ci];int cnt=cl.size();ccnt[ci]=cnt;ll sr=0,sc=0;for(int id:cl){sr+=id/C;sc+=id%C;}
  ll CX=2*sr,CY=2*sc;cCX[ci]=CX;cCY[ci]=CY;int k=1;
  if((2*sr)%cnt==0&&(2*sc)%cnt==0){int cx=(int)(2*sr/cnt),cy=(int)(2*sc/cnt);bool s2=true,s4=true,pr=((cx-cy)%2==0);
   for(int id:cl){int i=id/C,j=id%C;int ni=cx-i,nj=cy-j;if(ni<0||ni>=R||nj<0||nj>=C||compOf[ni*C+nj]!=ci)s2=false;
    if(pr){int ai=(cx-cy)/2+j,aj=(cx+cy)/2-i;if(ai<0||ai>=R||aj<0||aj>=C||compOf[ai*C+aj]!=ci)s4=false;}else s4=false;}
   if(s4)k=4;else if(s2)k=2;
   if(cx%2==0&&cy%2==0){int i=cx/2,j=cy/2;if(i>=0&&i<R&&j>=0&&j<C&&compOf[i*C+j]==ci)compCenter[ci]=i*C+j;}}
  ck[ci]=k;vector<pair<ll,ll>> best;int br=0;bool fi=true;
  for(int r=0;r<4;r++){vector<pair<ll,ll>> pts;pts.reserve(cnt);for(int id:cl){ll x=2LL*(id/C)*cnt-CX,y=2LL*(id%C)*cnt-CY;pts.push_back(rotrel(r,x,y));}
   ll mx=LLONG_MAX,my=LLONG_MAX;for(auto&p:pts){mx=min(mx,p.first);my=min(my,p.second);}for(auto&p:pts){p.first-=mx;p.second-=my;}sort(pts.begin(),pts.end());
   if(fi||pts<best){best=pts;br=r;fi=false;}}
  cbr[ci]=br;vector<pair<int,int>> key;key.reserve(cnt);for(auto&p:best)key.push_back({(int)p.first,(int)p.second});
  auto it=smap.find(key);int sid;if(it==smap.end()){sid=scnt.size();smap.emplace(move(key),sid);scnt.push_back(0);}else sid=it->second;csh[ci]=sid;scnt[sid]++;}
 clsOf.assign(4*N,-1);hopOf.assign(4*N,0);map<tuple<int,ll,ll,int>,int> cmap;
 for(int ci=0;ci<NC;ci++){int cnt=ccnt[ci],k=ck[ci],br=cbr[ci],sid=csh[ci];ll CX=cCX[ci],CY=cCY[ci];
  bool hop=(scnt[sid]>1)||(k>1&&compCenter[ci]<0);int js[4],nj;
  if(k==1){js[0]=0;nj=1;}else if(k==2){js[0]=0;js[1]=2;nj=2;}else{js[0]=0;js[1]=1;js[2]=2;js[3]=3;nj=4;}
  for(int id:comps[ci]){ll x=2LL*(id/C)*cnt-CX,y=2LL*(id%C)*cnt-CY;auto p0=rotrel(br,x,y);
   for(int d=0;d<4;d++){int dd=(d+br)&3;ll bx=p0.first,by=p0.second;int bd=dd;
    for(int t=0;t<nj;t++){int j=js[t];auto q=rotrel(j,p0.first,p0.second);int qd=(dd+j)&3;
     if(make_tuple(q.first,q.second,qd)<make_tuple(bx,by,bd)){bx=q.first;by=q.second;bd=qd;}}
    auto key=make_tuple(sid,bx,by,bd);auto it=cmap.find(key);int cid;
    if(it==cmap.end()){cid=cmap.size();cmap.emplace(key,cid);}else cid=it->second;
    int s=(id<<2)|d;clsOf[s]=cid;hopOf[s]=hop?1:0;}}}
 belief.reserve(4*N);for(int id=0;id<N;id++)if(g[id/C][id%C]=='.')for(int d=0;d<4;d++)belief.push_back((id<<2)|d);
 afterObs(readObs());for(int t=0;t<3;t++)doTurn(1);explore();answerNo();return 0;}

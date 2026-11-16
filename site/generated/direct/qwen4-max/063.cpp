#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const int MAXV=502,MAXA=3005,BIGK=(1<<29),INFD=1000000000;
static char ibuf[1<<16];static int ipos=0,ilen=0;
static inline int gc(){if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;}return (unsigned char)ibuf[ipos++];}
static inline ll rd(){int c=gc();while(c<'0'||c>'9'){if(c==-1)return 0;c=gc();}ll x=0;while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}return x;}
static char obuf[1<<16];static int opos=0;
static inline void flushOut(){fwrite(obuf,1,opos,stdout);opos=0;}
static inline void wc(char c){if(opos==(int)sizeof(obuf))flushOut();obuf[opos++]=c;}
static inline void wr(ll x){char t[24];int k=0;if(!x)t[k++]='0';while(x){t[k++]=(char)('0'+x%10);x/=10;}if(opos+k>(int)sizeof(obuf))flushOut();while(k)obuf[opos++]=t[--k];}
static int n,m,V,S,T;static ll K,D;
static int rw[505],cw[505];static bool keepH[505],keepV[505],kept[MAXV];
static ll d1[MAXV],d2[MAXV],pot[MAXV];
static int aTo[MAXA],aRev[MAXA],aKey[MAXA],aW2[MAXA];static signed char aFl[MAXA];
static int st[MAXV+1],fillp[MAXV],level_[MAXV],iter_[MAXV],q_[MAXV],dist_[MAXV],inc_[MAXV];
static int bhead[105],bval[MAXA+10],bnxt[MAXA+10],bcnt;
static pair<ll,int> hp[4200];static int hn;
static void hpush(ll dd,int v){int i=hn++;while(i>0){int p=(i-1)>>1;if(hp[p].first<=dd)break;hp[i]=hp[p];i=p;}hp[i]=make_pair(dd,v);}
static pair<ll,int> hpop(){pair<ll,int> r=hp[0];--hn;if(hn>0){ll dd=hp[hn].first;int v=hp[hn].second;int i=0;for(;;){int c=2*i+1;if(c>=hn)break;if(c+1<hn&&hp[c+1].first<hp[c].first)++c;if(hp[c].first>=dd)break;hp[i]=hp[c];i=c;}hp[i]=make_pair(dd,v);}return r;}
static void gridDij(ll*dist,bool left){int NM=n*m;for(int v=0;v<NM;v++)dist[v]=LLONG_MAX/4;hn=0;for(int i=0;i<n;i++){int v=i*m+(left?0:m-1);dist[v]=0;hpush(0,v);}while(hn){pair<ll,int> tp=hpop();ll dd=tp.first;int u=tp.second;if(dd!=dist[u])continue;int i=u/m,j=u-i*m;
 if(j>0){ll nd=dd+rw[i*(m-1)+j-1];int v=u-1;if(nd<dist[v]){dist[v]=nd;hpush(nd,v);}}
 if(j+1<m){ll nd=dd+rw[i*(m-1)+j];int v=u+1;if(nd<dist[v]){dist[v]=nd;hpush(nd,v);}}
 if(i>0){ll nd=dd+cw[(i-1)*m+j];int v=u-m;if(nd<dist[v]){dist[v]=nd;hpush(nd,v);}}
 if(i+1<n){ll nd=dd+cw[i*m+j];int v=u+m;if(nd<dist[v]){dist[v]=nd;hpush(nd,v);}}}}
static inline void addUnd(int u,int v,int w){int a=fillp[u]++,b=fillp[v]++;aTo[a]=v;aRev[a]=b;aTo[b]=u;aRev[b]=a;aW2[a]=aW2[b]=2*w;aFl[a]=aFl[b]=0;aKey[a]=(int)((ll)w+pot[u]-pot[v]);aKey[b]=(int)((ll)w+pot[v]-pot[u]);}
static inline void addST(int u,int v){int a=fillp[u]++,b=fillp[v]++;aTo[a]=v;aRev[a]=b;aTo[b]=u;aRev[b]=a;aW2[a]=aW2[b]=0;aFl[a]=aFl[b]=0;aKey[a]=(int)(pot[u]-pot[v]);aKey[b]=BIGK;}
static bool bfs(){memset(level_,-1,V*sizeof(int));int qh=0,qt=0;q_[qt++]=S;level_[S]=0;while(qh<qt){int u=q_[qh++],nl=level_[u]+1;for(int a=st[u],e=st[u+1];a<e;a++){if(aKey[a]!=0)continue;int v=aTo[a];if(level_[v]<0){level_[v]=nl;if(v==T)return true;q_[qt++]=v;}}}return false;}
static bool dfs(int u){if(u==T)return true;int lu=level_[u]+1;for(int &a=iter_[u];a<st[u+1];++a){if(aKey[a]!=0)continue;int v=aTo[a];if(level_[v]!=lu)continue;if(dfs(v)){int b=aRev[a];if(u==S||v==T){aKey[b]=0;}else if(aFl[a]==0){aFl[a]=1;aFl[b]=-1;aKey[a]=BIGK;aKey[b]=0;}else{aFl[a]=0;aFl[b]=0;aKey[a]=aW2[a];aKey[b]=0;}return true;}}return false;}
static void solve(){for(;;){while(bfs()){memcpy(iter_,st,V*sizeof(int));while(dfs(S)){}}
 int delta=(int)(D-pot[T]);for(int v=0;v<V;v++)dist_[v]=INFD;for(int i=0;i<=delta;i++)bhead[i]=-1;bcnt=0;
 dist_[S]=0;bval[bcnt]=S;bnxt[bcnt]=bhead[0];bhead[0]=bcnt++;int theta=delta;bool reached=false;
 for(int dd=0;dd<=delta&&!reached;dd++){while(bhead[dd]>=0){int nd0=bhead[dd];bhead[dd]=bnxt[nd0];int u=bval[nd0];if(dist_[u]!=dd)continue;if(u==T){theta=dd;reached=true;break;}int budget=delta-dd;
   for(int a=st[u],e=st[u+1];a<e;a++){int r=aKey[a];if(r<=budget){int v=aTo[a];int nd=dd+r;if(nd<dist_[v]){dist_[v]=nd;bval[bcnt]=v;bnxt[bcnt]=bhead[nd];bhead[nd]=bcnt++;}}}}}
 for(int v=0;v<V;v++)inc_[v]=dist_[v]<theta?dist_[v]:theta;
 for(int u=0;u<V;u++){int iu=inc_[u];pot[u]+=iu;for(int a=st[u],e=st[u+1];a<e;a++)aKey[a]+=iu-inc_[aTo[a]];}
 if(!reached||theta==delta)break;}}
int main(){int tc=(int)rd();while(tc--){n=(int)rd();m=(int)rd();K=rd();int nh=n*(m-1),nv=(n-1)*m;
 for(int i=0;i<nh;i++)rw[i]=(int)rd();for(int i=0;i<nv;i++)cw[i]=(int)rd();
 gridDij(d1,true);gridDij(d2,false);ll d=LLONG_MAX/4;for(int i=0;i<n;i++)d=min(d,d1[i*m+m-1]);D=d+K;int NM=n*m;
 for(int v=0;v<NM;v++)kept[v]=(d1[v]+d2[v]<D);
 V=NM+2;S=NM;T=NM+1;for(int v=0;v<V;v++)fillp[v]=0;
 for(int i=0;i<n;i++)for(int j=0;j+1<m;j++){int idx=i*(m-1)+j,u=i*m+j,v=u+1;ll w=rw[idx];bool kp=(d1[u]+w+d2[v]<D)||(d1[v]+w+d2[u]<D);keepH[idx]=kp;if(kp){fillp[u]++;fillp[v]++;}}
 for(int i=0;i+1<n;i++)for(int j=0;j<m;j++){int idx=i*m+j,u=i*m+j,v=u+m;ll w=cw[idx];bool kp=(d1[u]+w+d2[v]<D)||(d1[v]+w+d2[u]<D);keepV[idx]=kp;if(kp){fillp[u]++;fillp[v]++;}}
 for(int i=0;i<n;i++){int u=i*m,v=i*m+m-1;if(kept[u]){fillp[S]++;fillp[u]++;}if(kept[v]){fillp[v]++;fillp[T]++;}}
 st[0]=0;for(int v=0;v<V;v++){st[v+1]=st[v]+fillp[v];fillp[v]=st[v];}
 for(int v=0;v<NM;v++)pot[v]=kept[v]?d1[v]:0;pot[S]=0;pot[T]=d;
 for(int i=0;i<n;i++)for(int j=0;j+1<m;j++){int idx=i*(m-1)+j;if(keepH[idx])addUnd(i*m+j,i*m+j+1,rw[idx]);}
 for(int i=0;i+1<n;i++)for(int j=0;j<m;j++){int idx=i*m+j;if(keepV[idx])addUnd(i*m+j,i*m+j+m,cw[idx]);}
 for(int i=0;i<n;i++){if(kept[i*m])addST(S,i*m);if(kept[i*m+m-1])addST(i*m+m-1,T);}
 solve();
 ll ans=0;
 for(int i=0;i<n;i++)for(int j=0;j+1<m;j++){int idx=i*(m-1)+j;if(keepH[idx]){int u=i*m+j,v=u+1;ll df=pot[u]-pot[v];if(df<0)df=-df;if(df>rw[idx]){ans+=df-rw[idx];rw[idx]=(int)df;}}}
 for(int i=0;i+1<n;i++)for(int j=0;j<m;j++){int idx=i*m+j;if(keepV[idx]){int u=i*m+j,v=u+m;ll df=pot[u]-pot[v];if(df<0)df=-df;if(df>cw[idx]){ans+=df-cw[idx];cw[idx]=(int)df;}}}
 wr(ans);wc('\n');
 for(int i=0;i<n;i++){for(int j=0;j+1<m;j++){if(j)wc(' ');wr(rw[i*(m-1)+j]);}wc('\n');}
 for(int i=0;i+1<n;i++){for(int j=0;j<m;j++){if(j)wc(' ');wr(cw[i*m+j]);}wc('\n');}}
 flushOut();return 0;}

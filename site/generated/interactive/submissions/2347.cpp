#include <bits/stdc++.h>
using namespace std;
int n;
int ask(const string&s){cout<<s<<"\n";cout.flush();int r;if(!(cin>>r))exit(0);if(r==-1)exit(0);if(r==n)exit(0);return r;}
long long nk(int k){return k==0?0:(long long)k<<(k-1);}
vector<vector<char>> build(int k){
    if(k==0)return {};
    auto M=build(k-1);int m=(1<<(k-1))-1;int nn=nk(k-1);int N=2*nn+m+1;
    vector<vector<char>> R;
    for(int j=0;j<m;j++){vector<char> r(N,0);for(int i=0;i<nn;i++){r[i]=M[j][i];r[nn+i]=M[j][i];}r[2*nn+j]=1;R.push_back(r);}
    for(int j=0;j<m;j++){vector<char> r(N,0);for(int i=0;i<nn;i++){r[i]=M[j][i];r[nn+i]=1-M[j][i];}r[2*nn+m]=1;R.push_back(r);}
    {vector<char> r(N,0);for(int i=0;i<nn;i++)r[nn+i]=1;r[2*nn+m]=1;R.push_back(r);}
    return R;
}
bool dec(int k,const vector<long long>&v,vector<char>&out){
    out.clear();if(k==0)return true;
    int m=(1<<(k-1))-1;int nn=nk(k-1);long long c=v[2*m];
    vector<long long> My(m),Mx(m);vector<char> z(m);
    for(int j=0;j<m;j++){long long t=v[j]-v[m+j]+c;int zz=((t%2)+2)%2;z[j]=zz;My[j]=(t-zz)/2;Mx[j]=v[j]-My[j]-zz;}
    vector<char> x,y;
    if(!dec(k-1,My,y))return false;
    if(!dec(k-1,Mx,x))return false;
    long long Y=0;for(char b:y)Y+=b;
    long long w=c-Y;if(w<0||w>1)return false;
    for(int j=0;j<m;j++){if(v[m+j]!=Mx[j]+Y-My[j]+w)return false;}
    out.reserve(2*nn+m+1);
    for(char b:x)out.push_back(b);for(char b:y)out.push_back(b);for(char b:z)out.push_back(b);out.push_back((char)w);
    return true;
}
void finishCandidates(vector<vector<char>> cands){
    while(true){
        if(cands.empty())exit(0);
        int best=0;
        if(cands.size()>1&&cands.size()<=300){
            int bs=INT_MAX;
            for(size_t a=0;a<cands.size();a++){
                map<int,int> cnt;
                for(size_t b=0;b<cands.size();b++){if(a==b)continue;int mt=0;for(int i=0;i<n;i++)if(cands[a][i]==cands[b][i])mt++;cnt[mt]++;}
                int mx=0;for(auto&p:cnt)mx=max(mx,p.second);
                if(mx<bs){bs=mx;best=a;}
            }
        }
        string s(n,'F');for(int i=0;i<n;i++)if(cands[best][i])s[i]='T';
        int r=ask(s);
        vector<vector<char>> nc;
        for(size_t b=0;b<cands.size();b++){if((int)b==best)continue;int mt=0;for(int i=0;i<n;i++)if(cands[best][i]==cands[b][i])mt++;if(mt==r)nc.push_back(cands[b]);}
        cands=nc;
    }
}
int main(){
    if(!(cin>>n))return 0;
    if(n<=8){
        int F=1<<n;vector<int> cand(F);iota(cand.begin(),cand.end(),0);
        while(true){
            if(cand.size()==1){string s(n,'F');for(int i=0;i<n;i++)if(cand[0]>>i&1)s[i]='T';ask(s);return 0;}
            vector<char> isC(F,0);for(int c:cand)isC[c]=1;
            int bs=INT_MAX,bsh=0;bool bc=false;
            for(int s=0;s<F;s++){
                int cnt[10]={0};
                for(int c:cand){int mt=n-__builtin_popcount(s^c);if(mt!=n)cnt[mt]++;}
                int mx=0;for(int t=0;t<10;t++)mx=max(mx,cnt[t]);
                if(mx<bs||(mx==bs&&isC[s]&&!bc)){bs=mx;bsh=s;bc=isC[s];}
            }
            string s(n,'F');for(int i=0;i<n;i++)if(bsh>>i&1)s[i]='T';
            int r=ask(s);
            vector<int> nc;for(int c:cand){int mt=n-__builtin_popcount(bsh^c);if(mt==r)nc.push_back(c);}
            cand=nc;
        }
    }
    int K=2;
    while(true){int k=K-1;long long cap=2*nk(k)+((1<<k)-1);if(cap>=n)break;K++;}
    int k=K-1;int m=(1<<k)-1;int np=nk(k);
    auto M=build(k);
    int V=2*np+m;
    vector<long long> Va(m),Vb(m);
    for(int t=0;t<2;t++){
        for(int j=0;j<m;j++){
            vector<char> row(V,0);
            for(int i=0;i<np;i++){row[i]=M[j][i];row[np+i]=(t==0?M[j][i]:1-M[j][i]);}
            if(t==0)row[2*np+j]=1;
            string s(n,'F');int sz=0;
            for(int i=0;i<n;i++)if(row[i]){s[i]='T';sz++;}
            int r=ask(s);
            long long val=(long long)r-n+sz;
            if(t==0)Va[j]=val;else Vb[j]=val;
        }
    }
    vector<vector<char>> cands;
    vector<long long> D(m);for(int j=0;j<m;j++)D[j]=(Va[j]-Vb[j])/2;
    int par=((Va[0]%2)+2)%2;
    for(long long Y=0;Y<=np;Y++){
        vector<char> z(m);vector<long long> My(m);bool ok=true;
        for(int j=0;j<m;j++){long long t=D[j]+Y;int zz=((t%2)+2)%2;z[j]=zz;if(zz&&2*np+j>=n){ok=false;break;}My[j]=(t-zz)/2;}
        if(!ok)continue;
        vector<char> y;
        if(!dec(k,My,y))continue;
        long long sy=0;for(int i=0;i<np;i++){sy+=y[i];if(y[i]&&np+i>=n)ok=false;}
        if(!ok||sy!=Y)continue;
        long long Z=0;for(char b:z)Z+=b;
        for(long long U=par;U<=n;U+=2){
            vector<long long> Mx(m);
            for(int j=0;j<m;j++){long long A=(Va[j]+U)/2;Mx[j]=A-My[j]-z[j];}
            vector<char> x;
            if(!dec(k,Mx,x))continue;
            bool ok2=true;long long X=0;
            for(int i=0;i<np;i++){X+=x[i];if(x[i]&&i>=n)ok2=false;}
            if(!ok2)continue;
            if(X+Y+Z!=U)continue;
            for(int j=0;j<m;j++){long long B=(Vb[j]+U)/2;if(B!=Mx[j]+Y-My[j]){ok2=false;break;}}
            if(!ok2)continue;
            vector<char> u(n,0);
            for(int i=0;i<np;i++){if(i<n)u[i]=x[i];if(np+i<n)u[np+i]=y[i];}
            for(int j=0;j<m;j++)if(2*np+j<n)u[2*np+j]=z[j];
            cands.push_back(u);
            if(cands.size()>20000)break;
        }
        if(cands.size()>20000)break;
    }
    finishCandidates(cands);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

// codes: 0 dead,1 fresh,2 T1,3 T2,4 F1,5 F2
static int trans(int c, bool v){
    switch(c){
        case 1: return v?2:4;
        case 2: return v?3:4;
        case 3: return v?0:4;
        case 4: return v?2:5;
        case 5: return v?2:0;
    }
    return 0;
}
static double wt(int c){
    static const double phi=(1+sqrt(5.0))/2;
    switch(c){case 1:return 2;case 2:case 4:return phi;case 3:case 5:return 1;}
    return 0;
}

const int K=8;
static int8_t lbA[1679616];
static int8_t ubA[1679616];
static int16_t mvA[1679616];

static int encode(const vector<int>&d){
    int c=0,p=1;for(int x:d){c+=x*p;p*=6;}return c;
}

bool canWin(const vector<int>&s,int d){
    int k=s.size();
    if(k<=2) return true;
    int code=encode(s);
    if(ubA[code]<=d) return true;
    if(lbA[code]>d) return false;
    if(d==0){ if(lbA[code]<1)lbA[code]=1; return false;}
    vector<int> nx; nx.reserve(k);
    for(int i=0;i<k;i++)for(int j=i;j<k;j++){
        bool ok=true;
        for(int cl=0;cl<2&&ok;cl++){
            bool claimIn=cl;
            nx.clear();
            for(int t=0;t<k;t++){
                bool in=(t>=i&&t<=j);
                bool v=(in==claimIn);
                int c2=trans(s[t],v);
                if(c2)nx.push_back(c2);
            }
            if(nx.empty())continue;
            if(!canWin(nx,d-1))ok=false;
        }
        if(ok){
            ubA[code]=d; mvA[code]=i*16+j; return true;
        }
    }
    lbA[code]=d+1;
    return false;
}

vector<int> pos; vector<int> st;

bool ask(int l,int r){
    printf("? %d %d\n",l,r);fflush(stdout);
    long long cnt;
    if(scanf("%lld",&cnt)!=1) exit(0);
    if(cnt<0) exit(0);
    return cnt==(long long)(r-l);
}

void applyAns(int l,int r,bool claimIn){
    vector<int> np,ns;
    for(size_t t=0;t<pos.size();t++){
        bool in=(pos[t]>=l&&pos[t]<=r);
        bool v=(in==claimIn);
        int c2=trans(st[t],v);
        if(c2){np.push_back(pos[t]);ns.push_back(c2);}
    }
    pos=np;st=ns;
}

int main(){
    int n;
    if(scanf("%d",&n)!=1)return 0;
    memset(ubA,127,sizeof(ubA));
    memset(lbA,0,sizeof(lbA));
    for(int i=1;i<=n;i++){pos.push_back(i);st.push_back(1);}
    while(true){
        int k=pos.size();
        if(k<=2)break;
        if(k<=K){
            int d=0;
            while(!canWin(st,d))d++;
            int code=encode(st);
            int mv=mvA[code];
            int i=mv/16,j=mv%16;
            int l=pos[i],r=pos[j];
            bool c=ask(l,r);
            applyAns(l,r,c);
        } else {
            vector<double> P(k+1,0);
            double BT=0,BF=0;
            for(int t=0;t<k;t++){
                double fT=wt(trans(st[t],true)),fF=wt(trans(st[t],false));
                BT+=fT;BF+=fF;
                P[t+1]=P[t]+(fT-fF);
            }
            double A=P[k];
            double best=1e300;int bi=0,bj=k-1;
            set<pair<double,int>> S;
            S.insert({P[0],0});
            for(int r=1;r<=k;r++){
                double target=P[r]-A/2;
                auto it=S.lower_bound({target,-1});
                for(int rep=0;rep<2;rep++){
                    if(it!=S.end()){
                        double s=P[r]-it->first;
                        double cost=max(BF+s,BT-s);
                        if(cost<best){best=cost;bi=it->second;bj=r-1;}
                    }
                    if(it==S.begin())break;
                    --it;
                }
                S.insert({P[r],r});
            }
            int l=pos[bi],r=pos[bj];
            bool c=ask(l,r);
            applyAns(l,r,c);
        }
    }
    for(size_t t=0;t<pos.size();t++){
        printf("! %d\n",pos[t]);fflush(stdout);
        int res;
        if(scanf("%d",&res)!=1)return 0;
        if(res<0)return 0;
        if(res==1)break;
    }
    printf("#\n");fflush(stdout);
    return 0;
}

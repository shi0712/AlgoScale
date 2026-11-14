#include <bits/stdc++.h>
using namespace std;

/*
View A as edges of a circle. An operation deletes the vertex next to the
cursor and moves the cursor to the following vertex. Surviving vertices
therefore specify the final XOR blocks.

In a gap between survivors, three deletions can form an excursion returning
to the same endpoint. A block of length 1 (mod 3) can be cleared by such
excursions; length 2 (mod 3) crosses to the other endpoint; length 0 (mod 3)
cannot be cleared. A length-1 block permits no excursion at all.

For the gap containing the initial cursor, let its length be L and the
cursor's distance from its left endpoint be r. It can finish on the right
when r+L=2 (mod 3), and on the left when r+L=1 (mod 3).

The gap graph is a cycle: excursions use edges twice, crossings once.
An Euler walk must connect the initial and final cursor. In one direction,
this gives the four phases used below: positive excursions, untouched
single edges, positive excursions, then crossings, with the initial gap
between the last two phases. Reversing both arrays covers the other
direction. A full circle consisting entirely of crossings is handled first.

The DP merges paths with the same endpoint, phase, and start residue by
keeping the largest start. Their start prefix XORs agree automatically.
The last block can be extended by a multiple of three to make the total
length exactly n. Hash tables aggregate eligible predecessor endpoints,
giving O(n*m) time. Fixed-partition reconstruction uses the Euler criterion
to choose each next operation, also in O(n*m) time.
*/

struct FastInput {
    static constexpr int S=1<<16;
    char buf[S]; int at=0,len=0;
    inline char get(){if(at==len){len=fread(buf,1,S,stdin);at=0;if(!len)return 0;}return buf[at++];}
    void skip(){char c;do{c=get();}while(c<=32&&c);while(c>32)c=get();}
    int read(){char c;do{c=get();}while(c<=32&&c);int x=0;while(c>32){x=x*10+c-'0';c=get();}return x;}
};
struct IntMap {
    vector<uint32_t> keys;vector<int> vals;int mask;
    IntMap(int n){int z=8;while(z<n*4)z<<=1;keys.assign(z,UINT32_MAX);vals.resize(z);mask=z-1;}
    inline unsigned hash(uint32_t x)const{x^=x>>16;x*=0x7feb352dU;x^=x>>15;x*=0x846ca68bU;x^=x>>16;return x&mask;}
    int find(uint32_t x)const{unsigned h=hash(x);while(keys[h]!=UINT32_MAX){if(keys[h]==x)return vals[h];h=(h+1)&mask;}return -1;}
    int insert(uint32_t x,int id){unsigned h=hash(x);while(keys[h]!=UINT32_MAX){if(keys[h]==x)return vals[h];h=(h+1)&mask;}keys[h]=x;vals[h]=id;return id;}
};

// Find the circular partition in one of the two possible directions.
// The original starting vertex is represented by position n in doubled A.
bool partition_one(const vector<int>& a,const vector<int>& b,vector<int>& cuts){
    int n=a.size(),m=b.size(),nn=2*n;
    vector<int> p(nn+1),id(nn);IntMap mp(nn+1);int kinds=0;
    for(int i=0;i<nn;i++)p[i+1]=p[i]^a[i%n];
    for(int i=0;i<nn;i++){id[i]=mp.insert(p[i],kinds);if(id[i]==kinds)++kinds;}

    // A complete turn with every gap crossed once starts and ends at vertex 0.
    if(n%3==(2*m)%3){
        vector<vector<int>> occ(kinds*3);
        for(int i=0;i<=n;i++){int v=mp.find(p[i]);if(v>=0)occ[v*3+i%3].push_back(i);}
        int pos=0,x=0;vector<int> cc(1,0);bool ok=true;
        for(int j=0;j<m-1;j++){
            x^=b[j];int v=mp.find(x);if(v<0){ok=false;break;}
            auto &ls=occ[v*3+(pos+2)%3];auto it=lower_bound(ls.begin(),ls.end(),pos+2);
            if(it==ls.end()){ok=false;break;}pos=*it;cc.push_back(pos);
        }
        if(ok&&n-pos>=2&&(n-pos)%3==2){cc.push_back(n);cuts=move(cc);return true;}
    }

    // Before position n: 0 = nonempty excursions, 1 = untouched edges,
    // 2 = nonempty excursions again. After n: 3 = edges crossed once.
    const int width=12*n;
    vector<int16_t> old(width,-1),cur(width,-1);
    vector<uint16_t> parent((size_t)(m+1)*width);
    auto ix=[&](int i,int ph,int r){return i<n?9*i+3*ph+r:9*n+3*(i-n)+r;};
    for(int s=0;s<n;s++)old[ix(s,0,s%3)]=s;
    // Tables: prefix phase 0, prefix phases 1/2, initial gap, suffix.
    int tableSize=4*kinds*9;
    vector<int16_t> best(tableSize);
    vector<uint16_t> arg(tableSize);
    auto add=[&](int type,int h,int ph){
        int base=(type*kinds+id[h])*9+(h%3)*3;
        int oi=ix(h,ph,0);uint16_t enc=h*4+ph+1;
        for(int r=0;r<3;r++)if(old[oi+r]>best[base+r]){
            best[base+r]=old[oi+r];arg[base+r]=enc;
        }
    };
    for(int j=1;j<=m;j++){
        fill(cur.begin(),cur.end(),-1);fill(best.begin(),best.end(),-1);
        size_t row=(size_t)j*width;
        auto query=[&](int i,int ph,int type,int v,int residue){
            if(v<0)return;
            int base=(type*kinds+v)*9+residue*3, ci=ix(i,ph,0);
            for(int r=0;r<3;r++)if(best[base+r]>cur[ci+r]){
                cur[ci+r]=best[base+r];parent[row+ci+r]=arg[base+r];
            }
        };
        for(int i=0;i<n;i++){
            if(i>=4){int h=i-4;add(0,h,0);add(1,h,1);add(1,h,2);}
            int v=mp.find(p[i]^b[j-1]);
            query(i,0,0,v,(i+2)%3);query(i,2,1,v,(i+2)%3);
            if(i>=1&&a[i-1]==b[j-1]){
                int ci=ix(i,1,0);
                for(int ph=0;ph<=1;ph++){
                    int oi=ix(i-1,ph,0);
                    for(int r=0;r<3;r++)if(old[oi+r]>cur[ci+r]){
                        cur[ci+r]=old[oi+r];parent[row+ci+r]=(i-1)*4+ph+1;
                    }
                }
            }
        }
        for(int h=0;h<n;h++){
            add(2,h,0);add(2,h,1);if(h<=n-3)add(2,h,2);
        }
        for(int i=n;i<nn;i++){
            if(i==n+1&&n>=2)add(2,n-2,2);
            if(i==n+2)add(2,n-1,2);
            if(i>=n+2)add(3,i-2,3);
            int v=mp.find(p[i]^b[j-1]);
            int residue=(2-n-i)%3;if(residue<0)residue+=3;
            query(i,3,2,v,residue);
            query(i,3,3,v,(i+1)%3);
            if(j==m){
                int r=(i-n)%3;if(r<0)r+=3;
                int s=cur[ix(i,3,r)];
                if(s>=0&&s+n>=i){
                    cuts.resize(m+1);int at=i,ph=3;
                    for(int k=m;k>=1;k--){
                        cuts[k]=at;
                        unsigned code=parent[(size_t)k*width+ix(at,ph,r)]-1;
                        at=code/4;ph=code%4;
                    }
                    cuts[0]=at;cuts[m]=s+n;
                    return true;
                }
            }
        }
        old.swap(cur);
    }
    return false;
}

// For fixed surviving vertices, test the cursor-and-gap deletion game.
struct Reconstruct {
    int n,m,cursor,k,rank;
    vector<int> nxt,prv,sv,gap,L,weight;
    vector<int> ops;
    Reconstruct(int n_,const vector<int>& cuts):n(n_),m((int)cuts.size()-1),cursor(0){
        nxt.resize(n);prv.resize(n);sv.assign(n,-1);gap.resize(n);L.resize(m);weight.resize(m);
        for(int i=0;i<n;i++){nxt[i]=(i+1)%n;prv[i]=(i+n-1)%n;}
        for(int j=0;j<m;j++)sv[cuts[j]%n]=j;
        for(int j=0;j<m;j++){
            L[j]=cuts[j+1]-cuts[j];
            for(int x=cuts[j]+1;x<cuts[j+1];x++)gap[x%n]=j;
            if(cuts[j]<n&&n<=cuts[j+1]){k=j;rank=n-cuts[j];}
        }
    }
    bool possible(){
        int z=(rank+L[k])%3;if(z==0)return false;
        int start=z==2?(k+1)%m:k;
        int nonzero=0;
        for(int j=0;j<m;j++){
            if(j==k)weight[j]=L[j]>1?2:0;
            else{
                int r=L[j]%3;if(r==0)return false;
                weight[j]=r==2?1:(L[j]>1?2:0);
            }
            nonzero+=weight[j]>0;
        }
        for(int j=0;j<m;j++){
            bool odd=(weight[(j+m-1)%m]+weight[j])%2;
            if(odd!=((j==0)^(j==start)))return false;
        }
        if(!nonzero)return start==0;
        if(!weight[start]&&!weight[(start+m-1)%m])return false;
        int components=0;
        for(int j=0;j<m;j++)if(weight[j]&&!weight[(j+m-1)%m])++components;
        return components<=1;
    }
    bool attempt(int dir){
        int del=dir==1?nxt[cursor]:prv[cursor];
        if(sv[del]>=0)return false;
        int to=dir==1?nxt[del]:prv[del];
        int g=gap[del],oldk=k,oldrank=rank;--L[g];
        if(sv[to]>=0){k=(sv[to]+m-1)%m;rank=L[k];}
        else if(dir==1){
            if(sv[cursor]>=0){k=sv[cursor];rank=1;}else ++rank;
        }else{
            if(sv[cursor]>=0){k=(sv[cursor]+m-1)%m;rank=L[k]-1;}else rank-=2;
        }
        if(!possible()){++L[g];k=oldk;rank=oldrank;return false;}
        nxt[prv[del]]=nxt[del];prv[nxt[del]]=prv[del];cursor=to;ops.push_back(dir);return true;
    }
    vector<int> run(){
        for(int left=n;left>m;left--){
            if(!attempt(1)&&!attempt(2)){cerr<<"Reconstruction failed\n";abort();}
        }
        return ops;
    }
};

int main(){
    FastInput in;in.skip();int t=in.read();string out;
    while(t--){
        int n=in.read(),m=in.read();vector<int>a(n),b(m);int xa=0,xb=0;
        for(int &x:a){x=in.read();xa^=x;}for(int &x:b){x=in.read();xb^=x;}
        if(xa!=xb){out+="No\n";continue;}
        if(m==1){out+="Yes\n";for(int i=1;i<n;i++)out+="1 ";out+='\n';continue;}
        if(n==m){if(a==b)out+="Yes\n\n";else out+="No\n";continue;}
        vector<int>cuts;bool rev=false;
        bool ok=partition_one(a,b,cuts);
        if(!ok){reverse(a.begin(),a.end());reverse(b.begin(),b.end());rev=true;ok=partition_one(a,b,cuts);}
        if(!ok){out+="No\n";continue;}
        Reconstruct rc(n,cuts);auto ops=rc.run();out+="Yes\n";
        for(int op:ops){out+=char('0'+(rev?3-op:op));out+=' ';}out+='\n';
    }
    fwrite(out.data(),1,out.size(),stdout);
}

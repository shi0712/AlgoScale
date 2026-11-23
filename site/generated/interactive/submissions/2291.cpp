#include <bits/stdc++.h>
using namespace std;

static bool less_than(int i,int j){
    printf("? %d %d\n",i,j); fflush(stdout);
    int r; if(scanf("%d",&r)!=1) exit(0); if(r<0) exit(0); return r==1;
}
static int add_vals(int i,int j){
    printf("+ %d %d\n",i,j); fflush(stdout);
    int r; if(scanf("%d",&r)!=1) exit(0); if(r<0) exit(0); return r;
}
static vector<int> mergeRuns(const vector<int>&A,const vector<int>&B){
    vector<int> C; C.reserve(A.size()+B.size());
    if(!less_than(B[0],A.back())){C.insert(C.end(),A.begin(),A.end());C.insert(C.end(),B.begin(),B.end());return C;}
    size_t i=0,j=0;
    while(i<A.size()&&j<B.size()){ if(less_than(B[j],A[i])) C.push_back(B[j++]); else C.push_back(A[i++]); }
    while(i<A.size())C.push_back(A[i++]);
    while(j<B.size())C.push_back(B[j++]);
    return C;
}
static vector<int> sortLabels(vector<int> a){
    int n=(int)a.size(); if(n<=1)return a;
    vector<vector<int>> runs; int i=0;
    while(i<n){
        int j=i;
        if(j+1<n){
            if(less_than(a[j+1],a[j])){ while(j+1<n&&less_than(a[j+1],a[j]))j++; reverse(a.begin()+i,a.begin()+j+1); }
            else { while(j+1<n&&!less_than(a[j+1],a[j]))j++; }
        }
        runs.emplace_back(a.begin()+i,a.begin()+j+1); i=j+1;
    }
    while(runs.size()>1){
        vector<vector<int>> nr; nr.reserve((runs.size()+1)/2);
        for(size_t k=0;k+1<runs.size();k+=2) nr.push_back(mergeRuns(runs[k],runs[k+1]));
        if(runs.size()&1) nr.push_back(runs.back());
        runs.swap(nr);
    }
    return runs[0];
}
static int gallopInsert(const vector<int>&v,int s,int p){
    int k=(int)v.size(); if(p<0)p=0; if(p>=k)p=k-1;
    const int CAP=4;
    if(less_than(s,v[p])){
        int hi=p,step=1,lo=p-1,c=0;
        while(lo>=0&&c<CAP&&less_than(s,v[lo])){hi=lo;step<<=1;lo=p-step;c++;}
        int L,H;
        if(lo<0){L=0;H=hi;}
        else if(c<CAP){L=lo+1;H=hi;}
        else{L=hi;H=p;}
        while(L<H){int mid=L+(H-L)/2; if(less_than(s,v[mid]))H=mid; else L=mid+1;}
        return L;
    } else {
        int lo=p,step=1,hi=p+1,c=0;
        while(hi<k&&c<CAP&&!less_than(s,v[hi])){lo=hi;step<<=1;hi=p+step;c++;}
        int L,H;
        if(hi>=k){L=lo+1;H=k;}
        else if(c<CAP){L=lo+1;H=hi;}
        else{L=p+1;H=lo+1;}
        while(L<H){int mid=L+(H-L)/2; if(less_than(s,v[mid]))H=mid; else L=mid+1;}
        return L;
    }
}
int main(){
    int n; if(scanf("%d",&n)!=1)return 0;
    vector<int> lab(n); for(int i=0;i<n;i++)lab[i]=i+1;
    vector<int> v=sortLabels(lab);
    int lastPos=-1;
    while(v.size()>=2){
        int m=v.front(),M=v.back();
        int s=add_vals(m,M);
        v.erase(v.begin()); v.pop_back();
        if(v.empty()) break;
        if(v.size()==1){ add_vals(s,v[0]); v.clear(); break; }
        int k=(int)v.size(); int pos;
        if(lastPos<0){
            if(!less_than(s,v[0])) pos=0;
            else if(!less_than(v[k-1],s)) pos=k;
            else pos=gallopInsert(v,s,k/2);
        } else {
            int p=lastPos-1; if(p<0)p=0; if(p>=k)p=k-1;
            pos=gallopInsert(v,s,p);
        }
        v.insert(v.begin()+pos,s);
        lastPos=pos;
    }
    printf("!\n"); fflush(stdout);
    return 0;
}

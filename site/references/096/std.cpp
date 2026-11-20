#include <bits/stdc++.h>
using namespace std;
struct Appraiser {
    function<bool(int,int)> same;
    vector<int> fake;
    int genuine=-1;
    Appraiser(function<bool(int,int)> same):same(same),fake(1000,-1){}
    struct Group{vector<int> a,b;};
    Group partition(const vector<int>& list,int start,int size){
        Group g;g.a.push_back(list[start]);
        for(int k=1;k<size;++k)
            (same(list[start],list[start+k])?g.a:g.b).push_back(list[start+k]);
        return g;
    }
    int paint(Group& g,bool first_fake){
        for(int x:g.a) fake[x]=first_fake;
        for(int x:g.b) fake[x]=!first_fake;
        return first_fake?g.a.size():g.b.size();
    }
    void one(const vector<int>& v){
        for(int x:v)fake[x]=0;
        int i=0;
        for(;i+2<int(v.size());i+=2)if(!same(v[i],v[i+1])){
            fake[same(v[i],genuine)?v[i+1]:v[i]]=1;return;
        }
        if(i+1==int(v.size()))fake[v[i]]=1;
        else fake[same(v[i],genuine)?v[i+1]:v[i]]=1;
    }
    void two(const vector<int>& v){
        vector<Group> mixed;int i=0,remain=2;
        for(;i+2<int(v.size());i+=3){
            Group g=partition(v,i,3);
            if(g.b.empty())paint(g,false);else mixed.push_back(g);
        }
        for(;i<int(v.size());++i){fake[v[i]]=!same(v[i],genuine);remain-=fake[v[i]];}
        for(auto& g:mixed){
            int needed=mixed.size()==2?1:remain;
            paint(g,int(g.a.size())==needed);
        }
    }
    void three(const vector<int>& v){
        int n=v.size(),ca=0,cb=0,best=n+3;
        for(int a=0;2*a<=n;++a)for(int b=0;2*a+3*b<=n;++b){
            int cost=n-a-b+max({3,1+(a+1)/2,(b+1)/2});
            if(cost<best){best=cost;ca=a;cb=b;}
        }
        process(v,3,{{{2,ca},{3,cb}}});
    }
    void solve_small(const vector<int>& reps,int k){
        if(k==0){for(int x:reps)fake[x]=0;return;}
        if(k==1)one(reps);else if(k==2)two(reps);else three(reps);
    }
    void process(const vector<int>& v,int remain,array<pair<int,int>,2> shape){
        vector<Group> pure[2];vector<int> representatives[2];
        int next=0;
        for(int c=0;c<2;++c)for(int j=0;j<shape[c].second;++j){
            Group g=partition(v,next,shape[c].first);next+=shape[c].first;
            if(g.b.empty()){representatives[c].push_back(g.a[0]);pure[c].push_back(move(g));}
            else remain-=paint(g,!same(g.a[0],genuine));
        }
        for(;next<int(v.size());++next){
            fake[v[next]]=!same(v[next],genuine);remain-=fake[v[next]];
        }
        int num_a=0,num_b=0;
        for(int a=0;a*shape[0].first<=remain;++a)
            if((remain-a*shape[0].first)%shape[1].first==0){
                num_a=a;num_b=(remain-a*shape[0].first)/shape[1].first;break;
            }
        for(int c=0;c<2;++c){
            solve_small(representatives[c],c?num_b:num_a);
            for(auto& g:pure[c])paint(g,fake[g.a[0]]);
        }
    }
    void run(){
        vector<int> items(1000);iota(items.begin(),items.end(),0);
        Group anchor=partition(items,0,22);
        bool first_fake=anchor.a.size()<anchor.b.size();
        int remain=10-paint(anchor,first_fake);
        genuine=(first_fake?anchor.b:anchor.a).front();
        vector<int> rest(items.begin()+22,items.end());
        process(rest,remain,{{{3,196},{5,78}}});
    }
};
#ifdef VERIFY
int main(){
    mt19937 rng(14597);int worst=0;
    for(int t=0;t<9000;++t){
        vector<int> state(1000),all(1000);iota(all.begin(),all.end(),0);shuffle(all.begin(),all.end(),rng);
        int n=0;
        if(t%4==1){for(int g=0;g<3;++g)for(int j=0;j<3;++j){state[22+3*((t+g)%196)+j]=1;++n;}}
        if(t%4==2){for(int j=0;j<3;++j){state[22+3*(t%196)+j]=1;++n;}
            for(int j=0;j<5;++j){state[610+5*(t%78)+j]=1;++n;}}
        for(int x:all)if(n<10 && !state[x]){state[x]=1;++n;}
        int calls=0;Appraiser a([&](int i,int j){++calls;return state[i]==state[j];});
        a.run();if(a.fake!=state || calls>864)return 1;worst=max(worst,calls);
    }
    cout<<"9000 configurations verified, maximum "<<worst<<"\n";
}
#else
int main(){
    int n,m;if(!(cin>>n>>m))return 0;
    Appraiser a([](int i,int j){cout<<"? "<<i+1<<' '<<j+1<<endl;int r;
        if(!(cin>>r)||r<0)exit(0);return r==0;});
    a.run();cout<<"!";for(int i=0;i<n;++i)if(a.fake[i])cout<<' '<<i+1;cout<<endl;
}
#endif

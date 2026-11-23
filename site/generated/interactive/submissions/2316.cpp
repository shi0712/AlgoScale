#include <bits/stdc++.h>
using namespace std;

int main(){
    int n;
    if(!(cin>>n)) return 0;
    vector<char> removed(n+1,0);
    // ops: type 0 = A(v,u), type 1 = B(v,w)
    vector<array<int,3>> ops;
    int k=n;
    bool special=false;
    deque<int> dq;
    auto ask=[&](int d, int &a, int &b)->bool{
        cout<<"? "<<d<<"\n"<<flush;
        if(!(cin>>a)) return false;
        if(a==-1) return false;
        if(!(cin>>b)) return false;
        if(b==-1) return false;
        return true;
    };
    while(k>=3){
        int v,u;
        if(!ask(k-2,v,u)) return 0;
        removed[v]=1;
        if(u!=0){
            ops.push_back({0,v,u});
            k--;
        } else {
            k--;
            if(k==2){
                vector<int> rem;
                for(int i=1;i<=n;i++) if(!removed[i]) rem.push_back(i);
                dq.push_back(rem[0]);
                dq.push_back(v);
                dq.push_back(rem[1]);
                special=true;
                break;
            } else {
                int w,x;
                if(!ask(0,w,x)) return 0;
                removed[w]=1;
                ops.push_back({1,v,w});
                k--;
            }
        }
    }
    if(!special){
        for(int i=1;i<=n;i++) if(!removed[i]) dq.push_back(i);
    }
    for(int i=(int)ops.size()-1;i>=0;i--){
        auto &op=ops[i];
        if(op[0]==0){
            int v=op[1],u=op[2];
            if(dq.empty()) dq.push_back(v);
            else if(dq.front()!=u) dq.push_front(v);
            else dq.push_back(v);
        } else {
            int v=op[1],w=op[2];
            dq.push_front(v);
            dq.push_front(w);
        }
    }
    cout<<"!";
    for(int x:dq) cout<<" "<<x;
    cout<<"\n"<<flush;
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

int n;
vector<pair<int,int>> allEdges;

static inline void add_edge(int a, int b){
    if(a==b) return;
    if(a>b) swap(a,b);
    allEdges.push_back({a,b});
}

string ask(const vector<int>& d){
    cout << '?';
    for(int i=0;i<n;i++) cout << ' ' << d[i];
    cout << '\n';
    cout.flush();
    string s;
    if(!(cin>>s)) exit(0);
    return s;
}

struct Params { int Q, L, k, rows, cap; };

Params choose_params(int m){
    if(m<=9)   return {3,3,2,9,9};
    if(m<=25)  return {5,4,2,20,25};
    if(m<=125) return {5,5,3,25,125};
    if(m<=343) return {7,5,3,35,343};
    return {7,7,4,49,2401};
}

void solve_hybrid(const vector<int>& U);

void solve_direct(const vector<int>& U){
    for(int v: U){
        vector<int> d(n,0);
        d[v-1]=1;
        string s=ask(d);
        for(int i=1;i<=n;i++) if(s[i-1]=='1') add_edge(v,i);
    }
}

void solve_codes(const vector<int>& U){
    int m=(int)U.size();
    if(m==0) return;
    if(m<=24){ solve_direct(U); return; }
    Params P = choose_params(m);
    vector<unsigned long long> code(m,0);
    for(int idx=0; idx<m; idx++){
        int t=idx;
        int a[8];
        for(int j=0;j<P.k;j++){ a[j]=t%P.Q; t/=P.Q; }
        for(int i=0;i<P.L;i++){
            int x=i;
            long long pw=1, val=0;
            for(int j=0;j<P.k;j++){
                val = (val + (long long)a[j]*pw) % P.Q;
                pw = (pw * x) % P.Q;
            }
            int row = i*P.Q + (int)val;
            code[idx] |= (1ULL<<row);
        }
    }
    vector<unsigned long long> obs(m,0);
    for(int bit=0; bit<P.rows; bit++){
        vector<int> d(n,0);
        for(int idx=0; idx<m; idx++)
            if((code[idx]>>bit)&1ULL) d[U[idx]-1]=1;
        string s=ask(d);
        for(int idx=0; idx<m; idx++)
            if(s[U[idx]-1]=='1') obs[idx] |= (1ULL<<bit);
    }
    vector<int> H;
    for(int idx=0; idx<m; idx++){
        int cnt=0;
        int cand[2];
        for(int j=0;j<m;j++){
            if(j==idx) continue;
            if((code[j] & ~obs[idx]) == 0){
                if(cnt<2) cand[cnt]=j;
                cnt++;
                if(cnt>2) break;
            }
        }
        if(cnt<=2){
            for(int t=0;t<cnt;t++) add_edge(U[idx], U[cand[t]]);
        } else {
            H.push_back(U[idx]);
        }
    }
    if(!H.empty()) solve_hybrid(H);
}

void solve_hybrid(const vector<int>& U){
    int m=(int)U.size();
    if(m<=1) return;

    vector<char> rem(n+1,0);
    for(int v: U) rem[v]=1;
    int unvisited = m;
    long long spent = 0;

    while(unvisited>0){
        int root=-1;
        for(int v: U) if(rem[v]){ root=v; break; }
        if(root==-1) break;
        rem[root]=0; unvisited--;
        vector<int> S;
        S.push_back(root);
        bool returned=false;
        int prevSize = 1;
        while(!S.empty()){
            int remTotal = (int)S.size() + unvisited;
            if(spent >= 8 && remTotal > 40 && (int)S.size() <= prevSize && (int)S.size() <= 128 && (long long)remTotal*3 > (long long)m*2){
                vector<int> Slist;
                for(int v: U) if(rem[v]) Slist.push_back(v);
                Slist.insert(Slist.end(), S.begin(), S.end());
                solve_codes(Slist);
                returned=true;
                break;
            }
            int sz=(int)S.size();
            int q=0;
            while((1<<q) <= sz) ++q;
            vector<int> obs(n+1,0);
            for(int b=0;b<q;b++){
                vector<int> d(n,0);
                for(int idx=0; idx<sz; idx++)
                    if(((idx+1)>>b)&1) d[S[idx]-1]=1;
                string s=ask(d);
                for(int i=1;i<=n;i++) if(s[i-1]=='1') obs[i] |= (1<<b);
            }
            unordered_map<int,int> codeToVertex;
            codeToVertex.reserve(sz*2+1);
            for(int idx=0; idx<sz; idx++) codeToVertex[idx+1]=S[idx];
            vector<int> newS;
            for(int i=1;i<=n;i++){
                if(!rem[i]) continue;
                int c=obs[i];
                if(c==0) continue;
                auto it=codeToVertex.find(c);
                if(it!=codeToVertex.end()){
                    add_edge(i, it->second);
                    rem[i]=0;
                    --unvisited;
                    newS.push_back(i);
                }
            }
            spent += q;
            prevSize = sz;
            S.swap(newS);
            if(unvisited==0){
                returned=true;
                break;
            }
        }
        if(returned) break;
    }
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin>>n)) return 0;

    if(n==2){
        add_edge(1,2);
    } else if(n==3){
        vector<int> d(3,0); d[0]=1;
        string s=ask(d);
        vector<int> nb;
        for(int i=1;i<=3;i++) if(s[i-1]=='1') nb.push_back(i);
        if((int)nb.size()==2){
            add_edge(1,nb[0]);
            add_edge(1,nb[1]);
        } else {
            int x=nb[0];
            int y=1;
            for(int v=1;v<=3;v++) if(v!=1 && v!=x) y=v;
            add_edge(1,x);
            add_edge(x,y);
        }
    } else {
        vector<int> U(n);
        iota(U.begin(), U.end(), 1);
        solve_hybrid(U);
    }

    // Deduplicate edges and validate that we really obtained a spanning tree.
    sort(allEdges.begin(), allEdges.end());
    allEdges.erase(unique(allEdges.begin(), allEdges.end()), allEdges.end());

    bool ok = ((int)allEdges.size() == n-1);
    if(ok){
        vector<int> par(n+1);
        iota(par.begin(), par.end(), 0);
        function<int(int)> find = [&](int x){
            while(par[x]!=x){ par[x]=par[par[x]]; x=par[x]; }
            return x;
        };
        for(auto &e: allEdges){
            int a=find(e.first), b=find(e.second);
            if(a==b){ ok=false; break; }
            par[a]=b;
        }
    }
    if(!ok){
        // Extremely defensive fallback: ask all neighbourhoods directly.
        allEdges.clear();
        for(int v=1; v<=n; v++){
            vector<int> d(n,0);
            d[v-1]=1;
            string s=ask(d);
            for(int i=1;i<=n;i++) if(s[i-1]=='1') add_edge(v,i);
        }
        sort(allEdges.begin(), allEdges.end());
        allEdges.erase(unique(allEdges.begin(), allEdges.end()), allEdges.end());
    }

    cout << "!\n";
    for(auto &e: allEdges) cout << e.first << ' ' << e.second << '\n';
    cout.flush();
    return 0;
}

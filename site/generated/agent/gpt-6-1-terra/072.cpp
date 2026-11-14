#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n; cin >> n;
        vector<vector<int>> child(n+1);
        vector<int> depth(n+1), parent(n+1);
        for (int i=2;i<=n;i++) {
            cin >> parent[i];
            child[parent[i]].push_back(i);
            depth[i]=depth[parent[i]]+1;
        }
        vector<int> euler, first(n+1,-1), iter(n+1), st;
        st.push_back(1); first[1]=0; euler.push_back(1);
        while (!st.empty()) {
            int u=st.back();
            if (iter[u]<(int)child[u].size()) {
                int v=child[u][iter[u]++];
                first[v]=euler.size(); euler.push_back(v); st.push_back(v);
            } else {
                st.pop_back();
                if (!st.empty()) euler.push_back(st.back());
            }
        }
        int m=euler.size();
        vector<int> lg(m+1);
        for (int i=2;i<=m;i++) lg[i]=lg[i/2]+1;
        vector<vector<int>> sparse(lg[m]+1,vector<int>(m));
        sparse[0]=euler;
        for (int j=1;j<(int)sparse.size();j++)
            for (int i=0;i+(1<<j)<=m;i++) {
                int a=sparse[j-1][i], b=sparse[j-1][i+(1<<(j-1))];
                sparse[j][i]=depth[a]<depth[b]?a:b;
            }
        auto dist=[&](int a,int b) {
            int l=first[a],r=first[b]; if(l>r) swap(l,r);
            int j=lg[r-l+1];
            int x=sparse[j][l],y=sparse[j][r-(1<<j)+1];
            int c=depth[x]<depth[y]?x:y;
            return depth[a]+depth[b]-2*depth[c];
        };
        vector<int> candidates(n); iota(candidates.begin(),candidates.end(),1);
        vector<int> freq(n+1), ds;
        uint64_t rng=0x9e3779b97f4a7c15ULL;
        auto random64=[&]() { rng^=rng<<7; rng^=rng>>9; return rng; };
        while (candidates.size()>1) {
            int sz=candidates.size();
            int bestWorst=sz, bestU=1, bestK=0;
            vector<int> centers;
            if(sz<=128) centers=candidates;
            else {
                centers.push_back(1);
                for(int i=0;i<64;i++) centers.push_back(candidates[random64()%sz]);
                for(int i=0;i<16;i++) centers.push_back(1+random64()%n);
            }
            for(int u:centers) {
                fill(freq.begin(),freq.end(),0);
                int maxd=0;
                for(int v:candidates) { int d=dist(u,v); ++freq[d]; maxd=max(maxd,d); }
                int count=0;
                for(int k=0;k<=maxd;k++) {
                    count+=freq[k];
                    if (!freq[k]) continue;
                    int worst=max(count,sz-count);
                    if(worst<bestWorst) { bestWorst=worst; bestU=u; bestK=k; }
                    if(count>=sz/2) break;
                }
                if(bestWorst==(sz+1)/2) break;
            }
            cout << "? " << bestU << ' ' << bestK << endl;
            int response; if(!(cin>>response)) return 0;
            vector<int> next;
            for(int v:candidates) if((dist(bestU,v)<=bestK)==(response==1)) next.push_back(v);
            candidates.swap(next);
        }
        cout << "! " << candidates[0] << endl;
    }
}

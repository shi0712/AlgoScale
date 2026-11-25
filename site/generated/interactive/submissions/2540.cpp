#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    
    int n;
    if (!(cin >> n)) return 0;
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
    }
    
    vector<int> val(n + 1, 0);
    
    // Step 1: determine current value of node 1 and whether root is center or leaf
    cout << "? 1 " << n;
    for (int i = 1; i <= n; ++i) cout << " " << i;
    cout << endl;
    long long Q0;
    if (!(cin >> Q0)) return 0;
    
    cout << "? 2 1" << endl;
    
    cout << "? 1 " << n;
    for (int i = 1; i <= n; ++i) cout << " " << i;
    cout << endl;
    long long Q1;
    if (!(cin >> Q1)) return 0;
    
    long long D = Q1 - Q0;
    long long c1 = llabs(D) / 2;
    int w1 = (int)(D / (2 * c1));   // current value of node 1
    val[1] = w1;
    int rootType = (c1 == n) ? 0 : 1;   // 0: center, 1: leaf
    
    // maximum block size b such that 2^b - 1 <= n
    int b = 1;
    while ((1 << (b + 1)) - 1 <= n) ++b;
    
    if (rootType == 0) {
        // root is node 1
        vector<int> leaves;
        for (int i = 2; i <= n; ++i) leaves.push_back(i);
        
        int idx = 0;
        while (idx < (int)leaves.size()) {
            int m = min(b, (int)leaves.size() - idx);
            int K = (1 << m) - 1;
            
            cout << "? 1 " << K;
            for (int pos = 0; pos < m; ++pos) {
                int leaf = leaves[idx + pos];
                int cnt = 1 << pos;
                for (int t = 0; t < cnt; ++t) cout << " " << leaf;
            }
            cout << endl;
            
            long long Q;
            if (!(cin >> Q)) return 0;
            
            long long T = Q - 1LL * K * w1;
            long long X = (T + K) / 2;
            
            for (int pos = 0; pos < m; ++pos) {
                int leaf = leaves[idx + pos];
                val[leaf] = ((X >> pos) & 1) ? 1 : -1;
            }
            idx += m;
        }
        
        cout << "!";
        for (int i = 1; i <= n; ++i) cout << " " << val[i];
        cout << endl;
    } else {
        // root is a leaf
        // query s = w_root + w1
        cout << "? 1 " << n;
        for (int i = 0; i < n; ++i) cout << " 1";
        cout << endl;
        long long Qs;
        if (!(cin >> Qs)) return 0;
        int s = (int)(Qs / n);
        
        struct Block {
            vector<int> leaves;
            vector<int> weights;
            int K;
            long long Q;
        };
        
        vector<Block> blocks;
        vector<int> leaves;
        for (int i = 2; i <= n; ++i) leaves.push_back(i);
        
        int idx = 0;
        while (idx < (int)leaves.size()) {
            int m = min(b, (int)leaves.size() - idx);
            Block blk;
            blk.K = (1 << m) - 1;
            for (int pos = 0; pos < m; ++pos) {
                blk.leaves.push_back(leaves[idx + pos]);
                blk.weights.push_back(1 << pos);
            }
            
            cout << "? 1 " << blk.K;
            for (int pos = 0; pos < m; ++pos) {
                int leaf = blk.leaves[pos];
                int cnt = blk.weights[pos];
                for (int t = 0; t < cnt; ++t) cout << " " << leaf;
            }
            cout << endl;
            
            long long Q;
            if (!(cin >> Q)) return 0;
            blk.Q = Q;
            blocks.push_back(blk);
            idx += m;
        }
        
        vector<vector<int>> candidates;
        for (int j = 2; j <= n; ++j) {
            vector<int> w(n + 1, 0);
            w[1] = w1;
            bool ok = true;
            
            for (auto &blk : blocks) {
                bool in = false;
                int a_j = 0;
                for (int pos = 0; pos < (int)blk.leaves.size(); ++pos) {
                    if (blk.leaves[pos] == j) {
                        in = true;
                        a_j = blk.weights[pos];
                        break;
                    }
                }
                
                long long U;
                if (in) {
                    U = blk.Q - 1LL * (blk.K - a_j) * s;
                } else {
                    U = blk.Q - 1LL * blk.K * s;
                }
                
                if ((U + blk.K) % 2 != 0) { ok = false; break; }
                long long X = (U + blk.K) / 2;
                if (X < 0 || X > blk.K) { ok = false; break; }
                
                for (int pos = 0; pos < (int)blk.leaves.size(); ++pos) {
                    int leaf = blk.leaves[pos];
                    w[leaf] = ((X >> pos) & 1) ? 1 : -1;
                }
            }
            
            if (ok && w[j] == s - w1) {
                candidates.push_back(w);
            }
        }
        
        // resolve ambiguity if necessary
        while (candidates.size() > 1) {
            int i = -1;
            for (int node = 2; node <= n; ++node) {
                int v0 = candidates[0][node];
                bool diff = false;
                for (auto &c : candidates) {
                    if (c[node] != v0) { diff = true; break; }
                }
                if (diff) { i = node; break; }
            }
            if (i == -1) break;
            
            cout << "? 1 1 " << i << endl;
            long long Q1;
            if (!(cin >> Q1)) return 0;
            
            cout << "? 2 " << i << endl;
            
            cout << "? 1 1 " << i << endl;
            long long Q2;
            if (!(cin >> Q2)) return 0;
            
            int old_val = (int)((Q1 - Q2) / 2);
            
            vector<vector<int>> newc;
            for (auto &c : candidates) {
                if (c[i] == old_val) newc.push_back(c);
            }
            candidates = newc;
            
            for (auto &c : candidates) {
                c[i] = -c[i];
            }
        }
        
        if (candidates.empty()) return 0;
        
        cout << "!";
        for (int i = 1; i <= n; ++i) cout << " " << candidates[0][i];
        cout << endl;
    }
    
    return 0;
}

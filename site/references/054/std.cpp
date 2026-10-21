// IOI 2025 World Map: for N <= 40, guaranteed K/N <= 45/37.
// A and B1 reproduce https://qoj.ac/problem/13904/discussion/2715 .
// B2/B3 overlap more columns and restore lost root edges in spare cells.
// A second DFS is forced along a deterministically constructed long path.
// See worldmap_optimized_proof.md and verify_worldmap_bound.py.
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>
using namespace std;
namespace {

// A DFS tree is needed only to obtain a matching whose unmatched vertices
// form an independent set. Thereafter we may root the backbone anywhere.
vector<vector<int>> worldmap125_A(int N, int M, vector<int> A, vector<int> B) {
    if (N == 1) return {{1}};
    vector<vector<int>> g(N), tree(N), child(N);
    vector<vector<bool>> edge(N, vector<bool>(N));
    for (int i = 0; i < M; ++i) {
        int u = A[i] - 1, v = B[i] - 1;
        g[u].push_back(v); g[v].push_back(u);
        edge[u][v] = edge[v][u] = true;
    }
    vector<bool> seen(N);
    function<void(int)> dfs = [&](int u) {
        seen[u] = true;
        for (int v : g[u]) if (!seen[v]) {
            child[u].push_back(v);
            tree[u].push_back(v); tree[v].push_back(u);
            dfs(v);
        }
    };
    dfs(0);
    vector<int> mate(N, -1);
    int pairs = 0;
    function<void(int)> matching = [&](int u) {
        if (mate[u] == -1 && !child[u].empty()) {
            int v = child[u].back();
            mate[u] = v; mate[v] = u;
            ++pairs;
        }
        for (int v : child[u]) matching(v);
    };
    matching(0);
    vector<int> free;
    for (int u = 0; u < N; ++u) if (mate[u] == -1) free.push_back(u);
    int l = int(free.size());
    auto bfs = [&](int start, vector<int>& parent, vector<int>& distance) {
        parent.assign(N, -1); distance.assign(N, -1);
        queue<int> q; q.push(start); distance[start] = 0;
        int far = start;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            if (distance[u] > distance[far]) far = u;
            for (int v : tree[u]) if (mate[v] != -1 && distance[v] == -1) {
                distance[v] = distance[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
        return far;
    };
    vector<int> parent, distance;
    int s = bfs(0, parent, distance);
    int t = bfs(s, parent, distance);
    int diameter = distance[t];
    vector<int> next(N, -1);
    for (int u = t; parent[u] != -1; u = parent[u]) next[parent[u]] = u;
    vector<int> walk;
    function<void(int,int)> tour = [&](int u, int p) {
        walk.push_back(u);
        for (int v : tree[u]) {
            if (v == p || v == next[u] || mate[v] == -1) continue;
            tour(v, u);
            walk.push_back(u);
        }
        if (next[u] != -1) tour(next[u], u);
    };
    tour(s, -1);
    assert(int(walk.size()) == 4 * pairs - 1 - diameter);
    int firstA = walk[0], firstB = walk[1];
    assert(mate[firstA] == firstB);
    vector<int> group(N, -1);
    group[firstA] = group[firstB] = 0;
    int groups = 1;
    for (int u = 0; u < N; ++u) {
        if (mate[u] != -1 && group[u] == -1) {
            group[u] = group[mate[u]] = groups++;
        }
    }
    struct Gadget { int d, u, v, group; };
    vector<Gadget> gadgets;
    vector<bool> inserted(groups);
    inserted[0] = true;
    vector<int> word(l + 1, firstA);
    for (int i = 1; i < int(walk.size()); ++i) {
        int u = walk[i - 1], v = walk[i];
        if (mate[u] == v && !inserted[group[u]]) {
            inserted[group[u]] = true;
            gadgets.push_back({int(word.size()) - 1, u, v, group[u]});
            word.push_back(u);
            word.push_back(v);
        }
        word.push_back(v);
    }
    word.insert(word.end(), l, walk.back());
    int K = (int(word.size()) + 2) / 2;
    assert(K == N + pairs - 1 - diameter / 2);
    word.resize(2 * K - 1, word.back());
    vector<vector<int>> ans(K, vector<int>(K));
    for (int r = 0; r < K; ++r)
        for (int c = 0; c < K; ++c) ans[r][c] = word[r + c];
    // Outer diagonals are u, v; inner values are A_r, B_r.
    // A_r is adjacent to B_r and B_(r+1).
    auto strip = [&](int d, int u, int v, const vector<int>& targets) {
        vector<pair<int,int>> slots[3];
        for (int z : targets) {
            bool a = edge[u][z], b = edge[v][z];
            if (!a && b) slots[0].push_back({v, z});
            if (a && b) slots[1].push_back({u, z});
            if (a && !b) slots[2].push_back({z, u});
        }
        vector<pair<int,int>> all;
        for (auto& part : slots) all.insert(all.end(), part.begin(), part.end());
        int low = max(0, d + 3 - K);
        int high = min(K - 1, d + 1);
        assert(int(all.size()) <= high - low + 1);
        for (int r = 0; r < K; ++r) {
            int a = r < low ? v : u, b = a;
            if (r >= low && r - low < int(all.size())) {
                a = all[r - low].first;
                b = all[r - low].second;
            }
            int ca = d + 1 - r, cb = d + 2 - r;
            if (0 <= ca && ca < K) ans[r][ca] = a;
            if (0 <= cb && cb < K) ans[r][cb] = b;
        }
    };
    if (l >= 2) strip(l - 2, firstA, firstB, free);
    if (l == 1) {
        int z = free[0];
        bool a = edge[firstA][z], b = edge[firstB][z];
        if (a && b) { ans[0][0] = firstA; ans[0][1] = firstA; ans[1][0] = z; }
        if (a && !b) { ans[0][0] = z; ans[0][1] = ans[1][0] = firstA; }
        if (!a && b) { ans[0][0] = firstB; ans[0][1] = firstA; ans[1][0] = z; }
    }
    sort(gadgets.begin(), gadgets.end(), [&](const Gadget& a, const Gadget& b) {
        return min(a.d, 2 * K - 5 - a.d) < min(b.d, 2 * K - 5 - b.d);
    });
    vector<bool> processed(groups);
    processed[0] = true;
    for (auto [d, u, v, id] : gadgets) {
        vector<int> targets;
        for (int z = 0; z < N; ++z)
            if (group[z] == -1 || processed[group[z]]) targets.push_back(z);
        strip(d, u, v, targets);
        processed[id] = true;
    }
    for (auto& row : ans) for (int& v : row) ++v;
    return ans;
}


vector<vector<int>> worldmap125_B(int N, int M, vector<int> A, vector<int> B) {
    if (N == 1) return {{1}};
    vector<vector<int>> adj(N), child(N);
    vector<vector<bool>> edge(N, vector<bool>(N));
    for (int i=0;i<M;++i) {
        int u=A[i]-1,v=B[i]-1;
        adj[u].push_back(v); adj[v].push_back(u);
        edge[u][v]=edge[v][u]=true;
    }
    vector<int> parent(N,-1), depth(N), seen(N);
    function<void(int)> dfs=[&](int u) {
        seen[u]=1;
        for(int v:adj[u]) if(!seen[v]) {
            parent[v]=u;depth[v]=depth[u]+1;
            child[u].push_back(v);dfs(v);
        }
    };
    dfs(0);
    int endpoint=int(max_element(depth.begin(),depth.end())-depth.begin());
    int H=depth[endpoint],width=2*H;
    vector<vector<int>> row(N);
    for(int v=0;v<N;++v) {
        vector<int> path;
        for(int u=v;u!=-1;u=parent[u]) path.push_back(u);
        reverse(path.begin(),path.end());
        for(int k=H-1;k>=0;--k) {
            int x=v,y=v;
            if(k<depth[v]) {
                int a=path[k],b=path[k+1];
                x=edge[v][b]&&!edge[v][a]?b:a;
                y=edge[v][a]?v:a;
            }
            row[v].push_back(x);row[v].push_back(y);
        }
        row[v].erase(row[v].begin());row[v].push_back(0);
    }
    vector<int> next(N,-1);
    for(int v=endpoint;parent[v]!=-1;v=parent[v]) next[parent[v]]=v;
    vector<int> walk;
    function<void(int)> tour=[&](int u) {
        walk.push_back(u);
        for(int v:child[u]) if(v!=next[u]) {
            tour(v);walk.push_back(u);
        }
        if(next[u]!=-1) tour(next[u]);
    };
    tour(0);
    int L=int(walk.size());
    assert(L==2*N-1-H);
    int K=max(width,N-1+(H+1)/2);
    int prefix=max(0,L-K),suffix=L-prefix;
    vector<vector<int>> ans(K,vector<int>(K,0));
    for(int r=0;r<K;++r)
        for(int c=0;c<width;++c)
            ans[r][c]=row[walk[prefix+min(r,suffix-1)]][c];
    if(prefix>0) {
        for(int r=0;r<width;++r)
            for(int c=width-1;c<K;++c)
                ans[K-width+r][c]=row[walk[min(c-width+1,prefix-1)]][width-1-r];
    }
    for(auto& line:ans)for(int& v:line)++v;
    return ans;
}


// Two-column overlap, with root edges restored in the root-colored padding.
// Empty result means that this candidate's repair area is too small.
vector<vector<int>> worldmap_B2(int N, int M, vector<int> A, vector<int> B) {
    if (N == 1) return {{1}};
    vector<vector<int>> adj(N), child(N);
    vector<vector<bool>> edge(N, vector<bool>(N));
    for (int i = 0; i < M; ++i) {
        int u=A[i]-1,v=B[i]-1;
        adj[u].push_back(v); adj[v].push_back(u);
        edge[u][v]=edge[v][u]=true;
    }
    const int root=0; // Wrapper chooses a minimum-degree non-articulation vertex.
    vector<int> parent(N,-1),depth(N),seen(N);
    function<void(int)> dfs=[&](int u) {
        seen[u]=1;
        for(int v:adj[u]) if(!seen[v]) {
            parent[v]=u;depth[v]=depth[u]+1;
            child[u].push_back(v);dfs(v);
        }
    };
    dfs(root);
    int endpoint=int(max_element(depth.begin(),depth.end())-depth.begin());
    int H=depth[endpoint],w=2*H;
    if(H<2) return {};
    vector<vector<int>> row(N);
    for(int v=0;v<N;++v) {
        vector<int> path;
        for(int u=v;u!=-1;u=parent[u])path.push_back(u);
        reverse(path.begin(),path.end());
        for(int k=H-1;k>=0;--k) {
            int x=v,y=v;
            if(k<depth[v]) {
                int a=path[k],b=path[k+1];
                x=edge[v][b]&&!edge[v][a]?b:a;
                y=edge[v][a]?v:a;
            }
            row[v].push_back(x);row[v].push_back(y);
        }
        row[v].erase(row[v].begin());row[v].push_back(root);
    }
    vector<int> next(N,-1);
    for(int v=endpoint;parent[v]!=-1;v=parent[v])next[parent[v]]=v;
    vector<int> walk;
    function<void(int)> tour=[&](int u) {
        walk.push_back(u);
        for(int v:child[u])if(v!=next[u]){tour(v);walk.push_back(u);}
        if(next[u]!=-1)tour(next[u]);
    };
    tour(root);
    int L=int(walk.size());
    assert(L==2*N-1-H);
    int K=max(w,(L+w-1)/2),prefix=L-K,T=K-w;
    if(prefix<2) return {};
    vector<bool> inPrefix(N);
    for(int i=0;i<prefix;++i)inPrefix[walk[i]]=true;
    vector<int> repair;
    for(int v:adj[root])if(!inPrefix[v])repair.push_back(v);
    if(int(repair.size())>T*(T+1)/2) return {};
    vector<vector<int>> ans(K,vector<int>(K,root));
    // The final two columns are root-filled globally. This only removes
    // root edges and tree-edge copies; tree edges remain in the first column.
    for(int r=0;r<K;++r)
        for(int c=0;c<w-2;++c)
            ans[r][c]=row[walk[prefix+r]][c];
    // Rotate the prefix counterclockwise and overlap two columns.
    for(int r=0;r<w;++r)
        for(int c=w-2;c<K;++c)
            ans[T+r][c]=row[walk[min(c-w+2,prefix-1)]][w-1-r];
    // Column w-2 is a root guard. Below this rectangle is a root row.
    int used=0;
    for(int r=0;r<T;++r)for(int c=w-1;c<K;++c)
        if(((r+c-(w-1))&1)==0&&used<int(repair.size()))
            ans[r][c]=repair[used++];
    assert(used==int(repair.size()));
    for(auto& line:ans)for(int& v:line)++v;
    return ans;
}


// Three-column truncation and deletion of the initial root row.
vector<vector<int>> worldmap_B3(int N, int M, vector<int> A, vector<int> B) {
    if (N == 1) return {{1}};
    vector<vector<int>> adj(N),child(N);
    vector<vector<bool>> edge(N,vector<bool>(N));
    for(int i=0;i<M;++i){int u=A[i]-1,v=B[i]-1;adj[u].push_back(v);adj[v].push_back(u);edge[u][v]=edge[v][u]=true;}
    // A non-articulation DFS root has exactly one child. The least degree
    // among such vertices is at most any DFS leaf's degree, hence <= H.
    const int root=0; // Wrapper guarantees that deleting this root leaves a connected graph.
    vector<int>parent(N,-1),depth(N),seen(N);
    function<void(int)>dfs=[&](int u){seen[u]=1;for(int v:adj[u])if(!seen[v]){parent[v]=u;depth[v]=depth[u]+1;child[u].push_back(v);dfs(v);}};
    dfs(root);
    int endpoint=int(max_element(depth.begin(),depth.end())-depth.begin());
    int H=depth[endpoint],w=2*H;
    if(H<2||child[root].size()!=1)return{};
    int firstchild=child[root][0];
    vector<vector<int>>row(N);
    for(int v=0;v<N;++v){
        vector<int>path;for(int u=v;u!=-1;u=parent[u])path.push_back(u);reverse(path.begin(),path.end());
        for(int k=H-1;k>=0;--k){int x=v,y=v;if(k<depth[v]){int a=path[k],b=path[k+1];x=edge[v][b]&&!edge[v][a]?b:a;y=edge[v][a]?v:a;}row[v].push_back(x);row[v].push_back(y);}
        row[v].erase(row[v].begin());row[v].push_back(root);
    }
    vector<int>next(N,-1);for(int v=endpoint;parent[v]!=-1;v=parent[v])next[parent[v]]=v;
    vector<int>walk;function<void(int)>tour=[&](int u){walk.push_back(u);for(int v:child[u])if(v!=next[u]){tour(v);walk.push_back(u);}if(next[u]!=-1)tour(next[u]);};tour(root);
    int L=int(walk.size()),K=max(w,N-2+H/2),prefix=L-K,T=K-w;
    if(prefix<2)return{};
    assert(walk[1]==firstchild);
    vector<vector<int>>ans(K,vector<int>(K,root));
    for(int r=0;r<K;++r){
        for(int c=0;c<w-3;++c)ans[r][c]=row[walk[prefix+r]][c];
        ans[r][w-3]=firstchild;
    }
    for(int r=0;r<w;++r)for(int c=w-3;c<K;++c){
        int i=min(c-w+4,prefix-1);
        ans[T+r][c]=i==1?firstchild:row[walk[i]][w-1-r];
    }
    vector<bool> rootSeen(N);
    for(int r=0;r<K;++r)for(int c=0;c<K;++c){
        int u=ans[r][c];
        auto record=[&](int v){if(u==root)rootSeen[v]=true;if(v==root)rootSeen[u]=true;};
        if(r+1<K)record(ans[r+1][c]);
        if(c+1<K)record(ans[r][c+1]);
    }
    vector<int>repair;
    for(int v:adj[root])if(!rootSeen[v])repair.push_back(v);
    if(int(repair.size())>T*(T+1)/2)return{};
    // If T >= 1, the root guard next to the child guard already supplies
    // the root-child edge, hence at most degree(root)-1 repairs are needed.
    int used=0;
    for(int r=0;r<T;++r)for(int c=w-1;c<K;++c)
        if(((r+c-(w-1))&1)==0&&used<int(repair.size()))ans[r][c]=repair[used++];
    assert(used==int(repair.size()));
    for(auto&line:ans)for(int&v:line)++v;
    return ans;
}


vector<vector<int>> worldmap125_A(int N, int M, vector<int> A, vector<int> B);

// Produce a path on at least min(2*delta+1,N) vertices, then force the DFS
// of constructor A to start with this path. Input/output colors are 1-based.
vector<vector<int>> worldmap_dirac(int N, int M, vector<int> A, vector<int> B, int delta) {
    if (N == 1) return {{1}};
    vector<vector<int>> adj(N);
    vector<vector<bool>> edge(N, vector<bool>(N));
    for (int i = 0; i < M; ++i) {
        int u = A[i] - 1, v = B[i] - 1;
        adj[u].push_back(v); adj[v].push_back(u);
        edge[u][v] = edge[v][u] = true;
    }
    // delta is the minimum degree among NON-ARTICULATION vertices.
    assert(delta >= 1);
    const int target = min(2 * delta + 1, N);
    vector<int> path(1, 0);
    vector<bool> inPath(N);
    inPath[0] = true;
    while (int(path.size()) < target) {
        int append = -1;
        for (int v : adj[path.back()]) if (!inPath[v]) { append = v; break; }
        if (append != -1) {
            path.push_back(append); inPath[append] = true;
            continue;
        }
        int prepend = -1;
        for (int v : adj[path.front()]) if (!inPath[v]) { prepend = v; break; }
        if (prepend != -1) {
            path.insert(path.begin(), prepend); inPath[prepend] = true;
            continue;
        }
        // Both endpoints have all their neighbors inside the path.
        // Each endpoint is therefore non-articulation: all its neighbors
        // remain connected by the rest of the path after removing it.
        // Two subsets of k-1 possible splice positions each have size at
        // least delta, so when k <= 2*delta they must intersect.
        int k = int(path.size()), pivot = -1;
        assert(k <= 2 * delta && k < N);
        for (int i = 0; i + 1 < k; ++i) {
            if (edge[path.front()][path[i + 1]] && edge[path.back()][path[i]]) {
                pivot = i;
                break;
            }
        }
        assert(pivot != -1);
        vector<int> cycle;
        for (int i = 0; i <= pivot; ++i) cycle.push_back(path[i]);
        for (int i = k - 1; i > pivot; --i) cycle.push_back(path[i]);
        for (int i = 0; i < k; ++i) assert(edge[cycle[i]][cycle[(i+1)%k]]);
        int cut = -1, outside = -1;
        for (int i = 0; i < k && outside == -1; ++i) {
            for (int v : adj[cycle[i]]) if (!inPath[v]) {
                cut = i; outside = v;
                break;
            }
        }
        // The graph is connected and the cycle does not contain all vertices.
        assert(outside != -1);
        path.assign(1, outside);
        for (int i = 0; i < k; ++i) path.push_back(cycle[(cut+i)%k]);
        inPath[outside] = true;
    }
    assert(int(path.size()) >= target);
    vector<bool> distinct(N);
    for (int i = 0; i < int(path.size()); ++i) {
        assert(!distinct[path[i]]);
        distinct[path[i]] = true;
        if (i) assert(edge[path[i-1]][path[i]]);
    }
    vector<int> newToOld = path;
    for (int u = 0; u < N; ++u) if (!inPath[u]) newToOld.push_back(u);
    vector<int> oldToNew(N);
    for (int i = 0; i < N; ++i) oldToNew[newToOld[i]] = i;
    vector<int> reorderedA, reorderedB;
    vector<vector<bool>> inserted(N, vector<bool>(N));
    auto add = [&](int u, int v) {
        if (inserted[u][v]) return;
        inserted[u][v] = inserted[v][u] = true;
        reorderedA.push_back(oldToNew[u] + 1);
        reorderedB.push_back(oldToNew[v] + 1);
    };
    // At every path vertex, the predecessor is already visited and the
    // successor is the first unvisited neighbor in A's DFS adjacency order.
    for (int i = 1; i < int(path.size()); ++i) add(path[i-1], path[i]);
    for (int i = 0; i < M; ++i) add(A[i]-1, B[i]-1);
    assert(int(reorderedA.size()) == M);
    auto ans = worldmap125_A(N, M, reorderedA, reorderedB);
    int guaranteedHeight = min(2 * delta, N - 1);
    assert(int(ans.size()) <= (3 * N - guaranteedHeight - 1) / 2);
    for (auto& row : ans) for (int& color : row) color = newToOld[color-1] + 1;
    return ans;
}
} // namespace

vector<vector<int>> create_map(int N, int M, vector<int> A, vector<int> B) {
    if (N == 1) return {{1}};
    vector<vector<int>> adj(N);
    for (int i=0;i<M;++i) {
        adj[A[i]-1].push_back(B[i]-1);
        adj[B[i]-1].push_back(A[i]-1);
    }

    // N <= 40: directly test connectivity after deleting each vertex.
    int root=-1;
    for (int removed=0;removed<N;++removed) {
        vector<int> seen(N), queue{removed==0 ? 1 : 0};
        seen[queue[0]]=1;
        for (int i=0;i<(int)queue.size();++i)
            for (int v:adj[queue[i]]) if (v!=removed && !seen[v]) {
                seen[v]=1;queue.push_back(v);
            }
        if ((int)queue.size()==N-1 &&
            (root==-1 || adj[removed].size()<adj[root].size())) root=removed;
    }
    assert(root!=-1);
    int delta=(int)adj[root].size();
    vector<int> toOld(N),toNew(N);
    iota(toOld.begin(),toOld.end(),0);
    swap(toOld[0],toOld[root]);
    for(int i=0;i<N;++i)toNew[toOld[i]]=i;
    for(int i=0;i<M;++i){A[i]=toNew[A[i]-1]+1;B[i]=toNew[B[i]-1]+1;}

    auto answer=worldmap125_A(N,M,A,B);
    auto consider=[&](vector<vector<int>> candidate) {
        if(!candidate.empty() && candidate.size()<answer.size())
            answer=move(candidate);
    };
    consider(worldmap125_B(N,M,A,B));
    consider(worldmap_B2(N,M,A,B));
    consider(worldmap_B3(N,M,A,B));
    consider(worldmap_dirac(N,M,A,B,delta));
    assert(37*(int)answer.size()<=45*N);
    for(auto& line:answer)for(int& v:line)v=toOld[v-1]+1;
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int N, M;
        cin >> N >> M;
        vector<int> A(M), B(M);
        for (int i = 0; i < M; ++i) cin >> A[i] >> B[i];
        vector<vector<int>> map = create_map(N, M, A, B);
        cout << map.size() << '\n';
        for (const auto &row : map) {
            for (int j = 0; j < (int)row.size(); ++j) {
                if (j) cout << ' ';
                cout << row[j];
            }
            cout << '\n';
        }
    }
    return 0;
}

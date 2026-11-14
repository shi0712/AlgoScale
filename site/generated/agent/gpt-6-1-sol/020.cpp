#include <bits/stdc++.h>
using namespace std;

/*
dp[u][k] sums valid orders over all k-element subsets of u's subtree.
Color each vertex by its child subtree (u itself is a singleton color).
A valid order consists of a prefix with unequal neighboring colors and,
optionally, a final run of at least two vertices of one child color. The
final run is itself valid. There can be only one such run.

For child-alphabet sums X_i, the generating function for the prefix is
    S = 1 / (1 - sum_i X_i/(1+X_i)).
The full generating function is
    S * (1 + sum_i (F_i-1-X_i)/(1+X_i)).
Only squarefree vertex monomials are extracted. Inclusion-exclusion turns
a size-a ordinary color into R_a(t,z), and a recursive final run into Q_a.
Here z marks omitted vertices, and t marks blocks. Replacing t^j by j!
orders those blocks. PolynomialWork implements these transforms.

Process a largest child first; merging a remaining size-b child costs
O(b*s^2), where s is the current subtree size. The costs over the tree
are O(n^3): sum_u s_u^2*(s_u-largest_child_size) <= n^3. The specialized
recurrences obey the same bound. Space usage is O(n^2).
*/

static constexpr int MOD = 1000000007;
int n;
vector<vector<int>> adj;
vector<int> sz;
vector<vector<int>> dp;
vector<int> fac, ifac;

int power(int a, int b) {
    int r = 1;
    while (b) {
        if (b & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        b >>= 1;
    }
    return r;
}
int choose(int a, int b) {
    if (b < 0 || b > a) return 0;
    return (long long)fac[a] * ifac[b] % MOD * ifac[a-b] % MOD;
}
int falling(int a, int b) {
    return (long long)fac[a] * ifac[a-b] % MOD;
}
void addmod(int &a, int b) {
    a += b;
    if (a >= MOD) a -= MOD;
}

// All singletons have the same color as themselves, and hence never conflict.
// There is just one other color, whose final run can be a recursive sequence.
void one_child(int u, int v, int c, int low) {
    int a = sz[v];
    auto &f = dp[u];
    for (int r = 0; r <= c; ++r) {
        int single = falling(c, r);
        for (int t = max(0, low-r); t <= min(a, r+1); ++t) {
            int val = (long long)single * choose(r+1, t) % MOD * falling(a, t) % MOD;
            addmod(f[r+t], val);
        }
    }
    for (int k = 2; k <= a; ++k) if (dp[v][k]) {
        for (int r = 0; r <= c; ++r) {
            int base = (long long)dp[v][k] * falling(c, r) % MOD;
            for (int t = max(0, low-k-r); t <= min(r, a-k); ++t) {
                int val = (long long)base * choose(r, t) % MOD * falling(a-k, t) % MOD;
                addmod(f[k+r+t], val);
            }
        }
    }
}

// The only colors are the two children and the root singleton. Alternating
// two-color words have explicit coefficients, including one root separator.
void two_children(int u, int va, int vb, int low) {
    int a = sz[va], b = sz[vb];
    auto &f = dp[u];
    for (int i = 0; i <= a; ++i) {
        for (int delta = -2; delta <= 2; ++delta) {
            int j = i + delta;
            if (j < 0 || j > b) continue;
            int base = (long long)falling(a, i) * falling(b, j) % MOD;
            int w0 = 0, w1 = 0;
            if (delta == 0) {
                w0 = i ? 2 : 1;
                w1 = i ? 6*i : 1;
            } else if (delta == -1) { w0 = 1; w1 = 4*i-2; }
            else if (delta == 1) { w0 = 1; w1 = 4*i+2; }
            else if (delta == -2) w1 = i-1;
            else w1 = i+1;
            if (i+j >= low && w0) addmod(f[i+j], (long long)base*w0%MOD);
            if (i+j+1 >= low && w1) addmod(f[i+j+1], (long long)base*w1%MOD);
        }
    }
    auto tail = [&](int v, int x, int y) {
        for (int k = 2; k <= x; ++k) if (dp[v][k]) {
            for (int i = 0; i <= min(x-k, y+1); ++i) {
                int base = (long long)dp[v][k] * falling(x-k, i) % MOD;
                for (int delta = -1; delta <= 2; ++delta) {
                    int j = i + delta;
                    if (j < 0 || j > y) continue;
                    int val = (long long)base * falling(y, j) % MOD;
                    if ((delta == 0 || delta == 1) && k+i+j >= low)
                        addmod(f[k+i+j], val);
                    int w;
                    if (delta == -1) w = i;
                    else if (delta == 0) w = 3*i+1;
                    else if (delta == 1) w = 3*i+2;
                    else w = i+1;
                    if (k+i+j+1 >= low && w)
                        addmod(f[k+i+j+1], (long long)val*w%MOD);
                }
            }
        }
    };
    tail(va, a, b);
    tail(vb, b, a);
}

// R_m(t,z) = m! [x^m] exp(z*x + t*x/(1+x)).
// All arrays below store the coefficient of z^r*t^j at r*width+j.
struct PolynomialWork {
    int width, cap, length;
    PolynomialWork(int degree, int limit): width(degree+1), cap(min(degree,limit)),
        length(width*(cap+1)) {}

    vector<int> regular(int a) {
        vector<int> p(length);
        if (a <= cap) p[a*width] = 1;
        for (int r = 0; r <= min(cap,a-1); ++r) {
            int base = (long long)fac[a] * ifac[r] % MOD;
            for (int j = 1; j <= a-r; ++j) {
                int val = (long long)base * ifac[j] % MOD * choose(a-r-1,j-1) % MOD;
                if ((a-r-j)&1) val = val ? MOD-val : 0;
                p[r*width+j] = val;
            }
        }
        return p;
    }

    // Q_v = sum_{k>=2} dp[v][k] (a-k)! [x^(a-k)]
    //                       exp(z*x+t*x/(1+x))/(1+x).
    vector<int> special(int v) {
        int a = sz[v], m = min(a-2,n-a);
        vector<int> p(length), next(m+2), cur(m+2);
        for (int r = m; r >= 0; --r) {
            int h = (long long)dp[v][a-r] * fac[r] % MOD;
            cur[0] = h-next[0];
            if (cur[0] < 0) cur[0] += MOD;
            for (int j = 1; j <= m-r; ++j) {
                cur[j] = next[j-1]-next[j];
                if (cur[j] < 0) cur[j] += MOD;
            }
            cur[m-r+1] = 0;
            if (r <= cap) {
                for (int j = 0; j <= m-r; ++j)
                    p[r*width+j] = (long long)cur[j]*ifac[r]%MOD*ifac[j]%MOD;
            }
            swap(cur,next);
        }
        return p;
    }

    struct Result { vector<int> ordinary, special; };

    // Multiply p by R_b, and optionally also by Q_v. The three-term
    // recurrence for R_b avoids a four-dimensional polynomial convolution.
    Result multiply(vector<int> p, int degree, int b, int v = -1) {
        vector<int> prev(length), prev2(length), out(length);
        vector<int> q, uprime;
        int qm = -1;
        if (v != -1) {
            qm = min(b-2,n-b);
            q.assign(length,0);
            uprime = p; // p*R'_0, where R'_j=R_j-j*R'_(j-1).
            int f = dp[v][b];
            for (int r = 0; r <= min(cap,degree); ++r)
                for (int j = 0; j <= degree-r; ++j) {
                    int idx = r*width+j;
                    q[idx] = (long long)p[idx]*f%MOD;
                }
        }
        for (int step = 0; step < b; ++step) {
            int nd = degree+step+1;
            long long c1 = 2LL*step, c2 = 1LL*step*(step-1);
            for (int r = 0; r <= min(cap,nd); ++r) {
                int off = r*width;
                for (int j = 0; j <= nd-r; ++j) {
                    int idx = off+j;
                    long long val = (j ? p[idx-1] : 0);
                    val -= c1*p[idx] + c2*prev[idx];
                    if (r) val += p[idx-width] + c1*prev[idx-width] + c2*prev2[idx-width];
                    val %= MOD;
                    if (val < 0) val += MOD;
                    out[idx] = val;
                }
            }
            prev2.swap(prev);
            prev.swap(p);
            p.swap(out);
            int k = step+1;
            if (k <= qm) {
                int f = dp[v][b-k];
                for (int r = 0; r <= min(cap,degree+k); ++r) {
                    int off = r*width;
                    for (int j = 0; j <= degree+k-r; ++j) {
                        int idx = off+j;
                        long long val = p[idx]-1LL*k*uprime[idx];
                        val %= MOD;
                        if (val < 0) val += MOD;
                        uprime[idx] = val;
                        addmod(q[idx], val*f%MOD);
                    }
                }
            }
        }
        return {move(p),move(q)};
    }
};

void general_case(int u, vector<int> child, int singles, int low) {
    sort(child.begin(), child.end(), [](int x, int y) { return sz[x] > sz[y]; });
    int total = sz[u]-singles;
    int cap = sz[u]-low;
    PolynomialWork work(total,cap);
    int v = child[0], da = sz[v], db = min(da-2,n-da);
    vector<int> a = work.regular(da), b = work.special(v);
    for (int pos = 1; pos < (int)child.size(); ++pos) {
        v = child[pos];
        int s = sz[v], m = min(s-2,n-s);
        auto ar = work.multiply(move(a), da, s, v);
        auto br = work.multiply(move(b), db, s);
        db = max(db+s, da+m);
        da += s;
        a = move(ar.ordinary);
        b = move(br.ordinary);
        for (int r = 0; r <= min(work.cap,db); ++r)
            for (int j = 0; j <= db-r; ++j) {
                int idx = r*work.width+j;
                addmod(b[idx],ar.special[idx]);
            }
    }
    for (int i = 0; i < work.length; ++i) addmod(a[i],b[i]);
    // Each singleton contributes t+z. Integrating t^j against e^(-t)
    // replaces it by j!, which orders the inclusion-exclusion blocks.
    for (int r = 0; r <= work.cap; ++r) {
        for (int omitted = 0; omitted <= min(singles,cap-r); ++omitted) {
            int sum = 0, shift = singles-omitted;
            for (int j = 0; j <= total-r; ++j)
                addmod(sum, (long long)a[r*work.width+j]*fac[j+shift]%MOD);
            int val = (long long)sum*choose(singles,omitted)%MOD;
            addmod(dp[u][sz[u]-r-omitted],val);
        }
    }
}

void dfs(int u, int parent) {
    sz[u] = 1;
    vector<int> nonleaf;
    int singles = 1;
    for (int v : adj[u]) if (v != parent) {
        dfs(v,u);
        sz[u] += sz[v];
        if (sz[v] == 1) ++singles;
        else nonleaf.push_back(v);
    }
    int s = sz[u];
    dp[u].assign(s+1,0);
    dp[u][0] = 1;
    dp[u][1] = s;
    if (s == 1) return;
    // Before the final run inside this subtree, its vertices must be
    // separated by vertices outside it. At most n-s vertices can be omitted.
    int low = max(2,2*s-n);
    if (nonleaf.empty()) {
        for (int k = low; k <= s; ++k) dp[u][k] = falling(s,k);
    } else if (nonleaf.size() == 1) {
        one_child(u,nonleaf[0],singles,low);
    } else if (nonleaf.size() == 2 && singles == 1) {
        two_children(u,nonleaf[0],nonleaf[1],low);
    } else {
        general_case(u,nonleaf,singles,low);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> n;
    adj.resize(n);
    sz.resize(n);
    dp.resize(n);
    for (int i = 1; i < n; ++i) {
        int x,y;
        cin >> x >> y;
        --x; --y;
        adj[x].push_back(y);
        adj[y].push_back(x);
    }
    fac.resize(n+2); ifac.resize(n+2);
    fac[0] = 1;
    for (int i = 1; i <= n+1; ++i) fac[i] = (long long)fac[i-1]*i%MOD;
    ifac[n+1] = power(fac[n+1],MOD-2);
    for (int i = n+1; i; --i) ifac[i-1] = (long long)ifac[i]*i%MOD;
    dfs(0,-1);
    cout << dp[0][n] << '\n';
}

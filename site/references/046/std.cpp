#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int add_mod(int a, int b) {
    a += b;
    return a >= MOD ? a - MOD : a;
}

int sub_mod(int a, int b) {
    a -= b;
    return a < 0 ? a + MOD : a;
}

int mul_mod(int a, int b) {
    return int(1LL * a * b % MOD);
}

int mod_pow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = mul_mod(r, a);
        a = mul_mod(a, a);
        e >>= 1;
    }
    return r;
}

// A node with lc == 0 represents a constant function on its whole interval.
// Otherwise mul/add is a pending affine transformation for the interval.
class FunctionTree {
    struct Node {
        int lc = 0, rc = 0;
        int value = 0;
        int mul = 1, add = 0;
    };

    vector<Node> tr;

    void set_constant(int u, int value) {
        tr[u] = Node{0, 0, value, 1, 0};
    }

    void apply_affine(int u, int mul, int add) {
        if (mul == 0) {
            set_constant(u, add);
        } else if (tr[u].lc == 0) {
            tr[u].value = add_mod(mul_mod(mul, tr[u].value), add);
        } else {
            tr[u].mul = mul_mod(mul, tr[u].mul);
            tr[u].add = add_mod(mul_mod(mul, tr[u].add), add);
        }
    }

    void push(int u) {
        if (tr[u].mul == 1 && tr[u].add == 0) return;
        const int mul = tr[u].mul;
        const int add = tr[u].add;
        apply_affine(tr[u].lc, mul, add);
        apply_affine(tr[u].rc, mul, add);
        tr[u].mul = 1;
        tr[u].add = 0;
    }

    void pull(int u) {
        const int l = tr[u].lc;
        const int r = tr[u].rc;
        if (tr[l].lc == 0 && tr[r].lc == 0 && tr[l].value == tr[r].value) {
            set_constant(u, tr[l].value);
        }
    }

public:
    explicit FunctionTree(int n) {
        tr.reserve(48 * n + 16);
        tr.push_back(Node{});
    }

    int make_constant(int value) {
        tr.push_back(Node{0, 0, value, 1, 0});
        return int(tr.size()) - 1;
    }

    void affine(int u, int mul, int add) {
        apply_affine(u, mul, add);
    }

    // Destructive pointwise multiplication. Both roots are consumed.
    int multiply(int a, int b) {
        if (tr[a].lc == 0) {
            apply_affine(b, tr[a].value, 0);
            return b;
        }
        if (tr[b].lc == 0) {
            apply_affine(a, tr[b].value, 0);
            return a;
        }
        push(a);
        push(b);
        tr[a].lc = multiply(tr[a].lc, tr[b].lc);
        tr[a].rc = multiply(tr[a].rc, tr[b].rc);
        pull(a);
        return a;
    }

    int query(int u, int l, int r, int pos) {
        if (tr[u].lc == 0) return tr[u].value;
        push(u);
        const int mid = (l + r) >> 1;
        if (pos <= mid) return query(tr[u].lc, l, mid, pos);
        return query(tr[u].rc, mid + 1, r, pos);
    }

    void assign_prefix(int u, int l, int r, int right, int value) {
        if (right < l) return;
        if (r <= right) {
            set_constant(u, value);
            return;
        }
        if (tr[u].lc == 0) {
            const int old = tr[u].value;
            const int lc = make_constant(old);
            const int rc = make_constant(old);
            tr[u].lc = lc;
            tr[u].rc = rc;
        } else {
            push(u);
        }
        const int mid = (l + r) >> 1;
        assign_prefix(tr[u].lc, l, mid, right, value);
        if (right > mid) assign_prefix(tr[u].rc, mid + 1, r, right, value);
        pull(u);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;

    vector<vector<int>> children(n + 1);
    vector<int> depth(n + 1), need(n + 1);
    depth[1] = 1;
    for (int u = 2; u <= n; ++u) {
        int parent;
        cin >> parent;
        children[parent].push_back(u);
        depth[u] = depth[parent] + 1;
    }
    for (int i = 0; i < m; ++i) {
        int a, b;
        cin >> a >> b;
        need[b] = max(need[b], depth[a]);
    }

    // values[x] = P(x), where every vertex is independently unselected
    // with the formal probability x.
    vector<int> values(n + 1);
    vector<int> roots(n + 1);
    for (int x = 0; x <= n; ++x) {
        FunctionTree functions(n);
        for (int u = n; u >= 1; --u) {
            int root = functions.make_constant(1);
            for (int v : children[u]) {
                root = functions.multiply(root, roots[v]);
            }

            const int selected_part = mul_mod(
                sub_mod(1, x),
                functions.query(root, 0, n, depth[u])
            );
            functions.affine(root, x, selected_part);
            if (need[u] != 0) {
                functions.assign_prefix(root, 0, n, need[u] - 1, selected_part);
            }
            roots[u] = root;
        }
        values[x] = functions.query(roots[1], 0, n, 0);
    }

    // W(X) = product_{i=0}^n (X-i), low degree first.
    vector<int> product(n + 2);
    product[0] = 1;
    for (int i = 0; i <= n; ++i) {
        for (int j = i + 1; j >= 0; --j) {
            product[j] = sub_mod(j == 0 ? 0 : product[j - 1],
                                 mul_mod(i, product[j]));
        }
    }

    vector<int> factorial(n + 1, 1), inv_factorial(n + 1, 1);
    for (int i = 1; i <= n; ++i) factorial[i] = mul_mod(factorial[i - 1], i);
    inv_factorial[n] = mod_pow(factorial[n], MOD - 2);
    for (int i = n; i >= 1; --i) {
        inv_factorial[i - 1] = mul_mod(inv_factorial[i], i);
    }

    // Lagrange interpolation at 0,1,...,n.
    vector<int> coefficient(n + 1), quotient(n + 1);
    for (int i = 0; i <= n; ++i) {
        quotient[n] = product[n + 1];
        for (int j = n - 1; j >= 0; --j) {
            quotient[j] = add_mod(product[j + 1], mul_mod(i, quotient[j + 1]));
        }

        int scale = mul_mod(values[i],
                            mul_mod(inv_factorial[i], inv_factorial[n - i]));
        if ((n - i) & 1) scale = sub_mod(0, scale);
        for (int j = 0; j <= n; ++j) {
            coefficient[j] = add_mod(coefficient[j], mul_mod(scale, quotient[j]));
        }
    }

    int integral = 0;
    for (int k = 1; k <= n; ++k) {
        const int inv_k = mul_mod(factorial[k - 1], inv_factorial[k]);
        integral = add_mod(integral, mul_mod(coefficient[k], inv_k));
    }
    cout << mul_mod(n, sub_mod(0, integral)) << '\n';
    return 0;
}

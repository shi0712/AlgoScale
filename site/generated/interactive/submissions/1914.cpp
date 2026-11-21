#include <bits/stdc++.h>
using namespace std;

using ll = long long;
constexpr ll U = 1000000000LL;
constexpr ll INF = 1LL << 55;

int ask(int a, int b, int c) {
    cout << "? " << a << ' ' << b << ' ' << c << endl;
    int x;
    if (!(cin >> x) || x < 0) exit(0);
    return x;
}

struct Model {
    // Nodes 2*i and 2*i+1 represent x[i] and -x[i].
    // d[i][j] bounds the difference of the corresponding nodes.
    ll d[10][10];
    array<int, 5> rank;

    Model(array<int, 5> p) {
        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < 10; ++j)
                d[i][j] = i == j ? 0 : INF;
        for (int i = 0; i < 5; ++i) {
            rank[p[i]] = i;
            d[2*i][2*i+1] = 2*U;
            d[2*i+1][2*i] = 0;
            if (i) bound(2*p[i-1], 2*p[i], 0);
        }
    }

    void bound(int i, int j, ll v) {
        d[i][j] = min(d[i][j], v);
        d[j^1][i^1] = min(d[j^1][i^1], v);
    }

    static ll half(ll x) { return x >= 0 ? x/2 : (x-1)/2; }

    bool close() {
        for (int k = 0; k < 10; ++k) {
            for (int i = 0; i < 10; ++i)
                for (int j = 0; j < 10; ++j)
                    d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
            for (int i = 0; i < 10; ++i)
                if (d[i][i] < 0) return false;
        }
        // Integer tight closure for constraints of the form +/-x +/-y <= b.
        ll h[10];
        for (int i = 0; i < 10; ++i) h[i] = half(d[i][i^1]);
        for (int i = 0; i < 10; ++i)
            if (h[i] + h[i^1] < 0) return false;
        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < 10; ++j)
                d[i][j] = min(d[i][j], h[i] + h[j^1]);
        return true;
    }

    pair<int,int> ends(array<int,3> t) const {
        int a = t[0], b = t[0];
        for (int i : t) {
            if (rank[i] < rank[a]) a = i;
            if (rank[i] > rank[b]) b = i;
        }
        return {2*a, 2*b+1};
    }
};

array<int,5> seed(const vector<int>& id) {
    array<int,5> p{0,1,2,3,4};
    vector<Model> models;
    do {
        models.emplace_back(p);
        models.back().close();
    } while (next_permutation(p.begin(), p.end()));

    vector<array<int,3>> triples;
    for (int a = 0; a < 5; ++a)
        for (int b = a+1; b < 5; ++b)
            for (int c = b+1; c < 5; ++c)
                triples.push_back({a,b,c});

    while (!models.empty()) {
        array<int,5> ans{};
        bool done = true;
        for (int i = 0; i < 5; ++i) {
            ll lo = U, hi = 0;
            for (auto& m : models) {
                lo = min(lo, -m.d[2*i+1][2*i]/2);
                hi = max(hi, m.d[2*i][2*i+1]/2);
            }
            ans[i] = lo;
            if (lo != hi) done = false;
        }
        if (done) return ans;

        int best = -1;
        double merit = -1e30;
        for (int k = 0; k < (int)triples.size(); ++k) {
            vector<pair<ll,double>> events;
            double gain = 0, conditional = 0;
            ll mn = 2*U, mx = 0;
            for (auto& m : models) {
                auto [i,j] = m.ends(triples[k]);
                ll l = -m.d[j][i], r = m.d[i][j];
                mn = min(mn,l);
                mx = max(mx,r);
                double width = r-l+1;
                double density = 1.0 / (width * models.size());
                events.emplace_back(l, density);
                events.emplace_back(r+1, -density);
                conditional += log2(width);
                gain += l != r;
            }
            if (mn == mx) continue;
            // Prefer gaining a new equation and separating feasible orders.
            sort(events.begin(), events.end());
            double density = 0, entropy = 0;
            ll prev = events.front().first;
            for (auto [v, delta] : events) {
                if (v != prev && density > 0)
                    entropy -= (v-prev)*density*log2(density);
                density += delta;
                prev = v;
            }
            double score = gain/models.size()
                         + 0.45*(entropy - conditional/models.size());
            if (score > merit + 1e-9) merit = score, best = k;
        }
        if (best < 0) exit(0);
        auto t = triples[best];
        triples.erase(triples.begin()+best);
        int v = ask(id[t[0]], id[t[1]], id[t[2]]);
        vector<Model> next;
        for (auto& m : models) {
            auto [i,j] = m.ends(t);
            if (v < -m.d[j][i] || v > m.d[i][j]) continue;
            m.bound(i,j,v);
            m.bound(j,i,-v);
            if (m.close()) next.push_back(m);
        }
        models.swap(next);
    }
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    vector<int> id(n), ans(n+1,-1), known;
    iota(id.begin(), id.end(), 1);
    shuffle(id.begin(), id.end(), rng);
    int zeros = 0, tops = 0;
    auto record = [&](int i, int x) {
        ans[i] = x;
        zeros += x == 0;
        tops += x == U;
        known.insert(lower_bound(known.begin(), known.end(), i,
            [&](int a,int b){ return pair(ans[a],a) < pair(ans[b],b); }), i);
    };
    auto initial = seed(id);
    for (int i = 0; i < 5; ++i) record(id[i], initial[i]);

    auto learn = [&](int i, int low, int high) {
        int m = known.size();
        while (low < high) {
            // A query with known endpoints is invertible except on the
            // intersection of their interval with the remaining possibilities.
            int best = INT_MAX, a = -1, b = -1, ties = 0;
            for (int j = 1; j < m; ++j) {
                int l = known[j-1], r = known[j];
                int ambiguity = max(0, min(high, ans[r])-max(low, ans[l]));
                if (ambiguity < best) best = ambiguity, ties = 0;
                if (ambiguity == best) {
                    ++ties;
                    if (ties == 1 || rng()%ties == 0) a = l, b = r;
                }
                if (best == 0) break;
            }
            int q = ask(i,a,b), s = ans[a]+ans[b];
            if (q < s) low = high = q-ans[b];
            else if (q > s) low = high = q-ans[a];
            else low = max(low,ans[a]), high = min(high,ans[b]);
        }
        record(i,low);
    };

    for (int k = 5; k < n;) {
        if (k+3 <= n && 3*max(zeros,tops) >= 2*k) {
            // Extreme answers certify all three labels at once.
            int a = id[k++], b = id[k++], c = id[k++];
            int q = ask(a,b,c);
            int l = max(0, q-(int)U), r = min((int)U,q);
            learn(a,l,r);
            learn(b,l,r);
            int x = min(ans[a],ans[b]), y = max(ans[a],ans[b]);
            if (q < x+y) record(c,q-y);
            else if (q > x+y) record(c,q-x);
            else learn(c,x,y);
        } else {
            learn(id[k++],0,U);
        }
    }
    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << ans[i];
    cout << endl;
}

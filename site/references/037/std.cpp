#include <bits/stdc++.h>
using namespace std;
using int64 = long long;
using u64 = uint64_t;
using u32 = uint32_t;

// Exact hash-consed persistent segment tree.
// Internal node ID is a hash-table slot + 1 (< 2^31).
// Leaf ID has the high bit set and stores the frequency in the low 31 bits.
class CanonicalPST {
    static constexpr u32 LEAF = 0x80000000u;
    vector<u64> tab; // packed(left,right)+1, 0 means empty
    size_t mask_ = 0;
    int H = 0;
    u32 zeroRoot_ = 0;

    static u64 mix(u64 x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    static u64 pack(u32 l, u32 r) {
        return (u64(l) << 32) | u64(r);
    }

    u32 intern(u32 l, u32 r) {
        const u64 raw = pack(l, r);
        const u64 stored = raw + 1; // raw can never be UINT64_MAX here
        size_t p = size_t(mix(raw)) & mask_;
        while (true) {
            if (tab[p] == 0) {
                tab[p] = stored;
                return u32(p + 1);
            }
            if (tab[p] == stored) return u32(p + 1);
            p = (p + 1) & mask_;
        }
    }

    pair<u32,u32> children(u32 id) const {
        const u64 raw = tab[size_t(id - 1)] - 1;
        return {u32(raw >> 32), u32(raw)};
    }

    static u32 leaf(int cnt) {
        return LEAF | u32(cnt);
    }
    static int leafCnt(u32 id) {
        return int(id & ~LEAF);
    }

    u32 addRec(u32 root, int pos, int level) {
        if (level == 0) return leaf(leafCnt(root) + 1);
        auto [l, r] = children(root);
        const int bit = (pos >> (level - 1)) & 1;
        if (bit == 0) l = addRec(l, pos, level - 1);
        else r = addRec(r, pos, level - 1);
        return intern(l, r);
    }

    // Compare (A + [pa]) and (B + [pb]) lexicographically by frequency,
    // scanning larger coordinates first. pa/pb == -1 means no insertion.
    int cmpVirtualRec(u32 A, int pa, u32 B, int pb, int level) const {
        if (A == B && pa == pb) return 0;
        if (level == 0) {
            int ca = leafCnt(A) + (pa != -1);
            int cb = leafCnt(B) + (pb != -1);
            return (ca > cb) - (ca < cb);
        }
        auto [al, ar] = children(A);
        auto [bl, br] = children(B);
        const int bitpos = level - 1;
        int par = (pa != -1 && ((pa >> bitpos) & 1)) ? pa : -1;
        int pbr = (pb != -1 && ((pb >> bitpos) & 1)) ? pb : -1;
        if (!(ar == br && par == pbr)) {
            int t = cmpVirtualRec(ar, par, br, pbr, level - 1);
            if (t) return t;
        }
        int pal = (pa != -1 && !((pa >> bitpos) & 1)) ? pa : -1;
        int pbl = (pb != -1 && !((pb >> bitpos) & 1)) ? pb : -1;
        if (al == bl && pal == pbl) return 0;
        return cmpVirtualRec(al, pal, bl, pbl, level - 1);
    }

    void collectRec(u32 root, int level, int base, const vector<int64>& vals,
                    vector<int64>& out) const {
        if (level == 0) {
            if (base >= (int)vals.size()) return;
            int c = leafCnt(root);
            while (c--) out.push_back(vals[base]);
            return;
        }
        auto [l, r] = children(root);
        int half = 1 << (level - 1);
        // Larger coordinates first.
        collectRec(r, level - 1, base + half, vals, out);
        collectRec(l, level - 1, base, vals, out);
    }

public:
    CanonicalPST() = default;

    void init(int coordCount, int K) {
        H = 0;
        int P = 1;
        while (P < coordCount) P <<= 1, ++H;
        // If coordCount == 1, H = 0 and the root itself is a leaf.
        long long maxNew = 2LL * K * max(1, H) + H + 32;
        // Keep the open-addressing table below ~70% load.
        size_t cap = 1;
        while ((long double)cap * 0.70L < (long double)maxNew) cap <<= 1;
        if (cap >= (1ULL << 31)) throw runtime_error("canonical table too large");
        tab.assign(cap, 0);
        mask_ = cap - 1;

        u32 z = leaf(0);
        for (int lv = 1; lv <= H; ++lv) z = intern(z, z);
        zeroRoot_ = z;
    }

    u32 zeroRoot() const { return zeroRoot_; }

    u32 add(u32 root, int pos) {
        if (pos < 0) return root;
        return addRec(root, pos, H);
    }

    int cmp(u32 A, u32 B) const {
        return cmpVirtualRec(A, -1, B, -1, H);
    }

    int cmpCandidate(u32 A, int pa, u32 B, int pb) const {
        return cmpVirtualRec(A, pa, B, pb, H);
    }

    void collect(u32 root, const vector<int64>& vals, vector<int64>& out) const {
        collectRec(root, H, 0, vals, out);
    }
};

static vector<int64> solveCase(vector<pair<int64,int64>> seg) {
    sort(seg.begin(), seg.end(), [](auto a, auto b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second > b.second;
    });

    vector<int64> len;
    vector<int64> dl(1, 0); // dl[i] for kept interval i, dl[0]=0
    int64 lastR = -1;
    for (auto [l, r] : seg) {
        if (r <= lastR) continue; // contained in a previously kept interval
        int64 ov = len.empty() ? 0 : max<int64>(0, lastR - l + 1);
        len.push_back(r - l + 1);
        if (len.size() >= 2) dl.push_back(ov);
        lastR = r;
    }
    const int K = (int)len.size();
    dl.push_back(0); // right overlap of the final interval

    vector<int64> vals;
    vals.reserve(4LL * K);
    for (int i = 0; i < K; ++i) {
        int64 L = len[i], a = dl[i], b = dl[i + 1];
        int64 cand[4] = {L - a, L - a - b, L, L - b};
        for (int z = 0; z < 4; ++z) if (cand[z] > 0) vals.push_back(cand[z]);
    }
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());

    CanonicalPST pst;
    pst.init((int)vals.size(), K);
    auto pos = [&](int64 x) -> int {
        if (x <= 0) return -1;
        return int(lower_bound(vals.begin(), vals.end(), x) - vals.begin());
    };

    u32 dp0 = pst.zeroRoot(), dp1 = pst.zeroRoot();
    for (int i = 0; i < K; ++i) {
        const int64 L = len[i], left = dl[i], right = dl[i + 1];

        // ndp0 = max(dp0 + (L-left), dp1 + L)
        int p00 = pos(L - left), p10 = pos(L);
        bool take00 = pst.cmpCandidate(dp0, p00, dp1, p10) >= 0;
        u32 ndp0 = take00 ? pst.add(dp0, p00) : pst.add(dp1, p10);

        // ndp1 = max(dp0 + (L-left-right), dp1 + (L-right))
        int p01 = pos(L - left - right), p11 = pos(L - right);
        bool take01 = pst.cmpCandidate(dp0, p01, dp1, p11) >= 0;
        u32 ndp1 = take01 ? pst.add(dp0, p01) : pst.add(dp1, p11);

        dp0 = ndp0;
        dp1 = ndp1;
    }

    u32 root = pst.cmp(dp0, dp1) >= 0 ? dp0 : dp1;
    vector<int64> ans;
    ans.reserve(K);
    pst.collect(root, vals, ans);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<pair<int64,int64>> seg;
        seg.reserve(n + m);
        for (int i = 0; i < n + m; ++i) {
            int64 l, r;
            cin >> l >> r;
            seg.push_back({l, r});
        }
        auto ans = solveCase(move(seg));
        cout << ans.size() << '\n';
        for (size_t i = 0; i < ans.size(); ++i) {
            if (i) cout << ' ';
            cout << ans[i];
        }
        cout << '\n';
    }
    return 0;
}

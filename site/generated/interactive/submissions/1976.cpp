#include <bits/stdc++.h>
using namespace std;
using u32 = uint32_t;

static long long budgetLeft = 100000;
static mt19937 rng(0x9E3779B9u);

// ---------------- querying ----------------
static int askPath(const vector<int>& path, const vector<int>& labels) {
    static char buf[1 << 17];
    int len = 0;
    int k = (int)path.size();
    len += sprintf(buf + len, "? %d", k);
    for (int i = 0; i < k; i++) {
        int v = (path[i] < 0) ? 0 : labels[path[i]];
        len += sprintf(buf + len, " %d", v);
    }
    buf[len++] = '\n';
    fwrite(buf, 1, len, stdout);
    fflush(stdout);
    int ans;
    if (scanf("%d", &ans) != 1) exit(0);
    budgetLeft -= k;
    return ans;
}

// ---------------- helpers ----------------
static inline int countPathMask(u32 mask, const vector<int>& path) {
    int prev = (path[0] < 0) ? 0 : ((mask >> path[0]) & 1);
    int t = 0;
    for (size_t i = 1; i < path.size(); i++) {
        int c = (path[i] < 0) ? 0 : ((mask >> path[i]) & 1);
        if (c != prev) t++;
        prev = c;
    }
    return t;
}

static inline bool isVariable(const vector<u32>& C, int p) {
    u32 f = (C[0] >> p) & 1;
    for (u32 x : C) if (((x >> p) & 1) != f) return true;
    return false;
}

// Class of all colorings of m unknowns with exactly t transitions along the
// path [0, v_0, ..., v_{m-1}].
static vector<u32> buildClassComb(int m, int t) {
    vector<u32> res;
    if (t < 0 || t > m) return res;
    if (t == 0) { res.push_back(0); return res; }
    vector<int> comb(t);
    for (int i = 0; i < t; i++) comb[i] = i + 1;
    while (true) {
        u32 mask = 0;
        int ci = 0, par = 0;
        for (int e = 1; e <= m; e++) {
            if (ci < t && comb[ci] == e) { par ^= 1; ci++; }
            if (par) mask |= (1u << (e - 1));
        }
        res.push_back(mask);
        int i = t - 1;
        while (i >= 0 && comb[i] == m - t + i + 1) i--;
        if (i < 0) break;
        comb[i]++;
        for (int j = i + 1; j < t; j++) comb[j] = comb[j - 1] + 1;
    }
    return res;
}

// ---------------- query selection ----------------
static vector<int> chooseQuery(const vector<u32>& C, int m) {
    vector<int> V;
    for (int p = 0; p < m; p++) if (isVariable(C, p)) V.push_back(p);
    int vn = (int)V.size();
    size_t bestMax = SIZE_MAX;
    int bestCost = INT_MAX;
    vector<int> bestPath;
    vector<int> cnt(m + 2), path, vv = V;
    auto eval = [&](const vector<int>& pp) {
        fill(cnt.begin(), cnt.end(), 0);
        size_t mx = 0;
        for (u32 mask : C) {
            int a = countPathMask(mask, pp);
            if ((size_t)++cnt[a] > mx) mx = cnt[a];
        }
        int cst = (int)pp.size();
        if (mx < bestMax || (mx == bestMax && cst < bestCost)) {
            bestMax = mx; bestCost = cst; bestPath = pp;
        }
    };
    // For a small class, try to finish it with one query.
    if (C.size() >= 2 && C.size() <= 14 && vn >= 1) {
        int c = (int)C.size();
        for (int s = c; s <= min(vn + 1, c + 3); s++) {
            if (s - 1 > vn) break;
            for (int it = 0; it < 250; it++) {
                shuffle(vv.begin(), vv.end(), rng);
                int sub = s - 1;
                int pos = (it % 2 == 0) ? 0 : (int)(rng() % (unsigned)(sub + 1));
                path.clear();
                for (int i = 0; i <= sub; i++) {
                    if (i == pos) path.push_back(-1);
                    if (i < sub) path.push_back(vv[i]);
                }
                fill(cnt.begin(), cnt.end(), 0);
                bool ok = true;
                for (u32 mask : C) {
                    int a = countPathMask(mask, path);
                    if (cnt[a]++) { ok = false; break; }
                }
                if (ok) return path;
            }
        }
    }
    int ncand;
    if (C.size() > 40000) ncand = 22;
    else if (C.size() > 20000) ncand = 32;
    else if (C.size() > 1000) ncand = 80;
    else if (C.size() > 128) ncand = 180;
    else ncand = 300;
    for (int it = 0; it < ncand; it++) {
        int s;
        if (it % 5 == 0) s = vn;
        else s = 1 + (int)(rng() % (unsigned)vn);
        shuffle(vv.begin(), vv.end(), rng);
        int pos = (it % 3 == 0) ? 0 : (int)(rng() % (unsigned)(s + 1));
        path.clear();
        for (int i = 0; i <= s; i++) {
            if (i == pos) path.push_back(-1);
            if (i < s) path.push_back(vv[i]);
        }
        eval(path);
    }
    return bestPath;
}

// ---------------- block resolution ----------------
// capBlock: maximum number of machine elements this block may consume.
static vector<int> resolveBlock(const vector<int>& labels, int m, long long capBlock) {
    vector<int> col(m, 0);
    if (m == 0) return col;

    // Not enough head-room for the fancy strategy -> resolve individually.
    if (capBlock < 3LL * m + 1) {
        for (int p = 0; p < m; p++) {
            vector<int> fp{-1, p};
            col[p] = askPath(fp, labels);
        }
        return col;
    }

    // First query: full block after mushroom 0.
    vector<int> path;
    path.push_back(-1);
    for (int i = 0; i < m; i++) path.push_back(i);
    int t = askPath(path, labels);
    long long spent = (long long)m + 1;
    if (t == 0) return col;                 // every unknown is A
    if (t == m) {                           // alternating, first is B
        for (int i = 0; i < m; i++) col[i] = (i % 2 == 0) ? 1 : 0;
        return col;
    }
    vector<u32> C = buildClassComb(m, t);
    if (C.size() == 1) {
        u32 mask = C[0];
        for (int p = 0; p < m; p++) col[p] = (mask >> p) & 1;
        return col;
    }
    // Decisions depend only on (m, answer history); share them across blocks.
    static unordered_map<string, vector<int>> cache;
    string history;
    history.push_back((char)m);
    history.push_back((char)(t + 1));

    while (C.size() > 1) {
        int vcnt = 0;
        for (int p = 0; p < m; p++) if (isVariable(C, p)) vcnt++;
        vector<int> qpath;
        auto it = cache.find(history);
        if (it != cache.end()) qpath = it->second;
        else {
            qpath = chooseQuery(C, m);
            cache[history] = qpath;
        }
        long long cst = (long long)qpath.size();
        if (spent + cst + 2LL * vcnt > capBlock) break;
        int ans = askPath(qpath, labels);
        spent += cst;
        history.push_back((char)(ans + 1));
        vector<u32> NC;
        NC.reserve(C.size());
        for (u32 x : C) if (countPathMask(x, qpath) == ans) NC.push_back(x);
        if (NC.empty()) { C.clear(); break; }   // defensive; cannot happen
        C.swap(NC);
    }
    if (C.empty()) {
        for (int p = 0; p < m; p++) {
            vector<int> fp{-1, p};
            col[p] = askPath(fp, labels);
        }
        return col;
    }
    if (C.size() == 1) {
        u32 mask = C[0];
        for (int p = 0; p < m; p++) col[p] = (mask >> p) & 1;
        return col;
    }
    // Out of budget for this block: ask each remaining unknown separately.
    for (int p = 0; p < m; p++) {
        if (!isVariable(C, p)) { col[p] = (C[0] >> p) & 1; continue; }
        vector<int> fp{-1, p};
        col[p] = askPath(fp, labels);
    }
    return col;
}

// ---------------- block size choice ----------------
static const int MAXM = 18;

// Conservative worst-case number of machine elements per unknown.
static double estCostPerUnknown(int m) {
    switch (m) {
        case 2: return 1.6;
        case 3: return 2.1;
        case 4: return 2.6;
        case 5: return 3.1;
        case 6: return 3.5;
        case 7: return 3.8;
        case 8: return 4.1;
        case 9: return 4.3;
        case 10: return 4.5;
        case 11: return 4.7;
        case 12: return 4.9;
        case 13: return 5.1;
        case 14: return 5.3;
        case 15: return 5.5;
        case 16: return 5.7;
        case 17: return 5.9;
        case 18: return 6.15;
        default: return 6.5;
    }
}

int main() {
    int N;
    if (scanf("%d", &N) != 1) return 0;
    int unk = N - 1;
    if (unk <= 0) {
        printf("! 1\n");
        fflush(stdout);
        return 0;
    }
    long long Bcount = 0;
    long long processed = 0;
    while (processed < unk) {
        long long rem = unk - processed;
        double ratio = (double)budgetLeft / (double)rem;
        int m = 1;
        for (int mm = 2; mm <= MAXM && mm <= rem; mm++)
            if (estCostPerUnknown(mm) <= ratio * 1.2) m = mm;
        if (m > rem) m = (int)rem;
        if (m < 1) m = 1;

        int sz = m;
        vector<int> blockLabels(sz);
        for (int i = 0; i < sz; i++) blockLabels[i] = (int)(processed + i + 1);
        long long later = rem - sz;
        long long cap = budgetLeft - 2LL * later;
        vector<int> col = resolveBlock(blockLabels, sz, cap);
        for (int c : col) if (c) Bcount++;
        processed += sz;
    }
    long long countA = 1 + (long long)unk - Bcount;
    printf("! %lld\n", countA);
    fflush(stdout);
    return 0;
}

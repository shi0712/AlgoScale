#include <bits/stdc++.h>
using namespace std;

constexpr int M = 501;
using Set = bitset<M>;

int n, label[M], distinctNow;
Set dom[M];
deque<int> belt;

int response() {
    int x;
    if (!(cin >> x) || x < 0) exit(0);
    return x;
}

bool put(int x) {
    cout << "+ " << x << endl;
    int old = distinctNow;
    distinctNow = response();
    belt.push_back(x);
    return distinctNow > old;
}

bool popFront() {
    cout << "-" << endl;
    int old = distinctNow;
    distinctNow = response();
    belt.pop_front();
    return distinctNow < old;
}

int key(int x) { return label[x] ? label[x] : x; }

// Prepare an unordered set of representatives, reusing a suitable suffix.
void prepare(const vector<int>& v) {
    Set wanted, kept;
    for (int x : v) wanted.set(key(x));
    int len = 0;
    for (auto it = belt.rbegin(); it != belt.rend(); ++it) {
        int t = key(*it);
        if (!wanted[t] || kept[t]) break;
        kept.set(t);
        ++len;
    }
    while ((int)belt.size() > len) popFront();
    for (int x : v)
        if (!kept[key(x)]) put(x);
}

vector<int> elements(const Set& s) {
    vector<int> a;
    for (int i = 1; i <= n; ++i)
        if (s[i]) a.push_back(i);
    return a;
}

void fixLabel(int x, int c) {
    dom[x].reset();
    dom[x].set(c);
    label[x] = c;
}

int only(const Set& s) {
    for (int j = 1; j <= n; ++j) if (s[j]) return j;
    return 0;
}

// Each row in a is known to represent a different class.
void inject(vector<int> a) {
    Set all;
    for (;;) {
        if (a.empty()) return;
        all.reset();
        Set multiple;
        for (int x : a) {
            multiple |= all & dom[x];
            all |= dom[x];
        }
        Set requiredUnique;
        if (all.count() == a.size()) requiredUnique = all & ~multiple;
        Set used;
        vector<int> left;
        for (int x : a) {
            Set d = dom[x];
            if ((d & requiredUnique).any()) d &= requiredUnique;
            if (d.count() == 1) {
                fixLabel(x, only(d));
                used |= d;
            } else left.push_back(x);
        }
        if (used.none()) break;
        a.swap(left);
        for (int x : a) dom[x] &= ~used;
    }

    vector<int> cols = elements(all);
    int m = a.size(), k = cols.size();
    if (m * 2 <= k) {
        // Find the image before trying to determine the bijection.
        prepare(a);
        Set image;
        int need = m;
        for (int i = 0; i < k && need; ++i) {
            if (k-i == need || !put(cols[i])) {
                image.set(cols[i]);
                --need;
            }
        }
        for (int x : a) dom[x] &= image;
        inject(a);
        return;
    }

    int cut = max(1, (int)round(k * 0.42));
    Set p, test;
    for (int i = 0; i < cut; ++i) p.set(cols[i]);
    vector<int> l, r, undecided;
    for (int x : a) {
        if ((dom[x] & ~p).none()) l.push_back(x);
        else if ((dom[x] & p).none()) r.push_back(x);
        else {
            undecided.push_back(x);
            test |= dom[x] & p;
        }
    }
    if (!undecided.empty()) prepare(elements(test));
    for (int x : undecided) {
        bool hit;
        if ((int)l.size() == cut) hit = false;
        else if ((int)r.size() == k-cut) hit = true;
        else hit = !put(x);
        if (hit) {
            dom[x] &= p;
            l.push_back(x);
        } else {
            dom[x] &= ~p;
            r.push_back(x);
        }
    }
    inject(l);
    inject(r);
}

// After a fresh scan, identify the last occurrence of each class.
Set getLast(const vector<int>& v) {
    Set last;
    int m = v.size();
    for (int i = 0; i < m; ++i) {
        if (distinctNow == m-i) {
            for (; i < m; ++i) last.set(v[i]);
            break;
        }
        if (distinctNow == 1) {
            last.set(v.back());
            break;
        }
        if (popFront()) last.set(v[i]);
    }
    return last;
}

void classify(vector<int> v);

void block(const vector<int>& v, bool initial) {
    prepare({});
    Set first;
    vector<int> reps;
    for (int x : v) if (put(x)) {
        first.set(x);
        reps.push_back(x);
    }
    if (initial)
        for (int x : reps) fixLabel(x,x);
    if (reps.size() == 1) {
        int x = reps[0];
        if (!initial) {
            for (int y : v) dom[x] &= dom[y];
            inject({x});
        }
        for (int y : v) fixLabel(y,label[x]);
        return;
    }
    if (reps.size() == v.size()) {
        if (!initial) inject(reps);
        return;
    }

    Set last = getLast(v);
    if (!initial) inject(reps);

    // The non-singleton firsts and non-singleton lasts are two
    // rainbow lists in bijection. A last must follow its own first.
    Set active;
    vector<int> ends;
    for (int x : v) {
        if (first[x]) {
            if (!last[x]) active.set(label[x]);
        } else if (last[x]) {
            if (initial) dom[x] = active;
            else dom[x] &= active;
            ends.push_back(x);
        }
    }
    inject(ends);

    active.reset();
    vector<int> inside;
    for (int x : v) {
        if (first[x] && !last[x]) active.set(label[x]);
        else if (last[x]) active.reset(label[x]);
        else {
            if (initial) dom[x] = active;
            else dom[x] &= active;
            inside.push_back(x);
        }
    }
    classify(inside);
}

void classify(vector<int> v) {
    map<pair<int,int>,vector<int>> binary;
    vector<int> rest;
    for (int x : v) {
        int cnt = dom[x].count();
        if (cnt == 1) fixLabel(x,only(dom[x]));
        else if (cnt == 2) {
            auto d = elements(dom[x]);
            binary[{d[0],d[1]}].push_back(x);
        } else rest.push_back(x);
    }

    for (auto& [ab, group] : binary) {
        auto [a,b] = ab;
        int prev = a;
        if (!belt.empty() && label[belt.back()] == b) prev = b;
        prepare({prev});
        for (int i = 0; i < (int)group.size(); ++i) {
            int x = group[i];
            if (put(x)) prev = a+b-prev;
            fixLabel(x,prev);
            if (i+1 < (int)group.size()) popFront();
        }
    }

    // Bound the work of repeated first/last peeling on frequent classes.
    for (int i = 0; i < (int)rest.size();) {
        Set pool;
        vector<int> batch;
        do {
            int x = rest[i++];
            pool |= dom[x];
            batch.push_back(x);
        } while (i < (int)rest.size() && batch.size() < 2*pool.count());
        if (batch.size() == 1) inject(batch);
        else block(batch,false);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 1) return 0;
    vector<int> ids(n);
    iota(ids.begin(),ids.end(),1);
    if (n == 1) fixLabel(1,1);
    else block(ids,true);
    map<int,vector<int>> groups;
    for (int x : ids) groups[label[x]].push_back(x);
    cout << "! " << groups.size() << endl;
    for (auto& [c,g] : groups) {
        cout << g.size();
        for (int x : g) cout << ' ' << x;
        cout << endl;
    }
    return 0;
}

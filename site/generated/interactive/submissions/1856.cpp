#include <bits/stdc++.h>
using namespace std;

[[noreturn]] void stop() {
    exit(0);
}

int n;
vector<int> answer;

int ask(const vector<pair<int, int>>& terms) {
    int k = 0;
    for (auto [u, w] : terms) k += w;
    if (k < 1 || k > n) stop();

    cout << "? 1 " << k;
    for (auto [u, w] : terms)
        for (int j = 0; j < w; ++j) cout << ' ' << u;
    cout << '\n' << flush;

    int result;
    // In particular, -1 can be a legitimate path-sum response.
    if (!(cin >> result) || result < -3 * k || result > 3 * k) stop();
    return result;
}

void toggle(int u) {
    cout << "? 2 " << u << '\n' << flush;
}

void report() {
    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << answer[i];
    cout << '\n' << flush;
}

int toBits(int reply, int k, int baseline) {
    int z = reply - k * (baseline - 1);
    if (z < 0 || z > 2 * k || z % 2) stop();
    return z / 2;
}

struct Bit {
    int rep;
    // If the representative has value v, each listed vertex has value v * sign.
    vector<pair<int, int>> linked;
};

void settle(const Bit& bit, int value) {
    for (auto [u, sign] : bit.linked) answer[u] = value * sign;
}

struct Pack {
    int low;
    int extra;
    int size() const { return low + (extra != 0); }
    int weight(int j) const { return j < low ? (1 << j) : extra; }
};

Pack plan(int count, int budget, bool distanceThree) {
    int g = 0;
    while (g < count && (1 << (g + 1)) - 1 <= budget) ++g;

    int t = 0;
    if (g < count) {
        t = budget - ((1 << g) - 1);
        if (distanceThree && t > 0 && (t & (t - 1)) == 0) --t;
    }
    return {g, t};
}

vector<Bit> takeBits(vector<Bit>& todo, int count) {
    vector<Bit> bits;
    while (count--) {
        bits.push_back(move(todo.back()));
        todo.pop_back();
    }
    return bits;
}

// The low bits use binary weights. The optional extra bit leaves at
// most two explanations, which can be carried as one representative.
void unpack(const Pack& pack, int sum, int orientation,
            const vector<Bit>& bits, vector<Bit>& todo) {
    int limit = 1 << pack.low;
    vector<int> masks;
    if (0 <= sum && sum < limit) masks.push_back(sum);
    if (pack.extra && 0 <= sum - pack.extra && sum - pack.extra < limit)
        masks.push_back(limit + sum - pack.extra);
    if (masks.empty()) stop();

    Bit pending{0, {}};
    int repSign = 0;
    for (int j = 0; j < pack.size(); ++j) {
        int v = ((masks[0] >> j & 1) ? 1 : -1) * orientation;
        if (masks.size() == 1 || ((masks[0] ^ masks[1]) >> j & 1) == 0) {
            settle(bits[j], v);
        } else {
            if (!pending.rep) {
                pending.rep = bits[j].rep;
                repSign = v;
            }
            for (auto [u, sign] : bits[j].linked)
                pending.linked.emplace_back(u, sign * v * repSign);
        }
    }
    if (pending.rep) todo.push_back(move(pending));
}

void ordinaryPack(vector<Bit>& todo, int baseline, const Pack& pack) {
    auto bits = takeBits(todo, pack.size());
    vector<pair<int, int>> terms;
    int k = 0;
    for (int j = 0; j < pack.size(); ++j) {
        int w = pack.weight(j);
        terms.emplace_back(bits[j].rep, w);
        k += w;
    }
    int sum = toBits(ask(terms), k, baseline);
    unpack(pack, sum, 1, bits, todo);
}

int centerFrom(int reply, int k, int leafWeight) {
    int result = 9;
    for (int c = -2; c <= 2; ++c) {
        int z = reply - k * c + leafWeight;
        if (z < 0 || z > 2 * leafWeight || z % 2) continue;
        if (result != 9) stop();
        result = c;
    }
    if (result == 9) stop();
    return result;
}

void finishZero() {
    bool uniform = true;
    for (int u = 3; u <= n; ++u)
        if (answer[u] != answer[2]) uniform = false;
    if (uniform) {
        answer[1] = -answer[2];
        return;
    }

    toggle(1);
    int reply = ask({{1, 1}});
    if (abs(reply) != 2) stop();
    answer[1] = reply / 2;
}

void finishWithWitnesses(const Bit& pending) {
    vector<pair<int, int>> terms;
    for (int j = 0; j < 3; ++j)
        terms.emplace_back(pending.linked[j].first, 1 << j);

    toggle(1);
    int reply = ask(terms);
    if (!reply) stop();
    int s = reply > 0 ? 1 : -1;
    int mask = toBits(reply, 7, 2 * s);
    int selected = 0;
    for (int v : {-1, 1}) {
        int mismatches = 0;
        for (int j = 0; j < 3; ++j) {
            int observed = (mask >> j & 1) ? 1 : -1;
            mismatches += observed != v * pending.linked[j].second;
        }
        if (mismatches <= 1) {
            if (selected) stop();
            selected = v;
        }
    }
    if (!selected) stop();
    settle(pending, selected);
    answer[1] = s;
}

int tailSize(int count) {
    for (int h = 1; h <= count && (1 << h) - 1 <= n; ++h) {
        int a = count - h;
        if (a > 10) continue;
        if (h * (h + 1) / 2 + (h + 1) * ((1 << a) - 1) <= n)
            return h;
    }
    return 0;
}

// In the second weighing the center has been flipped. The checksum
// from the first weighing identifies the only possibly changed offset.
void finishTail(vector<Bit>& todo, int h) {
    auto bits = takeBits(todo, (int)todo.size());
    int a = (int)bits.size() - h;
    int mod = h + 1;
    vector<pair<int, int>> terms;
    int k = 0;
    for (int j = 0; j < h + a; ++j) {
        int w = j < h ? j + 1 : mod * (1 << (j - h));
        terms.emplace_back(bits[j].rep, w);
        k += w;
    }
    int initial = toBits(ask(terms), k, 0);

    auto checksum = [h](int mask) {
        int res = 0;
        for (int j = 0; j < h; ++j)
            if (mask >> j & 1) res += j + 1;
        return res;
    };
    auto apply = [&](int low, int high) {
        for (int j = 0; j < h; ++j)
            settle(bits[j], (low >> j & 1) ? 1 : -1);
        for (int j = 0; j < a; ++j)
            settle(bits[h + j], (high >> j & 1) ? 1 : -1);
    };

    int unique = -1;
    int itsHigh = 0;
    for (int mask = 0; mask < (1 << h); ++mask) {
        int diff = initial - checksum(mask);
        if (diff < 0 || diff % mod || diff / mod >= (1 << a)) continue;
        if (unique >= 0) {
            unique = -2;
            break;
        }
        unique = mask;
        itsHigh = diff / mod;
    }
    if (unique == -1) stop();
    if (unique >= 0) {
        apply(unique, itsHigh);
        finishZero();
        return;
    }

    toggle(1);
    terms.clear();
    for (int j = 0; j < h; ++j)
        terms.emplace_back(bits[j].rep, 1 << j);
    int reply = ask(terms);
    if (!reply) stop();
    int s = reply > 0 ? 1 : -1;
    int k2 = (1 << h) - 1;
    int mask = toBits(reply, k2, 2 * s);
    int correction = (s * (initial - checksum(mask))) % mod;
    if (correction < 0) correction += mod;
    if (correction) {
        if (((mask >> (correction - 1) & 1) != 0) != (s < 0)) stop();
        mask ^= 1 << (correction - 1);
    }
    int diff = initial - checksum(mask);
    if (diff < 0 || diff % mod || diff / mod >= (1 << a)) stop();
    apply(mask, diff / mod);
    answer[1] = s;
}

void solveZero(vector<Bit>& todo) {
    while (!todo.empty()) {
        if (todo.size() == 1 && todo.back().linked.size() >= 3 && n >= 7) {
            finishWithWitnesses(todo.back());
            return;
        }

        // Avoid a two-vertex swap ambiguity: it need not survive the
        // center flip in a distinguishable form.
        Pack pack = plan((int)todo.size(), n, n >= 7);
        if (n < 7) {
            pack.extra = 0;
        }
        if ((int)todo.size() > pack.size()) {
            int h = tailSize((int)todo.size());
            if (h) {
                finishTail(todo, h);
                return;
            }
        }
        ordinaryPack(todo, 0, pack);
    }
    finishZero();
}

void solveSmall() {
    if (n == 2) {
        int r = ask({{1, 1}, {2, 1}});
        if (abs(r) == 3) {
            answer[1] = answer[2] = r / 3;
        } else {
            if (abs(r) != 1) stop();
            int f = ask({{1, 1}});
            if (f != 0 && f != r) stop();
            answer[1] = f == 0 ? -r : r;
            answer[2] = -answer[1];
        }
        return;
    }

    int r = ask({{1, 1}, {2, 1}, {3, 1}});
    int c = centerFrom(r, 3, 2);
    int sum = r - 3 * c;

    if (abs(c) == 1) {
        answer[1] = c;
        answer[2] = sum ? sum / 2 : ask({{2, 1}}) - c;
        answer[3] = sum - answer[2];
    } else if (c == 0) {
        if (sum) {
            answer[2] = answer[3] = sum / 2;
            answer[1] = -answer[2];
        } else {
            toggle(2);
            int f = ask({{3, 1}});
            if (abs(f) != 1 && abs(f) != 3) stop();
            answer[2] = answer[3] = f > 0 ? 1 : -1;
            answer[1] = abs(f) == 3 ? answer[2] : -answer[2];
        }
    } else {
        int s = c / 2;
        if (sum == 0) {
            answer[1] = answer[2] = answer[3] = s;
        } else {
            if (sum != -2 * s) stop();
            toggle(1);
            answer[1] = -s;
            answer[2] = ask({{2, 1}});
            answer[3] = -answer[2];
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    for (int i = 1; i < n; ++i) {
        int u, v;
        if (!(cin >> u >> v)) return 0;
    }
    answer.assign(n + 1, 0);

    if (n <= 3) {
        solveSmall();
        report();
        return 0;
    }

    int g = 0;
    while ((1 << (g + 1)) + 1 <= n) ++g;
    int w = (1 << g) - 1;
    vector<pair<int, int>> first{{1, 2}};
    for (int j = 0; j < g; ++j) first.emplace_back(j + 2, 1 << j);

    int reply = ask(first);
    int c = centerFrom(reply, w + 2, w);
    int mask = toBits(reply - 2 * c, w, c);
    for (int j = 0; j < g; ++j)
        answer[j + 2] = (mask >> j & 1) ? 1 : -1;

    vector<Bit> todo;
    for (int u = n; u >= g + 2; --u)
        todo.push_back({u, {{u, 1}}});

    if (c == 0) {
        solveZero(todo);
    } else if (abs(c) == 1) {
        answer[1] = c;
        while (!todo.empty())
            ordinaryPack(todo, c, plan((int)todo.size(), n, false));
    } else {
        int s = c / 2;
        vector<int> possible;
        for (int u = 2; u <= g + 1; ++u)
            if (answer[u] == -s) possible.push_back(u);

        toggle(1);
        answer[1] = -s;

        int p = (int)possible.size();
        if (p) {
            // Each old candidate has normalized bit 0, except the root
            // if it lies here. Its weight is recovered as a residue.
            int mod = p + 1;
            int k = p * (p + 1) / 2;
            Pack pack = plan((int)todo.size(), (n - k) / mod, false);
            auto bits = takeBits(todo, pack.size());
            vector<pair<int, int>> terms;
            for (int j = 1; j <= p; ++j) terms.emplace_back(possible[j - 1], j);
            for (int j = 0; j < pack.size(); ++j) {
                int weight = mod * pack.weight(j);
                terms.emplace_back(bits[j].rep, weight);
                k += weight;
            }
            int sum = toBits(s * ask(terms), k, 0);
            int rootIndex = sum % mod;
            if (rootIndex) answer[possible[rootIndex - 1]] = s;
            unpack(pack, sum / mod, s, bits, todo);
        }

        while (!todo.empty())
            ordinaryPack(todo, 0, plan((int)todo.size(), n, false));
    }

    report();
    return 0;
}

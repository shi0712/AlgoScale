#include <bits/stdc++.h>
using namespace std;

int ask(const vector<int>& indices) {
    cout << "? " << indices.size();
    for (int i : indices) cout << ' ' << i + 1;
    cout << '\n' << flush;
    int answer;
    if (!(cin >> answer) || (answer != 0 && answer != 1)) exit(0);
    return answer;
}

struct Patterns {
    vector<vector<unsigned char>> rows;
    vector<array<int, 3>> count;
    vector<int> alive;

    void start(int width) {
        count.assign(width, {0, 0, 0});
        alive.resize(rows.size());
        iota(alive.begin(), alive.end(), 0);
        for (const auto& row : rows)
            for (int j = 0; j < width; ++j)
                ++count[j][row[j]];
    }

    void keep(int column, int allowed) {
        int size = 0;
        for (int id : alive) {
            const auto& row = rows[id];
            if (allowed & (1 << row[column])) {
                alive[size++] = id;
            } else {
                for (int j = 0; j < (int)row.size(); ++j)
                    --count[j][row[j]];
            }
        }
        alive.resize(size);
    }

    pair<int, int> split(int alphabet) const {
        int best = 0, column = -1, symbol = -1;
        for (int j = 0; j < (int)count.size(); ++j) {
            for (int c = 0; c < alphabet; ++c) {
                int score = min(count[j][c], (int)alive.size() - count[j][c]);
                if (score > best) {
                    best = score;
                    column = j;
                    symbol = c;
                }
            }
        }
        return {column, symbol};
    }
};

vector<vector<int>> groups(const vector<int>& residue, int modulus) {
    vector<vector<int>> g(modulus);
    for (int i = 0; i < (int)residue.size(); ++i)
        g[residue[i]].push_back(i);
    return g;
}

int frequency(int n, int r, int mod) {
    int first = r ? r : mod;
    return n < first ? 0 : 1 + (n - first) / mod;
}

// The labels modulo d are affine images of the true residues. When d
// does not divide n, they are actual residues in one of the two orientations.
void lift(vector<int>& residue, int d) {
    const int n = residue.size();
    auto g = groups(residue, d);
    vector<int> base(2 * d);
    for (int r = 0; r < d; ++r) {
        base[2 * r] = g[r][0];
        base[2 * r + 1] = g[r][1];
    }
    const int baseReply = ask(base);
    const int baseParity = (1 ^ baseReply) ^ ((d - 1) & 1);

    auto swapOne = [&](int slot, int index) {
        int old = base[slot];
        base[slot] = index;
        int bit = ask(base) ^ baseReply;
        base[slot] = old;
        return bit;
    };

    // rel[i] is the high bit of i XOR the high bit of its group's first index.
    vector<int> rel(n, -1), pending;
    int knownParity = 0;
    for (int r = 0; r < d; ++r) {
        const auto& v = g[r];
        int cap = ((int)v.size() + 1) / 2;
        int cnt[2] = {1, 0};
        rel[v[0]] = 0;
        for (int j = 2; j < (int)v.size(); ++j) {
            int b;
            if (cnt[0] == cap) b = 1;
            else if (cnt[1] == cap) b = 0;
            else b = swapOne(2 * r, v[j]);
            rel[v[j]] = b;
            ++cnt[b];
        }
        if (cnt[0] == cap) rel[v[1]] = 1;
        else if (cnt[1] == cap) rel[v[1]] = 0;
        if (rel[v[1]] >= 0) knownParity ^= rel[v[1]];
        else pending.push_back(r);
    }
    for (int j = 0; j < (int)pending.size(); ++j) {
        int r = pending[j];
        int b = (j + 1 == (int)pending.size())
                    ? (knownParity ^ baseParity)
                    : swapOne(2 * r + 1, g[r][2]) ^ rel[g[r][2]];
        rel[g[r][1]] = b;
        knownParity ^= b;
    }

    // Replace two copies of r+1 by one copy each of r and r+2.
    // The low-bit sum stays unchanged, exposing an XOR of group offsets.
    auto edge = [&](int r) {
        int slot = 2 * (r + 1);
        base[slot] = g[r][2];
        base[slot + 1] = g[r + 2][2];
        int bit = ask(base) ^ baseReply;
        base[slot] = g[r + 1][0];
        base[slot + 1] = g[r + 1][1];
        return bit ^ rel[g[r][2]] ^ rel[g[r + 2][2]] ^ rel[g[r + 1][1]];
    };

    vector<int> offset(d, -1), majority(d);
    for (int r = 0; r < d; ++r) {
        int ones = 0;
        for (int i : g[r]) ones += rel[i];
        majority[r] = 2 * ones > (int)g[r].size();
    }

    if (n % d != 0) {
        for (int r = 0; r < d; ++r) {
            if (g[r].size() & 1) {
                int high = frequency(n, r + d, 2 * d) > frequency(n, r, 2 * d);
                offset[r] = high ^ majority[r];
            }
        }
        for (int r = 0; r + 2 < d; ++r)
            if (offset[r] >= 0 && offset[r + 2] < 0)
                offset[r + 2] = offset[r] ^ edge(r);
        for (int r = d - 3; r >= 0; --r)
            if (offset[r] < 0)
                offset[r] = offset[r + 2] ^ edge(r);

    } else if (n % (2 * d) == 0) {
        offset[0] = offset[1] = 0;
        for (int r = 0; r + 2 < d; ++r)
            offset[r + 2] = offset[r] ^ edge(r);

    } else {
        // All groups have odd size. Their majority subgroups are the affine
        // image, modulo 2d, of the d extra residues 1,...,d.
        Patterns pool;
        vector<pair<int, int>> maps;
        vector<int> high(d);
        int mask = 2 * d - 1;
        auto makeHigh = [&](int a, int b) {
            for (int t = 1; t <= d; ++t) {
                int z = (a * t + b) & mask;
                high[z & (d - 1)] = z >= d;
            }
        };
        // Changing the high bits of a and b flips offsets only by parity,
        // so it does not affect any of the edge predictions.
        for (int a = 1; a < d; a += 2) {
            for (int b = 0; b < d; ++b) {
                makeHigh(a, b);
                vector<unsigned char> row(d - 2);
                for (int r = 0; r + 2 < d; ++r)
                    row[r] = high[r] ^ high[r + 2];
                pool.rows.push_back(move(row));
                maps.emplace_back(a, b);
            }
        }
        pool.start(d - 2);
        while (true) {
            auto [r, symbol] = pool.split(2);
            if (r < 0) break;
            int bit = edge(r) ^ majority[r] ^ majority[r + 2];
            pool.keep(r, 1 << bit);
        }

        auto [a, b] = maps[pool.alive.front()];
        makeHigh(a, b);
        vector<int> inverse(2 * d);
        for (int t = 0; t < 2 * d; ++t)
            inverse[(a * t + b) & mask] = t;
        for (int i = 0; i < n; ++i) {
            int r = residue[i];
            int bit = rel[i] ^ majority[r] ^ high[r];
            residue[i] = inverse[r + d * bit];
        }
        // An affine map preserving a proper cyclic interval with at least
        // two points both inside and outside is identity or reflection:
        // only translations by +/-1 overlap the interval in all but one point.
        return;
    }

    for (int i = 0; i < n; ++i)
        residue[i] += d * (rel[i] ^ offset[residue[i]]);
}

int testColor(int i, int color, const array<int, 3>& ref) {
    return ask({i, ref[(color + 1) % 3], ref[(color + 2) % 3]});
}

vector<int> finishExact(const vector<vector<int>>& g, int n, int m) {
    vector<vector<int>> values(m);
    for (int x = 1; x <= n; ++x) values[x % m].push_back(x);
    vector<int> answer(n);
    array<int, 3> ref = {-1, -1, -1};

    for (int r = 0; r < m; ++r) {
        if (g[r].size() == 1) {
            int x = values[r][0], i = g[r][0];
            answer[i] = x;
            ref[x % 3] = i;
        }
    }

    if (find(ref.begin(), ref.end(), -1) != ref.end()) {
        int a = -1, b = -1, ca = -1, cb = -1;
        for (int r = 0; r < m; ++r) {
            if (g[r].size() != 2) continue;
            int missing = (values[r][0] + 2 * m) % 3;
            if (a < 0) {
                a = r;
                ca = missing;
            } else if (missing != ca) {
                b = r;
                cb = missing;
                break;
            }
        }

        auto learnPair = [&](int r, int s, int color) {
            int hit = ask({g[r][0], g[s][0], g[s][1]});
            int first = ((values[r][0] % 3 == color) == bool(hit)) ? 0 : 1;
            for (int j = 0; j < 2; ++j) {
                int i = g[r][j], x = values[r][j ^ first];
                answer[i] = x;
                ref[x % 3] = i;
            }
        };
        learnPair(a, b, cb);
        learnPair(b, a, ca);
    }

    for (int r = 0; r < m; ++r) {
        if (answer[g[r][0]] != 0) continue;
        auto v = values[r];
        for (int j = 0; j + 1 < (int)v.size(); ++j) {
            int k = j;
            while (k + 1 < (int)v.size() &&
                   !testColor(g[r][j], v[k] % 3, ref))
                ++k;
            swap(v[j], v[k]);
            answer[g[r][j]] = v[j];
        }
        answer[g[r].back()] = v.back();
    }
    return answer;
}

// Here every residue modulo m has exactly two positions. Their missing
// color modulo 3 is enough to constrain the global affine residue map.
vector<int> finishPairs(const vector<vector<int>>& g, int m) {
    const int a = 0, b = m / 2;
    array<int, 3> ref = {-1, -1, -1};
    int ca = ask({g[a][0], g[b][0], g[b][1]}) ? 1 : 2;
    int cb = ask({g[b][0], g[a][0], g[a][1]}) ? 0 : 2;
    ref[ca] = g[a][0];
    ref[3 - ca] = g[a][1];
    ref[cb] = g[b][0];
    ref[2 - cb] = g[b][1];

    int missMask[3] = {}, colorMask[2][3] = {}, all = 0;
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 3; ++y) {
            if (x == y) continue;
            int bit = 1 << (3 * x + y);
            all |= bit;
            missMask[3 - x - y] |= bit;
            colorMask[0][x] |= bit;
            colorMask[1][y] |= bit;
        }
    }
    vector<int> options(m, all);
    options[a] = 1 << (3 * ca + (3 - ca));
    options[b] = 1 << (3 * cb + (2 - cb));

    struct Map {
        int mul, add;
        array<int, 3> decode;
    };
    vector<Map> maps;
    Patterns pool;
    auto representative = [&](int mul, int add, int r) {
        int t = (mul * r + add) & (m - 1);
        return t ? t : m;
    };
    for (int mul = 1; mul < m; mul += 2) {
        for (int add = 0; add < m; ++add) {
            int c0 = (representative(mul, add, a) + 2 * m) % 3;
            int c1 = (representative(mul, add, b) + 2 * m) % 3;
            int c2 = 3 - c0 - c1;
            array<int, 3> encode;
            encode[c0] = 0;
            encode[c1] = 1;
            encode[c2] = 2;
            vector<unsigned char> row(m);
            for (int r = 0; r < m; ++r)
                row[r] = encode[(representative(mul, add, r) + 2 * m) % 3];
            pool.rows.push_back(move(row));
            maps.push_back({mul, add, {c0, c1, c2}});
        }
    }

    pool.start(m);
    while (true) {
        auto [r, c] = pool.split(3);
        if (r < 0) break;
        int supported = 0;
        for (int z = 0; z < 3; ++z)
            if (pool.count[r][z]) supported |= missMask[z];
        int opt = options[r] & supported;
        while ((opt & missMask[c]) && (opt & ~missMask[c])) {
            int p0 = popcount((unsigned)(opt & colorMask[0][c]));
            int p1 = popcount((unsigned)(opt & colorMask[1][c]));
            int j = p1 > p0;
            int hit = testColor(g[r][j], c, ref);
            opt &= hit ? colorMask[j][c] : ~colorMask[j][c];
        }
        options[r] = opt;
        int allowed = 0;
        for (int z = 0; z < 3; ++z)
            if (opt & missMask[z]) allowed |= 1 << z;
        pool.keep(r, allowed);
    }

    int id = pool.alive.front();
    auto [mul, add, decode] = maps[id];
    vector<int> answer(2 * m);
    for (int r = 0; r < m; ++r) {
        int opt = options[r] & missMask[pool.rows[id][r]];
        if (opt & (opt - 1)) {
            int code = countr_zero((unsigned)opt);
            int c = code / 3;
            int hit = testColor(g[r][0], c, ref);
            opt &= hit ? colorMask[0][c] : ~colorMask[0][c];
        }
        int code = countr_zero((unsigned)opt);
        int t = representative(mul, add, r);
        if (t % 3 != decode[code / 3]) t += m;
        answer[g[r][0]] = t;
        answer[g[r][1]] = 2 * representative(mul, add, r) + m - t;
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;
    if (n == 2) {
        cout << "! 1 2\n" << flush;
        return 0;
    }

    vector<int> residue(n);
    int cnt[2] = {1, 0};
    for (int i = 1; i < n; ++i) {
        int bit;
        if (cnt[0] == n / 2) bit = 1;
        else if (cnt[1] == n / 2) bit = 0;
        else bit = 1 ^ ask({0, i});
        residue[i] = bit;
        ++cnt[bit];
    }

    int m = 2;
    while (3 * m <= n) {
        lift(residue, m);
        m *= 2;
    }

    auto g = groups(residue, m);
    vector<int> answer = n == 2 * m ? finishPairs(g, m) : finishExact(g, n, m);
    if (answer[0] > n / 2)
        for (int& x : answer) x = n + 1 - x;
    cout << '!';
    for (int x : answer) cout << ' ' << x;
    cout << '\n' << flush;
}

#include <bits/stdc++.h>
using namespace std;

using U32 = uint32_t;

struct Record {
    U32 value;
    int index;
};

// 数值不超过 30 位：四趟基数排序，避免哈希和比较排序。
static void radixSort(vector<Record>& a) {
    vector<Record> tmp(a.size());
    for (int shift = 0; shift < 32; shift += 8) {
        int cnt[256] = {};
        for (const auto& x : a) ++cnt[(x.value >> shift) & 255U];
        int sum = 0;
        for (int i = 0; i < 256; ++i) {
            int c = cnt[i];
            cnt[i] = sum;
            sum += c;
        }
        for (const auto& x : a)
            tmp[cnt[(x.value >> shift) & 255U]++] = x;
        a.swap(tmp);
    }
}

struct Best {
    int start = -1;
    int from = -1;
};

// 寻找环形分段：第 i 段为 A[cut[i] .. cut[i+1])。
static vector<int> findPartition(const vector<U32>& a, const vector<U32>& b) {
    int n = (int)a.size(), m = (int)b.size(), N = 2 * n;
    vector<U32> p(N);
    for (int i = 1; i < N; ++i) p[i] = p[i - 1] ^ a[(i - 1) % n];

    // 离线求出所有 p[v] ^ b[j] 对应的前缀异或值编号。
    vector<Record> records;
    records.reserve((m + 1) * N);
    for (int v = 0; v < N; ++v) records.push_back({p[v], v});
    for (int j = 0; j < m; ++j)
        for (int v = 0; v < N; ++v)
            records.push_back({p[v] ^ b[j], N + j * N + v});
    radixSort(records);

    vector<int> id(N), lookup(m * N, -1);
    int kinds = 0;
    for (int l = 0, r; l < (int)records.size(); l = r) {
        r = l + 1;
        while (r < (int)records.size() && records[r].value == records[l].value) ++r;
        bool exists = false;
        for (int k = l; k < r; ++k)
            if (records[k].index < N) exists = true;
        if (!exists) continue;
        for (int k = l; k < r; ++k) {
            int x = records[k].index;
            if (x < N) id[x] = kinds;
            else lookup[x - N] = kinds;
        }
        ++kinds;
    }
    vector<Record>().swap(records);

    vector<int> color(N);
    for (int v = 0; v < N; ++v)
        color[v] = (v <= n ? v : 2 * n - v) % 3;

    vector<array<int, 3>> previous(N), current(N);
    vector<int> parent(m * N * 3);
    vector<Best> best0(kinds), best2(kinds);

    for (int d = 1; d <= 2; ++d) {
        int r = (2 * n + m * d) % 3;
        fill(previous.begin(), previous.end(), array<int, 3>{-1, -1, -1});
        fill(parent.begin(), parent.end(), -1);
        for (int v = 0; v < n; ++v)
            if (v % 3 == r) previous[v][0] = v;

        for (int j = 0; j < m; ++j) {
            fill(current.begin(), current.end(), array<int, 3>{-1, -1, -1});
            fill(best0.begin(), best0.end(), Best{});
            fill(best2.begin(), best2.end(), Best{});
            int wantedColor = (r + (j + 1) * d) % 3;

            for (int v = 0; v < N; ++v) {
                int u = v - 2;
                if (u >= 0) {
                    int key = id[u];
                    if (previous[u][0] > best0[key].start)
                        best0[key] = {previous[u][0], u * 3};
                    for (int state = 1; state <= 2; ++state)
                        if (previous[u][state] > best2[key].start)
                            best2[key] = {previous[u][state], u * 3 + state};
                }
                if (color[v] != wantedColor) continue;
                int base = (j * N + v) * 3;
                int key = lookup[j * N + v];
                if (key >= 0) {
                    if (best0[key].start >= 0) {
                        current[v][0] = best0[key].start;
                        parent[base] = best0[key].from;
                    }
                    if (best2[key].start >= 0) {
                        current[v][2] = best2[key].start;
                        parent[base + 2] = best2[key].from;
                    }
                }
                // 长度为 1：只能进入或继续唯一的单元素段。
                if (v > 0 && (p[v] ^ p[v - 1]) == b[j]) {
                    int state = previous[v - 1][0] >= previous[v - 1][1] ? 0 : 1;
                    if (previous[v - 1][state] >= 0) {
                        current[v][1] = previous[v - 1][state];
                        parent[base + 1] = (v - 1) * 3 + state;
                    }
                }
            }
            previous.swap(current);
        }

        for (int end = n; end < N; ++end) {
            int start = end - n;
            for (int state = 0; state < 3; ++state) {
                if (previous[end][state] < start) continue;
                vector<int> cut(m + 1);
                cut[m] = end;
                int v = end, st = state;
                for (int j = m - 1; j >= 0; --j) {
                    int from = parent[(j * N + v) * 3 + st];
                    assert(from >= 0);
                    v = from / 3;
                    st = from % 3;
                    cut[j] = v;
                }
                // 将第一段向左延长，得到恰好覆盖 n 个原元素的分段。
                cut[0] = start;
                return cut;
            }
        }
    }
    return {};
}

static vector<int> construct(int n, const vector<int>& cut) {
    int m = (int)cut.size() - 1;
    vector<int> left(n), right(n), answer;
    answer.reserve(n - m);
    for (int i = 0; i < n; ++i) {
        left[i] = (i + n - 1) % n;
        right[i] = (i + 1) % n;
    }
    int cursor = 0;
    auto apply = [&](int type) {
        int erased = type == 1 ? right[cursor] : left[cursor];
        int destination = type == 1 ? right[erased] : left[erased];
        right[left[erased]] = right[erased];
        left[right[erased]] = left[erased];
        cursor = destination;
        answer.push_back(type);
    };
    // 112 / 221：把相应方向的前四段合成一段，分界点不变。
    auto shrink = [&](int direction) {
        apply(direction);
        apply(direction);
        apply(3 - direction);
    };

    vector<int> length(m), vertex(m), where(n, -1);
    for (int i = 0; i < m; ++i) {
        length[i] = cut[i + 1] - cut[i];
        vertex[i] = cut[i] % n;
        where[vertex[i]] = i;
    }

    if (where[0] < 0) {
        int e = 0;
        while (!(cut[e] < n && n < cut[e + 1])) ++e;
        int x = n - cut[e], y = cut[e + 1] - n;
        while (x >= 4) { shrink(2); x -= 3; }
        while (y >= 4) { shrink(1); y -= 3; }
        assert(x != y);
        static const string finish[4][4] = {
            {"", "", "", ""},
            {"", "", "12", "121"},
            {"", "21", "", "2112"},
            {"", "212", "2121", ""}
        };
        const string& s = finish[x][y];
        // 暂不执行最后一步：这一段保留长度 2，分界点落在端点。
        for (int k = 0; k + 1 < (int)s.size(); ++k) apply(s[k] - '0');
        length[e] = 2;
    }
    assert(where[cursor] >= 0);
    int start = where[cursor];

    // 长度 1 不通行；长度 2 mod 3 通行一次；其余合法长度通行两次。
    vector<int> remaining(m);
    int edgeCount = 0;
    for (int i = 0; i < m; ++i) {
        assert(length[i] % 3 != 0);
        remaining[i] = length[i] == 1 ? 0 : (length[i] % 3 == 2 ? 1 : 2);
        edgeCount += remaining[i];
    }

    // 在带重边的环上求欧拉迹。
    vector<pair<int, int>> stack = {{start, -1}};
    vector<int> trail;
    while (!stack.empty()) {
        int v = stack.back().first;
        int e = -1, to = -1;
        if (remaining[v] > 0) {
            e = v;
            to = (v + 1) % m;
        } else {
            int prev = (v + m - 1) % m;
            if (remaining[prev] > 0) { e = prev; to = prev; }
        }
        if (e >= 0) {
            --remaining[e];
            stack.push_back({to, e});
        } else {
            int incoming = stack.back().second;
            stack.pop_back();
            if (incoming >= 0) trail.push_back(incoming);
        }
    }
    assert((int)trail.size() == edgeCount);
    reverse(trail.begin(), trail.end());

    int v = start;
    for (int e : trail) {
        int direction = v == e ? 1 : 2;
        assert(v == e || v == (e + 1) % m);
        int targetLength = length[e] % 3 == 1 ? 4 : 2;
        while (length[e] > targetLength) {
            shrink(direction);
            length[e] -= 3;
        }
        int steps = targetLength / 2;
        while (steps--) { apply(direction); --length[e]; }
        v = direction == 1 ? (e + 1) % m : e;
        assert(cursor == vertex[v]);
    }
    assert(v == 0 && cursor == vertex[0]);
    assert((int)answer.size() == n - m);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string c;
    int t;
    if (!(cin >> c >> t)) return 0;
    while (t--) {
        int n, m;
        cin >> n >> m;
        vector<U32> a(n), b(m);
        U32 xa = 0, xb = 0;
        for (auto& x : a) { cin >> x; xa ^= x; }
        for (auto& x : b) { cin >> x; xb ^= x; }
        if (xa != xb || (n == m && a != b)) {
            cout << "No\n";
            continue;
        }
        vector<int> answer;
        if (n == m) {
            // 无需操作，仍须输出空行。
        } else if (m == 1) {
            answer.assign(n - 1, 1);
        } else {
            vector<int> cut = findPartition(a, b);
            if (cut.empty()) { cout << "No\n"; continue; }
            answer = construct(n, cut);
        }
        cout << "Yes\n";
        for (int i = 0; i < (int)answer.size(); ++i) {
            if (i) cout << ' ';
            cout << answer[i];
        }
        cout << '\n';
    }
    return 0;
}

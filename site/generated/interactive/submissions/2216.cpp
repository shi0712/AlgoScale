#include <bits/stdc++.h>
using namespace std;

int triangleResponse(int a, int b, int c) {
    array<int, 3> s{a, b, c};
    sort(s.begin(), s.end());
    if (s[0] + s[1] <= s[2]) return 0;
    long long x = 1LL * s[0] * s[0];
    long long y = 1LL * s[1] * s[1];
    long long z = 1LL * s[2] * s[2];
    return int(2 * x * y + 2 * y * z + 2 * z * x - x * x - y * y - z * z);
}

int valueAt(int code, int pos) {
    for (int i = 0; i < pos; ++i) code /= 4;
    return code % 4 + 1;
}

struct BaseInfo {
    array<int, 5> known{};
    vector<int> ones, nonOnes;
    int g1 = 0, g2 = 0, value = 0;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> answer(n + 1, 0);

    auto ask = [&](int i, int j, int k) -> int {
        array<int, 3> t{i, j, k};
        sort(t.begin(), t.end());
        cout << "? " << t[0] << ' ' << t[1] << ' ' << t[2] << '\n' << flush;
        int r;
        if (!(cin >> r)) exit(0);
        if (r < 0) exit(0);
        return r;
    };

    auto printAnswer = [&]() {
        cout << "!";
        for (int i = 1; i <= n; ++i) cout << ' ' << answer[i];
        cout << '\n' << flush;
    };

    auto printImpossible = [&]() {
        cout << "! -1\n" << flush;
    };

    auto decodeWithPair = [&](int v, int r) -> int {
        if (v == 2) {
            if (r == 15) return 1;
            if (r == 48) return 2;
            if (r == 63) return 3;
            return 4;
        }
        if (v == 3) {
            if (r == 35) return 1;
            if (r == 128) return 2;
            if (r == 243) return 3;
            return 4;
        }
        if (r == 63) return 1;
        if (r == 240) return 2;
        if (r == 495) return 3;
        return 4;
    };

    auto decodeRepeatedWithOne = [&](int r) -> int {
        if (r == 15) return 2;
        if (r == 35) return 3;
        if (r == 63) return 4;
        return 0;
    };

    auto finishWithPair = [&](int p, int q, int v) {
        answer[p] = answer[q] = v;
        for (int i = 1; i <= n; ++i) {
            if (answer[i] == 0) {
                int r = ask(p, q, i);
                answer[i] = decodeWithPair(v, r);
            }
        }
        printAnswer();
        exit(0);
    };

    if (n == 3) {
        int r = ask(1, 2, 3);
        if (r == 3) answer = {0, 1, 1, 1};
        else if (r == 48) answer = {0, 2, 2, 2};
        else if (r == 243) answer = {0, 3, 3, 3};
        else if (r == 768) answer = {0, 4, 4, 4};
        else {
            printImpossible();
            return 0;
        }
        printAnswer();
        return 0;
    }

    auto runScan = [&](int one, int two, vector<int> active, vector<int> scan) {
        auto addDistinctGroup = [&](const vector<int>& xs) {
            if (xs.empty()) return;
            vector<int> old = active;
            for (int x : xs) {
                for (int s : old) {
                    int r = ask(one, s, x);
                    if (r != 0) {
                        int v = decodeRepeatedWithOne(r);
                        finishWithPair(s, x, v);
                    }
                }
            }
            active.insert(active.end(), xs.begin(), xs.end());
        };

        size_t at = 0;
        while (at + 2 < scan.size()) {
            int i = scan[at], j = scan[at + 1], k = scan[at + 2];
            at += 3;
            int r = ask(i, j, k);

            if (r == 3) {
                answer[i] = answer[j] = answer[k] = 1;
                continue;
            }

            if (r == 135) {
                addDistinctGroup({i, j, k});
                continue;
            }

            if (r != 0) {
                array<pair<int, int>, 3> pairs{{
                    {i, j}, {i, k}, {j, k}
                }};
                for (auto [x, y] : pairs) {
                    int z = ask(one, x, y);
                    if (z == 15 || z == 35 || z == 63) {
                        finishWithPair(x, y, decodeRepeatedWithOne(z));
                    }
                }
                continue;
            }

            vector<int> xs;
            for (int x : {i, j, k}) {
                int z = ask(one, two, x);
                if (z == 3) answer[x] = 1;
                else xs.push_back(x);
            }

            if (xs.size() <= 2) {
                addDistinctGroup(xs);
            } else {
                for (int x : xs) addDistinctGroup({x});
            }
        }

        if (at + 1 < scan.size()) {
            int i = scan[at], j = scan[at + 1];
            at += 2;
            int r = ask(one, i, j);

            if (r == 3) {
                answer[i] = answer[j] = 1;
            } else if (r != 0) {
                finishWithPair(i, j, decodeRepeatedWithOne(r));
            } else {
                vector<int> xs;
                int z = ask(one, two, i);
                if (z == 3) {
                    answer[i] = 1;
                    xs.push_back(j);
                } else {
                    xs.push_back(i);
                    z = ask(one, two, j);
                    if (z == 3) answer[j] = 1;
                    else xs.push_back(j);
                }
                addDistinctGroup(xs);
            }
        }

        if (at < scan.size()) {
            int x = scan[at];
            int z = ask(one, two, x);
            if (z == 3) answer[x] = 1;
            else addDistinctGroup({x});
        }

        if (active.empty()) printAnswer();
        else printImpossible();
        exit(0);
    };

    int initial = ask(1, 2, 3);

    if (initial == 48 || initial == 243 || initial == 768) {
        int v = initial == 48 ? 2 : (initial == 243 ? 3 : 4);
        answer[1] = answer[2] = answer[3] = v;
        finishWithPair(1, 2, v);
    }

    if (initial == 3) {
        answer[1] = answer[2] = answer[3] = 1;
        vector<int> scan;
        for (int i = 4; i <= n; ++i) scan.push_back(i);
        runScan(1, 2, {}, scan);
    }

    map<array<int, 3>, int> obtained;
    auto makeKey = [&](int i, int j, int k) {
        array<int, 3> t{i, j, k};
        sort(t.begin(), t.end());
        return t;
    };
    auto askStore = [&](int i, int j, int k) {
        int r = ask(i, j, k);
        obtained[makeKey(i, j, k)] = r;
    };

    obtained[makeKey(1, 2, 3)] = initial;
    askStore(1, 2, 4);
    askStore(1, 3, 4);
    askStore(2, 3, 4);

    auto getCandidates = [&](int b) {
        vector<int> result;
        int total = 1;
        for (int i = 0; i < b; ++i) total *= 4;

        for (int code = 0; code < total; ++code) {
            array<int, 5> a{};
            for (int i = 0; i < b; ++i) a[i] = valueAt(code, i);
            bool ok = true;
            for (const auto& [key, r] : obtained) {
                if (triangleResponse(a[key[0] - 1], a[key[1] - 1], a[key[2] - 1]) != r) {
                    ok = false;
                    break;
                }
            }
            if (ok) result.push_back(code);
        }
        return result;
    };

    auto analyzeBase = [&](int b, const vector<int>& candidates) {
        BaseInfo info;
        for (int p = 0; p < b; ++p) {
            int v = valueAt(candidates[0], p);
            bool same = true, alwaysNonOne = true;
            for (int code : candidates) {
                int w = valueAt(code, p);
                if (w != v) same = false;
                if (w == 1) alwaysNonOne = false;
            }
            if (same) info.known[p] = v;
            if (same && v == 1) info.ones.push_back(p + 1);
            if (alwaysNonOne) info.nonOnes.push_back(p + 1);
        }

        for (int i = 0; i < b; ++i) {
            for (int j = i + 1; j < b; ++j) {
                if (info.known[i] > 1 && info.known[i] == info.known[j]) {
                    info.g1 = i + 1;
                    info.g2 = j + 1;
                    info.value = info.known[i];
                    return info;
                }
            }
        }
        return info;
    };

    auto useBase = [&](int b, const BaseInfo& info) {
        for (int p = 0; p < b; ++p) {
            if (info.known[p]) answer[p + 1] = info.known[p];
        }
        vector<int> scan;
        for (int i = b + 1; i <= n; ++i) scan.push_back(i);
        runScan(info.ones[0], info.ones[1], info.nonOnes, scan);
    };

    vector<int> candidates = getCandidates(4);

    if (n == 4) {
        if (candidates.size() == 1) {
            for (int p = 0; p < 4; ++p) answer[p + 1] = valueAt(candidates[0], p);
            printAnswer();
        } else {
            printImpossible();
        }
        return 0;
    }

    BaseInfo info = analyzeBase(4, candidates);
    if (info.g1) {
        for (int p = 0; p < 4; ++p) {
            if (info.known[p]) answer[p + 1] = info.known[p];
        }
        finishWithPair(info.g1, info.g2, info.value);
    }
    if (info.ones.size() >= 2) useBase(4, info);

    for (int i = 1; i <= 4; ++i) {
        for (int j = i + 1; j <= 4; ++j) askStore(i, j, 5);
    }

    candidates = getCandidates(5);
    info = analyzeBase(5, candidates);

    if (info.g1) {
        for (int p = 0; p < 5; ++p) {
            if (info.known[p]) answer[p + 1] = info.known[p];
        }
        finishWithPair(info.g1, info.g2, info.value);
    }

    useBase(5, info);
    return 0;
}

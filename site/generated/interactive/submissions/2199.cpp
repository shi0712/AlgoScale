#include <bits/stdc++.h>
using namespace std;

const int INF = 1000000000;

int n, m, L, S, Tsize, W, GW;
vector<int> G, valX, valY, valStep, rowPrefix;
vector<int> xlo, xhi, min1, min2, possibleMax, valAt, M;
vector<int> known, ub, currentMinLB, currentMinT;
vector<unsigned char> state;

priority_queue<int> H;
priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pqMin;

vector<int> prefKnown, prefUB, winKnown, winUB, dqK, dqU, nextKnown;

inline void insertH(int x) {
    if ((int)H.size() < m) {
        H.push(x);
    } else if (x < H.top()) {
        H.pop();
        H.push(x);
    }
}

void applyExactUpdate(int l, int T, int v) {
    int base = l * S;
    if (T > l) {
        int i = T - l, j = T - 1;
        for (int s = i; s <= j; ++s) {
            int newUb = v;
            int kv = known[base + s];
            if (kv != -1 && kv > newUb) newUb = kv;
            if (ub[base + s] > newUb) ub[base + s] = newUb;
        }
        int sMax = valStep[v];
        if (sMax >= i && sMax <= j) {
            if (known[base + sMax] == -1 || known[base + sMax] == v) {
                known[base + sMax] = v;
                ub[base + sMax] = v;
            }
        }
    } else {
        int k = l - T + 1;
        int rowMax = rowPrefix[k];
        for (int s = 1; s <= T - 1; ++s) {
            int newUb = v;
            int kv = known[base + s];
            if (kv != -1 && kv > newUb) newUb = kv;
            if (ub[base + s] > newUb) ub[base + s] = newUb;
        }
        if (v > rowMax) {
            int sMax = valStep[v];
            if (sMax >= 1 && sMax <= T - 1) {
                if (known[base + sMax] == -1 || known[base + sMax] == v) {
                    known[base + sMax] = v;
                    ub[base + sMax] = v;
                }
            }
        }
    }
}

bool inferKnown(int l) {
    int base = l * S;
    int nxt = -1;
    for (int s = L; s >= 1; --s) {
        if (known[base + s] != -1) nxt = s;
        nextKnown[s] = nxt;
    }

    int last = -1;
    bool changed = false;

    for (int s = 1; s <= L; ++s) {
        if (known[base + s] != -1) {
            last = s;
            continue;
        }

        bool set = false;

        if (ub[base + s] < min2[s]) {
            int v = min1[s];
            known[base + s] = v;
            ub[base + s] = v;
            set = true;
        }

        if (!set) {
            int a = last, b = nextKnown[s];
            if (a == -1) a = 1;
            if (b == -1) b = L;

            int va = known[base + a], vb = known[base + b];
            if (va != -1 && vb != -1) {
                int xa = valX[va], xb = valX[vb];

                int lo = xa;
                int t = xb - (b - s);
                if (t > lo) lo = t;

                int hi = xb;
                t = xa + (s - a);
                if (t < hi) hi = t;

                if (lo < xlo[s]) lo = xlo[s];
                if (hi > xhi[s]) hi = xhi[s];

                if (lo == hi) {
                    int y = s + 2 - lo;
                    int v = G[lo * GW + y];
                    known[base + s] = v;
                    ub[base + s] = v;
                    set = true;
                }
            }
        }

        if (set) {
            last = s;
            changed = true;
        }
    }

    return changed;
}

void computeArrays(int l) {
    int base = l * S;

    prefKnown[0] = 0;
    prefUB[0] = 0;

    for (int s = 1; s <= L; ++s) {
        int kv = known[base + s];
        if (kv < 0) kv = 0;

        int pk = prefKnown[s - 1];
        prefKnown[s] = (pk > kv ? pk : kv);

        int pu = prefUB[s - 1];
        int uv = ub[base + s];
        prefUB[s] = (pu > uv ? pu : uv);
    }

    int headK = 0, tailK = 0;
    int headU = 0, tailU = 0;

    for (int s = 1; s <= L; ++s) {
        int kv = known[base + s];
        if (kv < 0) kv = 0;
        int uv = ub[base + s];

        while (headK < tailK) {
            int pos = dqK[tailK - 1];
            int pv = known[base + pos];
            if (pv < 0) pv = 0;
            if (pv <= kv) --tailK;
            else break;
        }
        dqK[tailK++] = s;

        while (headU < tailU) {
            int pos = dqU[tailU - 1];
            if (ub[base + pos] <= uv) --tailU;
            else break;
        }
        dqU[tailU++] = s;

        while (headK < tailK && dqK[headK] <= s - l) ++headK;
        while (headU < tailU && dqU[headU] <= s - l) ++headU;

        if (s >= l) {
            int i = s - l + 1;
            int posK = dqK[headK];
            int kv2 = known[base + posK];
            winKnown[i] = (kv2 < 0 ? 0 : kv2);
            winUB[i] = ub[base + dqU[headU]];
        }
    }
}

void recompute(int l) {
    int minLB = INF, minT = -1;
    bool validMin = false;

    for (int iter = 0; iter < 3; ++iter) {
        bool changed = inferKnown(l);
        computeArrays(l);

        bool exactChanged = false;
        minLB = INF;
        minT = -1;

        for (int T = 1; T <= L + 1; ++T) {
            int idx = l * Tsize + T;
            if (state[idx] != 0) continue;

            int staticLB, knownMax, upper, rowMax = 0;

            if (T <= l) {
                int k = l - T + 1;
                rowMax = rowPrefix[k];
                knownMax = (T >= 2 ? prefKnown[T - 1] : 0);

                staticLB = rowMax;
                if (T >= 2) {
                    int mv = M[S + (T - 1)];
                    if (mv > staticLB) staticLB = mv;
                }

                int maxUBpath = (T >= 2 ? prefUB[T - 1] : 0);
                upper = (rowMax > maxUBpath ? rowMax : maxUBpath);
            } else {
                int i = T - l;
                knownMax = winKnown[i];
                staticLB = M[i * S + (T - 1)];
                upper = winUB[i];
            }

            int exact = -1;
            if (staticLB < INF / 2 && upper <= staticLB) {
                exact = staticLB;
            } else {
                int K = (T <= l ? (rowMax > knownMax ? rowMax : knownMax) : knownMax);
                if (K > 0 && upper <= K) exact = K;
            }

            if (exact != -1) {
                state[idx] = 1;
                insertH(exact);
                applyExactUpdate(l, T, exact);
                exactChanged = true;
            } else {
                int lb = (staticLB > knownMax ? staticLB : knownMax);
                if (lb < minLB) {
                    minLB = lb;
                    minT = T;
                }
            }
        }

        if (minLB == INF) {
            validMin = true;
            break;
        }
        if (!changed && !exactChanged) {
            validMin = true;
            break;
        }
    }

    if (!validMin) {
        computeArrays(l);
        minLB = INF;
        minT = -1;

        for (int T = 1; T <= L + 1; ++T) {
            int idx = l * Tsize + T;
            if (state[idx] != 0) continue;

            int staticLB, knownMax, upper, rowMax = 0;

            if (T <= l) {
                int k = l - T + 1;
                rowMax = rowPrefix[k];
                knownMax = (T >= 2 ? prefKnown[T - 1] : 0);

                staticLB = rowMax;
                if (T >= 2) {
                    int mv = M[S + (T - 1)];
                    if (mv > staticLB) staticLB = mv;
                }

                int maxUBpath = (T >= 2 ? prefUB[T - 1] : 0);
                upper = (rowMax > maxUBpath ? rowMax : maxUBpath);
            } else {
                int i = T - l;
                knownMax = winKnown[i];
                staticLB = M[i * S + (T - 1)];
                upper = winUB[i];
            }

            int exact = -1;
            if (staticLB < INF / 2 && upper <= staticLB) {
                exact = staticLB;
            } else {
                int K = (T <= l ? (rowMax > knownMax ? rowMax : knownMax) : knownMax);
                if (K > 0 && upper <= K) exact = K;
            }

            if (exact != -1) {
                state[idx] = 1;
                insertH(exact);
            } else {
                int lb = (staticLB > knownMax ? staticLB : knownMax);
                if (lb < minLB) {
                    minLB = lb;
                    minT = T;
                }
            }
        }
    }

    currentMinLB[l] = minLB;
    currentMinT[l] = minT;
    if (minLB < INF) pqMin.push({minLB, l});
}

void doQuery(int l, int T) {
    cout << "? " << l << " " << T << '\n';
    cout.flush();

    int v;
    if (!(cin >> v)) exit(0);
    if (v == -1) exit(0);

    applyExactUpdate(l, T, v);
    recompute(l);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n >> m)) return 0;

    GW = n + 1;
    int total = n * n;

    G.assign((n + 1) * (n + 1), 0);
    valX.assign(total + 1, 0);
    valY.assign(total + 1, 0);
    valStep.assign(total + 1, 0);

    for (int x = 1; x <= n; ++x) {
        for (int y = 1; y <= n; ++y) {
            int v;
            cin >> v;
            G[x * GW + y] = v;
            valX[v] = x;
            valY[v] = y;
            valStep[v] = x + y - 2;
        }
    }

    L = 2 * n - 2;
    S = L + 1;
    Tsize = L + 2;
    W = n + 2;

    rowPrefix.assign(n + 1, 0);
    for (int k = 1; k <= n; ++k) {
        int v = G[1 * GW + k];
        rowPrefix[k] = (rowPrefix[k - 1] > v ? rowPrefix[k - 1] : v);
    }

    xlo.assign(L + 1, 0);
    xhi.assign(L + 1, 0);
    min1.assign(L + 1, INF);
    min2.assign(L + 1, INF);
    possibleMax.assign(L + 1, 0);
    valAt.assign(S * W, INF);

    for (int s = 0; s <= L; ++s) {
        int lo, hi;
        if (s == 0) {
            lo = hi = 1;
        } else {
            lo = max(2, s + 2 - n);
            hi = min(n, s + 1);
        }

        xlo[s] = lo;
        xhi[s] = hi;

        int m1 = INF, m2 = INF, mx = 0;
        for (int x = lo; x <= hi; ++x) {
            int y = s + 2 - x;
            int v = G[x * GW + y];
            valAt[s * W + x] = v;

            if (v < m1) {
                m2 = m1;
                m1 = v;
            } else if (v < m2) {
                m2 = v;
            }

            if (v > mx) mx = v;
        }

        min1[s] = m1;
        min2[s] = m2;
        possibleMax[s] = mx;
    }

    M.assign(S * S, INF);
    vector<int> dp(n + 2, INF), ndp(n + 2, INF);

    for (int i = 1; i <= L; ++i) {
        for (int x = xlo[i]; x <= xhi[i]; ++x) {
            dp[x] = valAt[i * W + x];
        }

        int minv = INF;
        for (int x = xlo[i]; x <= xhi[i]; ++x) {
            if (dp[x] < minv) minv = dp[x];
        }
        M[i * S + i] = minv;

        int maxj = min(L, i + n - 1);
        for (int s = i; s < maxj; ++s) {
            int ns = s + 1;

            int left = xlo[ns] - 1;
            if (left >= 1 && left < xlo[s]) dp[left] = INF;
            if (xhi[ns] > xhi[s]) dp[xhi[ns]] = INF;

            int minv2 = INF;
            int baseNs = ns * W;

            for (int x = xlo[ns]; x <= xhi[ns]; ++x) {
                int best = dp[x];
                int b2 = dp[x - 1];
                if (b2 < best) best = b2;

                int val = valAt[baseNs + x];
                int nv = (best > val ? best : val);
                ndp[x] = nv;
                if (nv < minv2) minv2 = nv;
            }

            M[i * S + ns] = minv2;
            dp.swap(ndp);
        }
    }

    known.assign((n + 1) * S, -1);
    ub.assign((n + 1) * S, INF);
    state.assign((n + 1) * Tsize, 0);
    currentMinLB.assign(n + 1, INF);
    currentMinT.assign(n + 1, -1);

    for (int l = 1; l <= n; ++l) {
        int base = l * S;
        for (int s = 0; s <= L; ++s) {
            ub[base + s] = possibleMax[s];
        }

        int v0 = G[1 * GW + 1];
        int v1 = G[2 * GW + 1];
        int vL = G[n * GW + n];

        known[base + 0] = v0;
        ub[base + 0] = v0;

        known[base + 1] = v1;
        ub[base + 1] = v1;

        known[base + L] = vL;
        ub[base + L] = vL;
    }

    prefKnown.assign(L + 2, 0);
    prefUB.assign(L + 2, 0);
    winKnown.assign(L + 2, 0);
    winUB.assign(L + 2, 0);
    dqK.assign(L + 2, 0);
    dqU.assign(L + 2, 0);
    nextKnown.assign(L + 2, -1);

    for (int l = 1; l <= n; ++l) {
        recompute(l);
    }

    while (true) {
        if ((int)H.size() == m) {
            int UB = H.top();

            while (!pqMin.empty()) {
                auto [lb, l] = pqMin.top();
                if (currentMinLB[l] == lb) break;
                pqMin.pop();
            }

            if (pqMin.empty() || pqMin.top().first >= UB) break;
        }

        while (!pqMin.empty()) {
            auto [lb, l] = pqMin.top();
            if (currentMinLB[l] == lb) break;
            pqMin.pop();
        }

        if (pqMin.empty()) break;

        int l = pqMin.top().second;
        int T = currentMinT[l];

        if (T < 0) {
            pqMin.pop();
            continue;
        }

        doQuery(l, T);
    }

    if ((int)H.size() < m) {
        bool done = false;
        for (int l = 1; l <= n && !done; ++l) {
            for (int T = 1; T <= L + 1 && !done; ++T) {
                int idx = l * Tsize + T;
                if (state[idx] == 0) {
                    doQuery(l, T);
                    if ((int)H.size() == m) done = true;
                }
            }
        }
    }

    vector<int> ans;
    ans.reserve(m);
    while (!H.empty()) {
        ans.push_back(H.top());
        H.pop();
    }
    reverse(ans.begin(), ans.end());

    if ((int)ans.size() < m) ans.resize(m, 0);

    cout << "!";
    for (int i = 0; i < m; ++i) {
        if (i) cout << ' ';
        cout << ans[i];
    }
    cout << '\n';
    cout.flush();

    return 0;
}

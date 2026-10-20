#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int n, m;
int tp[105];
ll a[105][20], b[105][20];

static ll cArr[1 << 18], bestArr[1 << 18], bestActArr[1 << 18];
static char flagArr[1 << 18], valArr[1 << 18];
static ll Gtab[1 << 18], Gptab[1 << 18];
static unsigned char failtab[1 << 18];
static int projArr[1 << 18];

int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    for (int i = 0; i < n; i++) scanf("%d", &tp[i]);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) scanf("%lld", &a[i][j]);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) scanf("%lld", &b[i][j]);

    // ranks at which the attacker owns at least one card
    int rankIdx[20];
    vector<int> R;
    for (int j = 0; j < m; j++) {
        ll s = 0;
        for (int i = 0; i < n; i++) s += a[i][j];
        if (s > 0) { rankIdx[j] = (int)R.size(); R.push_back(j); }
        else rankIdx[j] = -1;
    }
    int K = (int)R.size();
    int SZ = 1 << K;

    // subset sums over T of: attacker trump cards, defender trump cards, attacker normal cards
    vector<ll> TA(SZ), ZZ(SZ), NormA(SZ);
    {
        ll baseTA = 0, baseZ = 0, baseNA = 0;
        vector<ll> at(K, 0), bt(K, 0), an(K, 0);
        for (int j = 0; j < m; j++) {
            ll A = 0, B = 0, AN = 0;
            for (int i = 0; i < n; i++) {
                if (tp[i]) { A += a[i][j]; B += b[i][j]; }
                else AN += a[i][j];
            }
            if (rankIdx[j] >= 0) { int id = rankIdx[j]; at[id] = A; bt[id] = B; an[id] = AN; }
            else { baseTA += A; baseZ += B; baseNA += AN; }
        }
        TA[0] = baseTA; ZZ[0] = baseZ; NormA[0] = baseNA;
        for (int T = 1; T < SZ; T++) {
            int lb = T & -T; int i = __builtin_ctz(T);
            TA[T] = TA[T ^ lb] + at[i];
            ZZ[T] = ZZ[T ^ lb] + bt[i];
            NormA[T] = NormA[T ^ lb] + an[i];
        }
    }

    vector<unsigned char> cntFail(SZ, 0);
    vector<ll> sumG(SZ, 0);
    vector<ll> maxGp(SZ, LLONG_MIN / 4);

    int vbit[20];
    int bmap[20];

    for (int s = 0; s < n; s++) {
        bool hasA = false;
        for (int j = 0; j < m; j++) if (a[s][j] > 0) { hasA = true; break; }
        if (!hasA) continue;

        vector<int> rel;
        for (int j = m - 1; j >= 0; j--) if (a[s][j] > 0 || b[s][j] > 0) rel.push_back(j);

        for (int j = 0; j < m; j++) vbit[j] = -1;
        int cnt = 0;
        for (int r : rel) if (rankIdx[r] >= 0) vbit[r] = 1 << (cnt++);

        if (tp[s]) {
            // ---------- condition (1) for a trump suit ----------
            int cur = 1; cArr[0] = 0; valArr[0] = 1;
            for (int r : rel) {
                ll ar = a[s][r], br = b[s][r];
                if (rankIdx[r] < 0) {
                    for (int i = 0; i < cur; i++) {
                        if (valArr[i] && cArr[i] < ar) valArr[i] = 0;
                        cArr[i] += br - ar;
                    }
                } else {
                    for (int i = 0; i < cur; i++) {
                        ll c = cArr[i]; char v = valArr[i];
                        cArr[i + cur] = c + br - ar;
                        valArr[i + cur] = (v && c >= ar) ? 1 : 0;
                    }
                    cur *= 2;
                }
            }
            bool anyValid = false;
            for (int i = 0; i < cur; i++) { failtab[i] = valArr[i] ? 0 : 1; if (valArr[i]) anyValid = true; }
            if (!anyValid) {
                // no S is matchable at all -> every starting card wins
                ll ans = 0;
                for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) ans += a[i][j];
                printf("%lld\n", ans);
                return 0;
            }
            for (int i = 0; i < K; i++) bmap[i] = (vbit[R[i]] > 0) ? vbit[R[i]] : 0;
            projArr[0] = 0;
            if (failtab[0]) cntFail[0]++;
            for (int T = 1; T < SZ; T++) {
                int lb = T & -T; int i = __builtin_ctz(T);
                int p = projArr[T ^ lb] | bmap[i]; projArr[T] = p;
                if (failtab[p]) cntFail[T]++;
            }
        } else {
            // ---------- condition (2) contribution of a normal suit ----------
            int cur = 1;
            cArr[0] = 0; bestArr[0] = LLONG_MIN / 4; bestActArr[0] = LLONG_MIN / 4; flagArr[0] = 0;
            for (int r : rel) {
                ll ar = a[s][r], br = b[s][r];
                if (rankIdx[r] < 0) {
                    for (int i = 0; i < cur; i++) {
                        ll c = cArr[i];
                        ll f = ar - c;
                        if (f > bestArr[i]) bestArr[i] = f;
                        if (flagArr[i] && f > bestActArr[i]) bestActArr[i] = f;
                        cArr[i] = c + br - ar;
                        if (ar > 0) flagArr[i] = 1;
                    }
                } else {
                    for (int i = 0; i < cur; i++) {
                        ll c = cArr[i]; ll bst = bestArr[i], bac = bestActArr[i]; char fl = flagArr[i];
                        ll f0 = -c;
                        ll nb0 = max(bst, f0);
                        ll na0 = fl ? max(bac, f0) : bac;
                        ll f1 = ar - c;
                        bool act1 = fl || (ar > 0);
                        ll nb1 = max(bst, f1);
                        ll na1 = act1 ? max(bac, f1) : bac;
                        cArr[i + cur] = c + br - ar; bestArr[i + cur] = nb1;
                        bestActArr[i + cur] = na1; flagArr[i + cur] = act1 ? 1 : 0;
                        cArr[i] = c; bestArr[i] = nb0; bestActArr[i] = na0; flagArr[i] = fl;
                    }
                    cur *= 2;
                }
            }
            for (int i = 0; i < cur; i++) {
                Gtab[i] = bestArr[i] > 0 ? bestArr[i] : 0;
                Gptab[i] = bestActArr[i];
            }
            for (int i = 0; i < K; i++) bmap[i] = (vbit[R[i]] > 0) ? vbit[R[i]] : 0;
            projArr[0] = 0;
            sumG[0] += Gtab[0];
            if (Gptab[0] > maxGp[0]) maxGp[0] = Gptab[0];
            for (int T = 1; T < SZ; T++) {
                int lb = T & -T; int i = __builtin_ctz(T);
                int p = projArr[T ^ lb] | bmap[i]; projArr[T] = p;
                sumG[T] += Gtab[p];
                if (Gptab[p] > maxGp[T]) maxGp[T] = Gptab[p];
            }
        }
    }

    vector<char> survive(K, 0);
    for (int T = 0; T < SZ; T++) {
        if (cntFail[T]) continue;
        if (NormA[T] > 0) {
            if (sumG[T] > 0) {
                if (TA[T] + sumG[T] > ZZ[T]) continue;
            } else {
                if (TA[T] + maxGp[T] > ZZ[T]) continue;
            }
        }
        for (int i = 0; i < K; i++) if ((T >> i) & 1) survive[i] = 1;
    }

    ll ans = 0;
    for (int i = 0; i < K; i++)
        if (!survive[i]) {
            int j = R[i];
            for (int s2 = 0; s2 < n; s2++) ans += a[s2][j];
        }
    printf("%lld\n", ans);
    return 0;
}

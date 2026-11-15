#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> trump(n);
    for (int i = 0; i < n; ++i) cin >> trump[i];

    vector<vector<long long>> A(n, vector<long long>(m));
    vector<vector<long long>> D(n, vector<long long>(m));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            cin >> A[i][j];

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            cin >> D[i][j];

    int N = 1 << m;

    vector<long long> regularNeed(N, 0);
    vector<long long> trumpSurplus(N, 0);
    vector<unsigned char> invalid(N, 0);

    vector<unsigned char> high(N);
    vector<unsigned int> low(N);

    for (int mask = 1; mask < N; ++mask) {
        int h = 31 - __builtin_clz((unsigned)mask);
        high[mask] = (unsigned char)h;
        low[mask] = (unsigned int)mask ^ (1u << h);
    }

    vector<long long> at(N), dt(N), def(N);

    for (int i = 0; i < n; ++i) {
        at[0] = dt[0] = def[0] = 0;

        if (trump[i]) {
            for (int mask = 1; mask < N; ++mask) {
                int h = high[mask];
                int T = (int)low[mask];

                long long ah = A[i][h];
                long long dh = D[i][h];

                at[mask] = at[T] + ah;
                dt[mask] = dt[T] + dh;

                long long cur = def[T] + ah - dh;
                if (cur < ah) cur = ah;
                def[mask] = cur;

                if (cur != 0) {
                    invalid[mask] = 1;
                } else {
                    trumpSurplus[mask] += dt[mask] - at[mask];
                }
            }
        } else {
            for (int mask = 1; mask < N; ++mask) {
                int h = high[mask];
                int T = (int)low[mask];

                long long ah = A[i][h];
                long long dh = D[i][h];

                at[mask] = at[T] + ah;
                dt[mask] = dt[T] + dh;

                long long cur = def[T] + ah - dh;
                if (cur < ah) cur = ah;
                def[mask] = cur;

                regularNeed[mask] += cur;
            }
        }
    }

    vector<unsigned char> safe(m, 0);

    for (int mask = 0; mask < N; ++mask) {
        if (!invalid[mask] && regularNeed[mask] <= trumpSurplus[mask]) {
            int bits = mask;
            while (bits) {
                int b = __builtin_ctz((unsigned)bits);
                safe[b] = 1;
                bits &= bits - 1;
            }
        }
    }

    long long answer = 0;

    for (int rank = 0; rank < m; ++rank) {
        if (!safe[rank]) {
            for (int suit = 0; suit < n; ++suit) {
                answer += A[suit][rank];
            }
        }
    }

    cout << answer << '\n';
    return 0;
}

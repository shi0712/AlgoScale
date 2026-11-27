#include <bits/stdc++.h>
using namespace std;

class MatrixBuilder {
public:
    int k, m, d;
    mt19937 &rng;
    vector<vector<int>> C;
    vector<int> colSum;
    vector<int> q;

    MatrixBuilder(int k_, int m_, mt19937 &rng_)
        : k(k_), m(m_), d(m_ - 2), rng(rng_), C(k_, vector<int>(m_)),
          colSum(k_), q(k_) {}

    bool checkDFS(int pos, int sum, bool used) {
        if (pos == k) {
            if (!used || sum % d != 0) return true;

            int t = sum / d;
            for (int i = 0; i < m; ++i) {
                int w = 0;
                for (int j = 0; j < k; ++j) {
                    w += q[j] * C[j][i];
                }
                int value = t - w;
                if (value < -2 || value > 2 || value % 2 != 0) {
                    return true;
                }
            }

            // A nonzero relation with coefficients in {-1,0,1} exists.
            return false;
        }

        q[pos] = -1;
        if (!checkDFS(pos + 1, sum - colSum[pos], true)) return false;

        q[pos] = 0;
        if (!checkDFS(pos + 1, sum, used)) return false;

        q[pos] = 1;
        if (!checkDFS(pos + 1, sum + colSum[pos], true)) return false;

        return true;
    }

    bool build() {
        vector<int> pool;
        for (int s = 1; s <= m / 2; ++s) {
            if ((s & 1) == (m & 1)) pool.push_back(s);
        }
        if ((int)pool.size() < k) return false;

        for (int attempt = 0; attempt < 3; ++attempt) {
            shuffle(pool.begin(), pool.end(), rng);

            vector<int> indices(m);
            iota(indices.begin(), indices.end(), 0);

            for (int e = 0; e < k; ++e) {
                shuffle(indices.begin(), indices.end(), rng);
                int positives = (m + pool[e]) / 2;

                fill(C[e].begin(), C[e].end(), -1);
                for (int i = 0; i < positives; ++i) {
                    C[e][indices[i]] = 1;
                }
                colSum[e] = pool[e];
            }

            if (checkDFS(0, 0, false)) return true;
        }

        return false;
    }
};

static bool ask(const string &sheet, int &response) {
    cout << sheet << '\n' << flush;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    return true;
}

static string makeFinalSheet(const vector<int> &y) {
    string sheet(y.size(), 'F');
    for (int i = 0; i < (int)y.size(); ++i) {
        if (y[i] > 0) sheet[i] = 'T';
    }
    return sheet;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    mt19937 rng(712367821);

    if (n == 2) {
        vector<string> queries = {"TT", "TF"};
        vector<int> r(2);

        for (int i = 0; i < 2; ++i) {
            int response;
            ask(queries[i], response);
            if (response == n) return 0;
            r[i] = 2 * response - n;
        }

        vector<int> y(2);
        y[0] = (r[0] + r[1]) / 2;
        y[1] = (r[0] - r[1]) / 2;

        string answer = makeFinalSheet(y);
        int response;
        ask(answer, response);
        return 0;
    }

    int k = 0;
    int m = n;
    vector<vector<int>> C;

    for (int candidateK = min(16, n - 3); candidateK >= 1; --candidateK) {
        int candidateM = n - candidateK;
        if (candidateM < 3 || candidateM == 2) continue;

        MatrixBuilder builder(candidateK, candidateM, rng);
        if (builder.build()) {
            k = candidateK;
            m = candidateM;
            C = move(builder.C);
            break;
        }
    }

    if (k == 0) {
        m = n;
    }

    vector<int> r(m);

    for (int row = 0; row < m; ++row) {
        string sheet(n, 'F');

        for (int col = 0; col < m; ++col) {
            if (col == row) sheet[col] = 'T';
        }
        for (int e = 0; e < k; ++e) {
            if (C[e][row] > 0) sheet[m + e] = 'T';
        }

        int response;
        ask(sheet, response);
        if (response == n) return 0;
        r[row] = 2 * response - n;
    }

    vector<int> y(n);
    long long d = m - 2;
    long long denominator = 2 * d;

    if (k == 0) {
        long long sum = accumulate(r.begin(), r.end(), 0LL);
        for (int i = 0; i < m; ++i) {
            long long numerator = 1LL * r[i] * d - sum;
            if (numerator == denominator) y[i] = 1;
            else if (numerator == -denominator) y[i] = -1;
            else return 0;
        }
    } else {
        vector<int> v(m);
        for (int i = 0; i < m; ++i) {
            v[i] = r[i];
            for (int e = 0; e < k; ++e) {
                v[i] += C[e][i];
            }
        }

        int masks = 1 << k;
        bool found = false;

        for (int mask = 0; mask < masks; ++mask) {
            if (mask > 0) {
                int changed = __builtin_ctz((unsigned)mask);
                int gray = mask ^ (mask >> 1);
                int newSign = ((gray >> changed) & 1) ? 1 : -1;

                for (int i = 0; i < m; ++i) {
                    v[i] -= 2 * newSign * C[changed][i];
                }
            }

            long long sum = accumulate(v.begin(), v.end(), 0LL);
            bool valid = true;

            for (int i = 0; i < m; ++i) {
                long long numerator = 1LL * v[i] * d - sum;
                if (numerator == denominator) {
                    y[i] = 1;
                } else if (numerator == -denominator) {
                    y[i] = -1;
                } else {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                int gray = mask ^ (mask >> 1);
                for (int e = 0; e < k; ++e) {
                    y[m + e] = ((gray >> e) & 1) ? 1 : -1;
                }
                found = true;
                break;
            }
        }

        if (!found) return 0;
    }

    string answer = makeFinalSheet(y);
    int response;
    ask(answer, response);
    return 0;
}

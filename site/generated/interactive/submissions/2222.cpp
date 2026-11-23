#include <bits/stdc++.h>
using namespace std;

int n;
map<string, int> memo;

int cardinality(const string &s) {
    return count(s.begin(), s.end(), '1');
}

string unite(string a, const string &b) {
    for (int i = 0; i < n; ++i) {
        if (b[i] == '1') a[i] = '1';
    }
    return a;
}

[[noreturn]] void finish(int answer) {
    cout << "! " << answer << '\n' << flush;
    exit(0);
}

int getF(const string &s) {
    auto it = memo.find(s);
    if (it != memo.end()) return it->second;

    int sz = cardinality(s);
    if (sz == 0 || sz == n) {
        memo[s] = 0;
        return 0;
    }

    cout << "? " << s << '\n' << flush;
    int response;
    if (!(cin >> response) || response < 0) exit(0);

    memo[s] = response;
    if (response == 0) finish(0);
    return response;
}

bool known(const string &s) {
    int sz = cardinality(s);
    return sz == 0 || sz == n || memo.find(s) != memo.end();
}

// Tests whether there is an edge C-A or a vertex outside C and A
// adjacent to both sets. Either certificate puts A in C's component.
bool related(const string &C, int fC, const string &A) {
    string CA = unite(C, A);
    int fCA = getF(CA);
    int difference = fCA - fC;

    if (difference < 0) return true;

    int maxFA = n - cardinality(A);
    if (difference == maxFA) return false;

    int fA = getF(A);
    return fC + fA - fCA > 0;
}

int estimatedCost(const string &C, const string &A) {
    return (!known(A)) + (!known(unite(C, A)));
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (n <= 1) finish(1);

    string C(n, '0');
    C[0] = '1';
    int fC = getF(C);

    while (true) {
        int outsideCount = n - cardinality(C);

        // C is contained in one component. If every outside vertex is
        // adjacent to C, that component is the whole graph.
        if (outsideCount == 0 || fC == outsideCount) finish(1);

        string U(n, '0');
        for (int i = 0; i < n; ++i) {
            if (C[i] == '0') U[i] = '1';
        }

        // Vertices not eliminated by negative tests contain N(C)\C.
        string possibleBoundary = U;

        // A singleton attempt makes dense and sequentially ordered graphs
        // inexpensive; a negative answer also reduces the search space.
        int chosen = -1;
        int bestCost = INT_MAX;
        for (int i = 0; i < n; ++i) {
            if (U[i] == '0') continue;
            string A(n, '0');
            A[i] = '1';
            int cost = estimatedCost(C, A);
            if (cost < bestCost) {
                bestCost = cost;
                chosen = i;
            }
        }

        string singleton(n, '0');
        singleton[chosen] = '1';
        string withSingleton = unite(C, singleton);

        if (related(C, fC, singleton)) {
            C = withSingleton;
            fC = getF(C);
            continue;
        }

        possibleBoundary[chosen] = '0';
        if (cardinality(possibleBoundary) == fC) {
            C = unite(C, possibleBoundary);
            fC = getF(C);
            continue;
        }

        // Since fC>0, C has an outside neighbor. It is not the singleton
        // just eliminated, so this remaining set has a positive certificate.
        string W = U;
        W[chosen] = '0';
        bool batch = false;

        while (cardinality(W) > 1) {
            vector<int> vertices;
            for (int i = 0; i < n; ++i) {
                if (W[i] == '1') vertices.push_back(i);
            }

            int half = vertices.size() / 2;
            string A(n, '0'), B(n, '0');
            for (int i = 0; i < half; ++i) A[vertices[i]] = '1';
            for (int i = half; i < (int)vertices.size(); ++i) {
                B[vertices[i]] = '1';
            }

            string X = A, Y = B;
            if (estimatedCost(C, B) < estimatedCost(C, A)) {
                X = B;
                Y = A;
            }

            if (related(C, fC, X)) {
                W = X;
            } else {
                // A negative test also proves that X contains no vertex
                // of N(C)\C.
                for (int i = 0; i < n; ++i) {
                    if (X[i] == '1') possibleBoundary[i] = '0';
                }

                if (cardinality(possibleBoundary) == fC) {
                    batch = true;
                    break;
                }
                W = Y;
            }
        }

        if (batch) {
            // The remaining possibilities are exactly N(C)\C, so all of
            // them can be adjoined to C at once.
            C = unite(C, possibleBoundary);
            fC = getF(C);
        } else {
            // W is a singleton with a positive certificate.
            C = unite(C, W);
            fC = getF(C);
        }
    }
}

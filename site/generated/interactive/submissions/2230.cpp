#include <bits/stdc++.h>
using namespace std;

int n, m;
int baselineAnswer;
string preferred;

int ask(int v, const string &b) {
    cout << "? " << v << ' ' << b << '\n' << flush;
    int answer;
    if (!(cin >> answer) || answer < 0) {
        exit(0);
    }
    return answer;
}

int pathIntersectionQuery(int v, int depth, const vector<int> &subset) {
    string b = preferred;
    for (int edge : subset) {
        b[edge] ^= 1;
    }

    int answer = ask(v, b);
    int numerator = answer - m + depth + static_cast<int>(subset.size());
    if (numerator < 0 || (numerator & 1)) {
        exit(0);
    }
    return numerator / 2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) {
        return 0;
    }
    m = n - 1;

    string allZero(m, '0');
    baselineAnswer = ask(1, allZero);

    preferred.assign(m, '0');
    int totalOnes = m - baselineAnswer;

    function<void(int, int, int)> recover = [&](int left, int right, int ones) {
        int length = right - left;
        if (ones == 0) {
            fill(preferred.begin() + left, preferred.begin() + right, '0');
            return;
        }
        if (ones == length) {
            fill(preferred.begin() + left, preferred.begin() + right, '1');
            return;
        }
        if (length <= 1) {
            exit(0);
        }

        int middle = (left + right) / 2;
        string b(m, '0');
        for (int i = left; i < middle; ++i) {
            b[i] = '1';
        }

        int answer = ask(1, b);
        int leftLength = middle - left;
        int numerator = answer - baselineAnswer + leftLength;
        if (numerator < 0 || (numerator & 1)) {
            exit(0);
        }
        int leftOnes = numerator / 2;

        recover(left, middle, leftOnes);
        recover(middle, right, ones - leftOnes);
    };

    recover(0, m, totalOnes);

    vector<int> originalS(m, -1), originalF(m, -1);

    if (n == 2) {
        if (preferred[0] == '0') {
            originalS[0] = 1;
            originalF[0] = 2;
        } else {
            originalS[0] = 2;
            originalF[0] = 1;
        }
        cout << "! " << originalS[0] << ' ' << originalF[0] << '\n' << flush;
        return 0;
    }

    vector<vector<int>> byDepth(n);
    byDepth[0].push_back(1);

    for (int v = 2; v <= n; ++v) {
        int answer = ask(v, preferred);
        int depth = m - answer;
        if (depth < 1 || depth >= n) {
            exit(0);
        }
        byDepth[depth].push_back(v);
    }

    vector<int> parent(n + 1, 0);
    vector<int> edgeToParent(n + 1, -1);
    set<int> unusedEdges;
    for (int i = 0; i < m; ++i) {
        unusedEdges.insert(i);
    }

    for (int depth = 1; depth < n; ++depth) {
        if (byDepth[depth].empty()) {
            continue;
        }

        for (int v : byDepth[depth]) {
            vector<int> parentCandidates = byDepth[depth - 1];
            vector<int> edgeCandidates(unusedEdges.begin(), unusedEdges.end());

            while (parentCandidates.size() > 1 || edgeCandidates.size() > 1) {
                if (parentCandidates.size() > 1 && edgeCandidates.size() > 1) {
                    int parentHalf = static_cast<int>(parentCandidates.size()) / 2;
                    int edgeHalf = static_cast<int>(edgeCandidates.size()) / 2;

                    vector<int> firstParents(
                        parentCandidates.begin(),
                        parentCandidates.begin() + parentHalf
                    );
                    vector<int> firstEdges(
                        edgeCandidates.begin(),
                        edgeCandidates.begin() + edgeHalf
                    );

                    vector<int> subset = firstEdges;
                    for (int u : firstParents) {
                        subset.push_back(edgeToParent[u]);
                    }

                    int result = pathIntersectionQuery(v, depth, subset);
                    bool parentInFirst;
                    bool edgeInFirst;

                    if (result == 0) {
                        parentInFirst = false;
                        edgeInFirst = false;
                    } else if (result == 2) {
                        parentInFirst = true;
                        edgeInFirst = true;
                    } else if (result == 1) {
                        vector<int> parentSubset;
                        for (int u : firstParents) {
                            parentSubset.push_back(edgeToParent[u]);
                        }
                        int parentResult = pathIntersectionQuery(v, depth, parentSubset);
                        parentInFirst = (parentResult == 1);
                        edgeInFirst = !parentInFirst;
                    } else {
                        exit(0);
                    }

                    if (parentInFirst) {
                        parentCandidates = firstParents;
                    } else {
                        parentCandidates = vector<int>(
                            parentCandidates.begin() + parentHalf,
                            parentCandidates.end()
                        );
                    }

                    if (edgeInFirst) {
                        edgeCandidates = firstEdges;
                    } else {
                        edgeCandidates = vector<int>(
                            edgeCandidates.begin() + edgeHalf,
                            edgeCandidates.end()
                        );
                    }
                } else if (parentCandidates.size() > 1) {
                    int half = static_cast<int>(parentCandidates.size()) / 2;
                    vector<int> firstParents(
                        parentCandidates.begin(),
                        parentCandidates.begin() + half
                    );
                    vector<int> subset;
                    for (int u : firstParents) {
                        subset.push_back(edgeToParent[u]);
                    }

                    int result = pathIntersectionQuery(v, depth, subset);
                    if (result == 1) {
                        parentCandidates = firstParents;
                    } else {
                        parentCandidates = vector<int>(
                            parentCandidates.begin() + half,
                            parentCandidates.end()
                        );
                    }
                } else {
                    int half = static_cast<int>(edgeCandidates.size()) / 2;
                    vector<int> firstEdges(
                        edgeCandidates.begin(),
                        edgeCandidates.begin() + half
                    );

                    int result = pathIntersectionQuery(v, depth, firstEdges);
                    if (result == 1) {
                        edgeCandidates = firstEdges;
                    } else {
                        edgeCandidates = vector<int>(
                            edgeCandidates.begin() + half,
                            edgeCandidates.end()
                        );
                    }
                }
            }

            int p = parentCandidates[0];
            int edge = edgeCandidates[0];
            parent[v] = p;
            edgeToParent[v] = edge;
            unusedEdges.erase(edge);
        }
    }

    for (int v = 2; v <= n; ++v) {
        int edge = edgeToParent[v];
        int p = parent[v];
        if (preferred[edge] == '0') {
            originalS[edge] = p;
            originalF[edge] = v;
        } else {
            originalS[edge] = v;
            originalF[edge] = p;
        }
    }

    cout << "!";
    for (int i = 0; i < m; ++i) {
        cout << ' ' << originalS[i] << ' ' << originalF[i];
    }
    cout << '\n' << flush;

    return 0;
}

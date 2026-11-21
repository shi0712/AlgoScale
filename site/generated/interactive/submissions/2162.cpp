#include <bits/stdc++.h>
using namespace std;

const int WINDOW = 32;

int m;
int queryCount = 0;
int c = 0;

vector<int> prefixNode;
vector<int> repIndex;
vector<int> cnt;
vector<int> ceilLog;
vector<char> isBridge;

int failedBridge = 0;
int failedNonbridge = 0;

vector<int> hist;
int histIdx = 0, histSize = 0;
int recB = 0, recS = 0, recN = 0;

int ask(int p, int e) {
    ++queryCount;
    printf("? %d %d\n", p, e);
    fflush(stdout);

    int ans;
    if (scanf("%d", &ans) != 1) exit(0);
    if (ans < 0) exit(0);
    return ans;
}

void addOutcome(int type) {
    // type: 0 = bridge, 1 = selected representative, 2 = non-bridge excluded
    if (histSize == WINDOW) {
        int old = hist[histIdx];
        if (old == 0) --recB;
        else if (old == 1) --recS;
        else --recN;
    } else {
        ++histSize;
    }

    hist[histIdx] = type;
    histIdx = (histIdx + 1) % WINDOW;

    if (type == 0) ++recB;
    else if (type == 1) ++recS;
    else ++recN;
}

void selectEdge(int e, int qid) {
    ++c;
    prefixNode[c] = qid;
    repIndex[e] = c;
    cnt[c] = 1;
    addOutcome(1);
}

void markBridge(int e) {
    isBridge[e] = 1;
    ++failedBridge;
    addOutcome(0);
}

void markNonbridge(int e, int r) {
    repIndex[e] = r;
    ++cnt[r];
    ++failedNonbridge;
    addOutcome(2);
}

void classifyFailed(int e, int cc) {
    if (cc == 0) {
        markBridge(e);
        return;
    }

    int a = ceilLog[cc + 1];
    long long total = (long long)failedBridge + failedNonbridge;

    // Use "ask 0 first" if bridges have been frequent enough.
    bool useB = (total > 0 && failedBridge * 1LL * a > total);

    int r;
    if (useB) {
        int ans0 = ask(0, e);
        if (ans0 == 0) {
            r = 0;
        } else {
            int lo = 1, hi = cc;
            while (lo < hi) {
                int mid = (lo + hi) / 2;
                int ans = ask(prefixNode[mid], e);
                if (ans == 0) hi = mid;
                else lo = mid + 1;
            }
            r = lo;
        }
    } else {
        int lo = 0, hi = cc;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            int ans = ask(prefixNode[mid], e);
            if (ans == 0) hi = mid;
            else lo = mid + 1;
        }
        r = lo;
    }

    if (r == 0) markBridge(e);
    else markNonbridge(e, r);
}

int main() {
    if (scanf("%d", &m) != 1) return 0;

    prefixNode.assign(m + 2, 0);
    repIndex.assign(m + 1, 0);
    cnt.assign(m + 2, 0);
    isBridge.assign(m + 1, 0);
    ceilLog.assign(m + 2, 0);

    for (int i = 1; i <= m + 1; ++i) {
        int k = 0;
        while ((1LL << k) < i) ++k;
        ceilLog[i] = k;
    }

    hist.assign(WINDOW, 0);

    for (int e = 1; e <= m; ++e) {
        if (c == 0) {
            int ans = ask(0, e);
            if (ans == 1) selectEdge(e, queryCount);
            else markBridge(e);
            continue;
        }

        // If recently there were bridges but no selected representatives,
        // try to identify bridges with a single query first.
        bool useC = (recS == 0 && recB > 0);

        if (useC) {
            int ans0 = ask(0, e);
            if (ans0 == 0) {
                markBridge(e);
            } else {
                int lo = 1, hi = c + 1;
                int selectedQid = -1;

                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    int ans = ask(prefixNode[mid], e);
                    if (ans == 0) {
                        hi = mid;
                    } else {
                        lo = mid + 1;
                        if (mid == c) selectedQid = queryCount;
                    }
                }

                if (lo == c + 1) {
                    if (selectedQid == -1) {
                        // Should not be needed for a valid monotone oracle,
                        // but keeps the prefix chain well-defined.
                        ask(prefixNode[c], e);
                        selectedQid = queryCount;
                    }
                    selectEdge(e, selectedQid);
                } else {
                    markNonbridge(e, lo);
                }
            }
        } else {
            int ans = ask(prefixNode[c], e);
            if (ans == 1) {
                selectEdge(e, queryCount);
            } else {
                classifyFailed(e, c);
            }
        }
    }

    printf("!");
    for (int i = 1; i <= m; ++i) {
        int val;
        if (isBridge[i]) {
            val = -1;
        } else {
            int len = cnt[repIndex[i]];
            val = (len > 14 ? 0 : len);
        }
        printf(" %d", val);
    }
    printf("\n");
    fflush(stdout);

    int confirm;
    if (scanf("%d", &confirm) != 1) return 0;
    return 0;
}

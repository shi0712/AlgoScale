#include <bits/stdc++.h>
using namespace std;

int m;
vector<int> qid, cnt, cntSmall;
int queryCnt = 0;

int ask(int p, int e) {
    cout << "? " << p << " " << e << '\n' << flush;
    int ans;
    cin >> ans;
    if (ans == -1) exit(0);
    ++queryCnt;
    return ans;
}

void addRepBelow(int node, int l, int r, int pos, int e, int aloneId) {
    if (cnt[node] == 0) {
        qid[node] = aloneId;
    } else {
        ask(qid[node], e);
        qid[node] = queryCnt;
    }

    ++cnt[node];
    ++cntSmall[node];

    if (l == r) return;

    int mid = (l + r) >> 1;
    if (pos <= mid)
        addRepBelow(node * 2, l, mid, pos, e, aloneId);
    else
        addRepBelow(node * 2 + 1, mid + 1, r, pos, e, aloneId);
}

void removeSmallPath(int node, int l, int r, int pos) {
    --cntSmall[node];

    if (l == r) return;

    int mid = (l + r) >> 1;
    if (pos <= mid)
        removeSmallPath(node * 2, l, mid, pos);
    else
        removeSmallPath(node * 2 + 1, mid + 1, r, pos);
}

int findSmallMate(int c) {
    int node = 1, l = 1, r = m;

    while (l < r) {
        int mid = (l + r) >> 1;
        int left = node * 2;
        int right = left + 1;

        bool inLeft;
        if (cntSmall[left] == 0) {
            inLeft = false;
        } else if (cntSmall[right] == 0) {
            inLeft = true;
        } else {
            int t = ask(qid[left], c);
            inLeft = (t == 0);
        }

        if (inLeft) {
            node = left;
            r = mid;
        } else {
            node = right;
            l = mid + 1;
        }
    }

    return l;
}

struct Group {
    int count;
    bool big;
    int rep;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> m;

    vector<int> bridge(m + 1, 0);
    vector<int> aloneQid(m + 1, 0);
    vector<int> answer(m + 1, -2);
    vector<int> edgeGroup(m + 1, -1);
    vector<int> repGroup(m + 1, -1);

    vector<int> nonBridges;
    nonBridges.reserve(m);

    for (int e = 1; e <= m; ++e) {
        int ans = ask(0, e);
        aloneQid[e] = queryCnt;

        if (ans == 0) {
            bridge[e] = 1;
        } else {
            nonBridges.push_back(e);
        }
    }

    int sz = 4 * m + 5;
    qid.assign(sz, 0);
    cnt.assign(sz, 0);
    cntSmall.assign(sz, 0);

    vector<Group> groups;
    groups.reserve(m);

    int totalSmall = 0;
    int totalBig = 0;
    int qidBig = 0;

    for (int c : nonBridges) {
        if (cnt[1] == 0) {
            int g = (int)groups.size();
            groups.push_back({1, false, c});

            edgeGroup[c] = g;
            repGroup[c] = g;

            qid[1] = aloneQid[c];
            ++cnt[1];
            ++cntSmall[1];
            ++totalSmall;

            if (m > 1) {
                int mid = (1 + m) >> 1;
                if (c <= mid)
                    addRepBelow(2, 1, mid, c, c, aloneQid[c]);
                else
                    addRepBelow(3, mid + 1, m, c, c, aloneQid[c]);
            }

            continue;
        }

        int rootAns = ask(qid[1], c);

        if (rootAns == 1) {
            int g = (int)groups.size();
            groups.push_back({1, false, c});

            edgeGroup[c] = g;
            repGroup[c] = g;

            qid[1] = queryCnt;
            ++cnt[1];
            ++cntSmall[1];
            ++totalSmall;

            if (m > 1) {
                int mid = (1 + m) >> 1;
                if (c <= mid)
                    addRepBelow(2, 1, mid, c, c, aloneQid[c]);
                else
                    addRepBelow(3, mid + 1, m, c, c, aloneQid[c]);
            }
        } else {
            if (totalSmall == 0) {
                answer[c] = 0;
                continue;
            }

            bool isBig = false;

            if (totalBig > 0) {
                int t = ask(qidBig, c);
                if (t == 0) isBig = true;
            }

            if (isBig) {
                answer[c] = 0;
            } else {
                int r = findSmallMate(c);
                int g = repGroup[r];

                edgeGroup[c] = g;
                ++groups[g].count;

                if (groups[g].count == 15) {
                    groups[g].big = true;
                    --totalSmall;

                    removeSmallPath(1, 1, m, r);

                    if (totalBig == 0) {
                        qidBig = aloneQid[r];
                    } else {
                        ask(qidBig, r);
                        qidBig = queryCnt;
                    }

                    ++totalBig;
                }
            }
        }
    }

    vector<int> value(m + 1, 0);

    for (int e = 1; e <= m; ++e) {
        if (bridge[e]) {
            value[e] = -1;
        } else if (answer[e] == 0) {
            value[e] = 0;
        } else if (edgeGroup[e] != -1) {
            int g = edgeGroup[e];
            if (groups[g].big)
                value[e] = 0;
            else
                value[e] = groups[g].count;
        } else {
            value[e] = 0;
        }
    }

    cout << "!";
    for (int e = 1; e <= m; ++e) {
        cout << ' ' << value[e];
    }
    cout << '\n' << flush;

    int confirmation;
    if (cin >> confirmation) {
        if (confirmation == -1) return 0;
    }

    return 0;
}

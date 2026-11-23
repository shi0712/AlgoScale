#include <bits/stdc++.h>
using namespace std;

using Groups = vector<vector<int>>;

static int enqueueMineral(int id) {
    cout << "+ " << id << '\n' << flush;
    int d;
    if (!(cin >> d) || d < 0) exit(0);
    return d;
}

static int dequeueMineral() {
    cout << "-\n" << flush;
    int d;
    if (!(cin >> d) || d < 0) exit(0);
    return d;
}

static void clearBelt(int size) {
    for (int i = 0; i < size; ++i) dequeueMineral();
}

/*
 * Every class in bIds is known to occur among the classes in aIds.
 * Split aIds in parallel for all classes in bIds until their matches are found.
 */
static void locateMatches(Groups &a, Groups &b,
                          const vector<int> &aIds,
                          const vector<int> &bIds) {
    if (bIds.empty()) return;

    if (aIds.size() == 1) {
        int dst = aIds[0];
        for (int src : bIds) {
            a[dst].insert(a[dst].end(), b[src].begin(), b[src].end());
            b[src].clear();
        }
        return;
    }

    int leftSize = (int)aIds.size() / 2;
    int rightSize = (int)aIds.size() - leftSize;

    vector<int> leftA(aIds.begin(), aIds.begin() + leftSize);
    vector<int> rightA(aIds.begin() + leftSize, aIds.end());

    int previous = 0;
    for (int x : leftA) previous = enqueueMineral(a[x][0]);

    vector<int> leftB, rightB;
    leftB.reserve(bIds.size());
    rightB.reserve(bIds.size());

    int loadedB = 0;
    int pos = 0;
    int usedLeft = 0, usedRight = 0;
    int total = (int)bIds.size();

    while (pos < total) {
        if (usedLeft == leftSize) {
            for (; pos < total; ++pos) rightB.push_back(bIds[pos]);
            break;
        }
        if (usedRight == rightSize) {
            for (; pos < total; ++pos) leftB.push_back(bIds[pos]);
            break;
        }

        int x = bIds[pos++];
        int current = enqueueMineral(b[x][0]);
        ++loadedB;

        if (current == previous) {
            leftB.push_back(x);
            ++usedLeft;
        } else {
            rightB.push_back(x);
            ++usedRight;
        }
        previous = current;
    }

    clearBelt(leftSize + loadedB);

    locateMatches(a, b, leftA, leftB);
    locateMatches(a, b, rightA, rightB);
}

/* The overlap is the number of classes occurring in both partitions. */
static Groups mergePartitions(Groups left, Groups right, int overlap) {
    if (overlap == 0) {
        for (auto &g : right) left.push_back(std::move(g));
        return left;
    }

    Groups a = std::move(left);
    Groups b = std::move(right);

    /* Split the smaller collection of representatives during matching. */
    if (a.size() > b.size()) swap(a, b);

    int na = (int)a.size();
    int nb = (int)b.size();
    vector<char> matched(nb, 0);

    if (overlap == nb) {
        fill(matched.begin(), matched.end(), 1);
    } else {
        int previous = 0;
        for (int i = 0; i < na; ++i)
            previous = enqueueMineral(a[i][0]);

        int found = 0;
        int pos = 0;
        int loadedB = 0;

        while (pos < nb) {
            if (found == overlap) break;

            if (overlap - found == nb - pos) {
                for (; pos < nb; ++pos) matched[pos] = 1;
                found = overlap;
                break;
            }

            int current = enqueueMineral(b[pos][0]);
            if (current == previous) {
                matched[pos] = 1;
                ++found;
            }
            previous = current;
            ++pos;
            ++loadedB;
        }

        clearBelt(na + loadedB);
    }

    vector<int> aIds(na), bMatched;
    iota(aIds.begin(), aIds.end(), 0);
    for (int i = 0; i < nb; ++i)
        if (matched[i]) bMatched.push_back(i);

    locateMatches(a, b, aIds, bMatched);

    for (int i = 0; i < nb; ++i)
        if (!matched[i]) a.push_back(std::move(b[i]));

    return a;
}

static Groups classifyKnownCount(const vector<int> &ids, int distinct);

/* Starts and finishes with an empty belt. */
static Groups classify(const vector<int> &ids) {
    if (ids.size() == 1) return {{ids[0]}};

    int distinct = 0;
    for (int id : ids) distinct = enqueueMineral(id);
    clearBelt((int)ids.size());

    if (distinct == 1) return {ids};

    if (distinct == (int)ids.size()) {
        Groups result;
        for (int id : ids) result.push_back({id});
        return result;
    }

    return classifyKnownCount(ids, distinct);
}

/* distinct is the already known number of classes in ids. */
static Groups classifyKnownCount(const vector<int> &ids, int distinct) {
    int mid = (int)ids.size() / 2;
    vector<int> leftIds(ids.begin(), ids.begin() + mid);
    vector<int> rightIds(ids.begin() + mid, ids.end());

    Groups left = classify(leftIds);
    Groups right = classify(rightIds);

    int overlap = (int)left.size() + (int)right.size() - distinct;
    return mergePartitions(std::move(left), std::move(right), overlap);
}

static void printAnswer(const Groups &groups) {
    cout << "! " << groups.size() << '\n';
    for (const auto &g : groups) {
        cout << g.size();
        for (int id : g) cout << ' ' << id;
        cout << '\n';
    }
    cout << flush;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        printAnswer({{1}});
        return 0;
    }

    vector<int> ids(n);
    iota(ids.begin(), ids.end(), 1);

    int distinct = 0;
    for (int id : ids) distinct = enqueueMineral(id);

    if (distinct == 1) {
        printAnswer({ids});
        return 0;
    }

    if (distinct == n) {
        Groups result;
        for (int id : ids) result.push_back({id});
        printAnswer(result);
        return 0;
    }

    clearBelt(n);
    Groups answer = classifyKnownCount(ids, distinct);
    printAnswer(answer);
    return 0;
}

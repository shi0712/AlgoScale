#include <bits/stdc++.h>
using namespace std;

using ull = unsigned long long;
using Mask = array<ull, 10>;

int n;
map<Mask, int> queryCache;

vector<int> parentVertex, vertexDepth;
vector<char> discovered;
vector<int> traversalOrder;

int ask(const vector<int>& vertices) {
    if (vertices.empty()) return 0;

    Mask mask{};
    for (int v : vertices) {
        int x = v - 1;
        mask[x / 64] |= 1ULL << (x % 64);
    }

    int k = 0;
    for (ull word : mask) k += __builtin_popcountll(word);

    if (k == 1) {
        queryCache[mask] = 0;
        return 0;
    }

    auto it = queryCache.find(mask);
    if (it != queryCache.end()) return it->second;

    cout << "? " << k << '\n';
    bool first = true;
    for (int v = 1; v <= n; ++v) {
        int x = v - 1;
        if ((mask[x / 64] >> (x % 64)) & 1ULL) {
            if (!first) cout << ' ';
            first = false;
            cout << v;
        }
    }
    cout << '\n' << flush;

    int answer;
    if (!(cin >> answer) || answer < 0) exit(0);
    queryCache[mask] = answer;
    return answer;
}

void addChild(int u, int v) {
    discovered[v] = 1;
    parentVertex[v] = u;
    vertexDepth[v] = vertexDepth[u] + 1;
    traversalOrder.push_back(v);
}

int linearDiscover(int u, const vector<int>& vertices, int required) {
    int found = 0;
    int m = (int)vertices.size();

    for (int i = 0; i < m; ++i) {
        if (required >= 0) {
            int need = required - found;
            int left = m - i;
            if (need == 0) break;
            if (need == left) {
                for (; i < m; ++i) addChild(u, vertices[i]);
                found = required;
                break;
            }
        }

        if (ask(vector<int>{u, vertices[i]}) == 1) {
            addChild(u, vertices[i]);
            ++found;
        }
    }

    return required >= 0 ? required : found;
}

// state: 0 = unknown, 1 = independent set, 2 = complete graph
int discoverNeighbors(int u, const vector<int>& vertices, int required, int state) {
    int m = (int)vertices.size();
    if (m == 0 || required == 0) return 0;

    if (required == m) {
        for (int v : vertices) addChild(u, v);
        return m;
    }

    if (m == 1) {
        if (ask(vector<int>{u, vertices[0]}) == 1) {
            addChild(u, vertices[0]);
            return 1;
        }
        return 0;
    }

    if (state == 0 && m <= 4) {
        return linearDiscover(u, vertices, required);
    }

    if (required < 0) {
        int inside;
        int completeEdges = m * (m - 1) / 2;

        if (state == 1) inside = 0;
        else if (state == 2) inside = completeEdges;
        else inside = ask(vertices);

        if (state == 0) {
            if (inside == 0) state = 1;
            else if (inside == completeEdges) state = 2;
        }

        vector<int> withU = vertices;
        withU.push_back(u);
        required = ask(withU) - inside;

        if (required == 0) return 0;
        if (required == m) {
            for (int v : vertices) addChild(u, v);
            return m;
        }
    }

    int half = m / 2;
    vector<int> left(vertices.begin(), vertices.begin() + half);
    vector<int> right(vertices.begin() + half, vertices.end());

    int leftFound = discoverNeighbors(u, left, -1, state);
    int rightRequired = required - leftFound;
    discoverNeighbors(u, right, rightRequired, state);

    return required;
}

// A and B are independent sets, and at least one A-B edge exists.
int findEndpointInA(const vector<int>& A, const vector<int>& B) {
    if (A.size() == 1) return A[0];

    int half = (int)A.size() / 2;
    vector<int> left(A.begin(), A.begin() + half);
    vector<int> right(A.begin() + half, A.end());

    vector<int> test = left;
    test.insert(test.end(), B.begin(), B.end());

    if (ask(test) > 0) return findEndpointInA(left, B);
    return findEndpointInA(right, B);
}

// B is independent, and a has at least one neighbor in B.
int findEndpointInB(int a, const vector<int>& B) {
    if (B.size() == 1) return B[0];

    int half = (int)B.size() / 2;
    vector<int> left(B.begin(), B.begin() + half);
    vector<int> right(B.begin() + half, B.end());

    vector<int> test = left;
    test.push_back(a);

    if (ask(test) > 0) return findEndpointInB(a, left);
    return findEndpointInB(a, right);
}

pair<int, int> findEdge(const vector<int>& vertices, int edgeCount) {
    int m = (int)vertices.size();

    if (m == 2 || edgeCount == m * (m - 1) / 2) {
        return {vertices[0], vertices[1]};
    }

    int half = m / 2;
    vector<int> A(vertices.begin(), vertices.begin() + half);
    vector<int> B(vertices.begin() + half, vertices.end());

    int edgesA = ask(A);
    if (edgesA > 0) return findEdge(A, edgesA);

    int edgesB = ask(B);
    if (edgesB > 0) return findEdge(B, edgesB);

    // All edges must cross between the two independent sets A and B.
    int a = findEndpointInA(A, B);
    int b = findEndpointInB(a, B);
    return {a, b};
}

void printVertices(const vector<int>& vertices) {
    for (int i = 0; i < (int)vertices.size(); ++i) {
        if (i) cout << ' ';
        cout << vertices[i];
    }
    cout << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "Y 1\n1\n" << flush;
        return 0;
    }

    parentVertex.assign(n + 1, 0);
    vertexDepth.assign(n + 1, 0);
    discovered.assign(n + 1, 0);

    discovered[1] = 1;
    traversalOrder.push_back(1);

    vector<int> remaining;
    for (int v = 2; v <= n; ++v) remaining.push_back(v);

    size_t head = 0;
    while (!remaining.empty()) {
        int u = traversalOrder[head++];
        discoverNeighbors(u, remaining, -1, 0);

        vector<int> nextRemaining;
        for (int v : remaining) {
            if (!discovered[v]) nextRemaining.push_back(v);
        }
        remaining.swap(nextRemaining);
    }

    vector<int> sides[2];
    for (int v = 1; v <= n; ++v) {
        sides[vertexDepth[v] & 1].push_back(v);
    }

    int edges0 = ask(sides[0]);
    int edges1 = ask(sides[1]);

    if (edges0 == 0 && edges1 == 0) {
        cout << "Y " << sides[0].size() << '\n';
        printVertices(sides[0]);
        cout << flush;
        return 0;
    }

    int chosen;
    if (edges0 > 0 && (edges1 == 0 || sides[0].size() <= sides[1].size())) {
        chosen = 0;
    } else {
        chosen = 1;
    }

    int sameSideEdges = chosen == 0 ? edges0 : edges1;
    auto [u, v] = findEdge(sides[chosen], sameSideEdges);

    int a = u, b = v;
    vector<int> leftPath, rightPath;

    while (vertexDepth[a] > vertexDepth[b]) {
        leftPath.push_back(a);
        a = parentVertex[a];
    }
    while (vertexDepth[b] > vertexDepth[a]) {
        rightPath.push_back(b);
        b = parentVertex[b];
    }
    while (a != b) {
        leftPath.push_back(a);
        rightPath.push_back(b);
        a = parentVertex[a];
        b = parentVertex[b];
    }

    leftPath.push_back(a);
    reverse(rightPath.begin(), rightPath.end());
    leftPath.insert(leftPath.end(), rightPath.begin(), rightPath.end());

    cout << "N " << leftPath.size() << '\n';
    printVertices(leftPath);
    cout << flush;
    return 0;
}

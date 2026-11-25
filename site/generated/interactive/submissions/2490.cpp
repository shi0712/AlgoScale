#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

int n;
vector<vector<unsigned char>> edge;
vector<vector<int>> adjacent;
vector<int> dsu, componentSize, parity;
vector<int> color, parent, depth;
vector<int> answerCycle;

int ask(vector<int> vertices) {
    sort(vertices.begin(), vertices.end());
    vertices.erase(unique(vertices.begin(), vertices.end()), vertices.end());
    cout << "? " << vertices.size() << '\n';
    for (int i = 0; i < (int)vertices.size(); ++i) {
        if (i) cout << ' ';
        cout << vertices[i];
    }
    cout << '\n' << flush;

    int result;
    if (!(cin >> result)) exit(0);
    if (result < 0) exit(0);
    return result;
}

int inducedEdges(const vector<int>& vertices) {
    int result = 0;
    for (int i = 0; i < (int)vertices.size(); ++i)
        for (int j = 0; j < i; ++j)
            result += edge[vertices[i]][vertices[j]];
    return result;
}

int findRoot(int v) {
    if (dsu[v] == v) return v;
    int oldParent = dsu[v];
    int root = findRoot(oldParent);
    parity[v] ^= parity[oldParent];
    return dsu[v] = root;
}

bool joinVertices(int v, int u) {
    int rv = findRoot(v), ru = findRoot(u);
    if (rv == ru) return (parity[v] ^ parity[u]) == 1;
    if (componentSize[rv] > componentSize[ru]) {
        swap(rv, ru);
        swap(v, u);
    }
    dsu[rv] = ru;
    parity[rv] = parity[v] ^ parity[u] ^ 1;
    componentSize[ru] += componentSize[rv];
    return true;
}

void makeOddCycle(int v, int u) {
    vector<int> left{v}, right{u};
    int a = v, b = u;
    while (depth[a] > depth[b]) {
        a = parent[a];
        left.push_back(a);
    }
    while (depth[b] > depth[a]) {
        b = parent[b];
        right.push_back(b);
    }
    while (a != b) {
        a = parent[a];
        b = parent[b];
        left.push_back(a);
        right.push_back(b);
    }
    right.pop_back();
    reverse(right.begin(), right.end());
    left.insert(left.end(), right.begin(), right.end());
    answerCycle = move(left);
}

void buildOddCycle(int current) {
    fill(color.begin(), color.end(), -1);
    fill(parent.begin(), parent.end(), -1);
    fill(depth.begin(), depth.end(), 0);
    for (int start = 1; start <= current && answerCycle.empty(); ++start) {
        if (color[start] != -1) continue;
        vector<int> queue{start};
        color[start] = 0;
        for (int head = 0; head < (int)queue.size() && answerCycle.empty(); ++head) {
            int v = queue[head];
            for (int u : adjacent[v]) {
                if (color[u] == -1) {
                    color[u] = color[v] ^ 1;
                    parent[u] = v;
                    depth[u] = depth[v] + 1;
                    queue.push_back(u);
                } else if (color[u] == color[v]) {
                    makeOddCycle(v, u);
                    break;
                }
            }
        }
    }
}

bool addEdge(int v, int u) {
    edge[v][u] = edge[u][v] = 1;
    adjacent[v].push_back(u);
    adjacent[u].push_back(v);
    if (joinVertices(v, u)) return true;
    buildOddCycle(v);
    return false;
}

bool resolveGroup(const vector<int>& vertices, int v, int edgesToGroup) {
    if (edgesToGroup == 0) return true;
    if (edgesToGroup == (int)vertices.size()) {
        for (int u : vertices)
            if (!addEdge(v, u)) return false;
        return true;
    }

    vector<int> first, second;
    for (int u : vertices) {
        findRoot(u);
        (parity[u] == 0 ? first : second).push_back(u);
    }
    if (first.empty() || second.empty()) {
        first.clear();
        second.clear();
        int middle = vertices.size() / 2;
        first.assign(vertices.begin(), vertices.begin() + middle);
        second.assign(vertices.begin() + middle, vertices.end());
    }

    vector<int> query = first;
    query.push_back(v);
    int firstEdges = ask(query) - inducedEdges(first);
    int secondEdges = edgesToGroup - firstEdges;
    return resolveGroup(first, v, firstEdges) &&
           resolveGroup(second, v, secondEdges);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    edge.assign(n + 1, vector<unsigned char>(n + 1, 0));
    adjacent.resize(n + 1);
    dsu.resize(n + 1);
    componentSize.assign(n + 1, 1);
    parity.assign(n + 1, 0);
    color.resize(n + 1);
    parent.resize(n + 1);
    depth.resize(n + 1);
    for (int v = 1; v <= n; ++v) dsu[v] = v;

    vector<int> previous{1};
    for (int v = 2; v <= n; ++v) {
        vector<int> query = previous;
        query.push_back(v);
        int edgesToPrevious = ask(query) - inducedEdges(previous);
        if (!resolveGroup(previous, v, edgesToPrevious)) break;
        previous.push_back(v);
    }

    if (!answerCycle.empty()) {
        cout << "N " << answerCycle.size() << '\n';
        for (int i = 0; i < (int)answerCycle.size(); ++i) {
            if (i) cout << ' ';
            cout << answerCycle[i];
        }
        cout << '\n' << flush;
    } else {
        vector<int> side;
        for (int v = 1; v <= n; ++v) {
            findRoot(v);
            if (parity[v] == 0) side.push_back(v);
        }
        cout << "Y " << side.size() << '\n';
        for (int i = 0; i < (int)side.size(); ++i) {
            if (i) cout << ' ';
            cout << side[i];
        }
        cout << '\n' << flush;
    }
    return 0;
}

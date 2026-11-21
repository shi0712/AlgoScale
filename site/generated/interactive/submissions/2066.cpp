#include <bits/stdc++.h>
using namespace std;

constexpr int INF = 1000000000;
constexpr double UNBOUNDED = 1e30;
enum Action { LEFT, RIGHT, STEP };
const char *command[] = {"left", "right", "step"};

struct Map {
    int rows, cols;
    vector<string> grid;
    vector<int> cls, obs, representative, weight, common, solo;
    array<int,4> stride;

    bool open(int r, int c) const {
        return r >= 0 && r < rows && c >= 0 && c < cols && grid[r][c] == '.';
    }

    int move(int s, int a) const {
        if (a == STEP) return s + 4 * stride[s & 3];
        return (s & ~3) + ((s + (a == RIGHT ? 1 : 3)) & 3);
    }

    void build() {
        int n = rows * cols;
        stride = {-cols, 1, cols, -1};
        const int dr[] = {-1,0,1,0}, dc[] = {0,1,0,-1};
        cls.assign(4*n, -1);
        obs.resize(4*n);
        for (int d = 0; d < 4; ++d) {
            for (int i = 0; i < n; ++i) {
                int p = (d == 1 || d == 2) ? n-1-i : i;
                if (open(p/cols + dr[d], p%cols + dc[d]))
                    obs[4*p+d] = 1 + obs[4*(p+stride[d])+d];
            }
        }

        vector<bool> used(n);
        map<vector<int>,int> shapes;
        for (int p = 0; p < n; ++p) {
            if (used[p] || !open(p/cols,p%cols)) continue;
            vector<int> component{p};
            used[p] = true;
            for (size_t i = 0; i < component.size(); ++i) {
                int q = component[i];
                for (int d = 0; d < 4; ++d) {
                    int t = q + stride[d];
                    if (open(q/cols+dr[d], q%cols+dc[d]) && !used[t]) {
                        component.push_back(t);
                        used[t] = true;
                    }
                }
            }

            // Express the entire component in the observer's coordinate frame.
            // Equal normalized shapes and equal offsets give identical futures.
            for (int d = 0; d < 4; ++d) {
                vector<pair<int,int>> points;
                int minR = INF, minC = INF;
                for (int q : component) {
                    int r = q/cols, c = q%cols;
                    for (int j = 0; j < d; ++j) {
                        int oldR = r;
                        r = -c;
                        c = oldR;
                    }
                    minR = min(minR,r);
                    minC = min(minC,c);
                    points.emplace_back(r*101+c,q);
                }
                sort(points.begin(),points.end());
                vector<int> shape;
                for (auto [coord,q] : points)
                    shape.push_back(coord-minR*101-minC);
                int base = representative.size();
                auto [it,inserted] = shapes.emplace(std::move(shape),base);
                base = it->second;
                if (inserted) {
                    representative.resize(base+points.size());
                    common.resize(base+points.size());
                    weight.resize(base+points.size());
                }
                for (int j = 0; j < (int)points.size(); ++j) {
                    int id = base+j, q = points[j].second, s = 4*q+d;
                    cls[s] = id;
                    if (!weight[id]) {
                        representative[id] = s;
                        common[id] = q;
                    } else if (common[id] != q) {
                        common[id] = -1;
                    }
                    ++weight[id];
                }
            }
        }

        // Reverse shortest paths to a class whose members share a square.
        int m = weight.size();
        solo.assign(m, INF);
        vector<int> queue;
        for (int i = 0; i < m; ++i) if (common[i] >= 0) {
            solo[i] = 0;
            queue.push_back(i);
        }
        for (size_t i = 0; i < queue.size(); ++i) {
            int id = queue[i], s = representative[id], d = s & 3;
            auto visit = [&](int prev) {
                int j = cls[prev];
                if (solo[j] == INF) {
                    solo[j] = solo[id]+1;
                    queue.push_back(j);
                }
            };
            visit(move(s,LEFT));
            visit(move(s,RIGHT));
            if (obs[(s & ~3) + ((d+2)&3)] > 0) visit(s-4*stride[d]);
        }
    }
};

struct Bounds {
    double low = 0, estimate = 0, upper = 0;
};

struct VectorHash {
    size_t operator()(const vector<int>& v) const {
        size_t h = v.size();
        for (int x : v) h = (h ^ (x+1)) * 1099511628211ULL;
        return h;
    }
};

struct Option {
    int pose;
    double cost;
    vector<int> children;
};

struct Node {
    const vector<int>* belief;
    vector<Option> options;
    int mass;
    Bounds value;
    bool expanded = false;
    double direct = UNBOUNDED;
    int directPose = -1;
    double frontier = UNBOUNDED;
};

// Between informative observations, every possible start follows the same
// relative path. BFS of one representative's poses enumerates shortest prefixes
// ending in information or a common position. A split strictly reduces each
// child's number of classes, so the resulting planning graph is acyclic.
class Planner {
    const Map& m;
    deque<Node> nodes;
    unordered_map<vector<int>,int,VectorHash> cache;
    vector<int> stamp, rootParent;
    vector<unsigned char> rootAction;
    int epoch = 0;
    size_t storedIds = 0, storedOptions = 0, storedEdges = 0;
    double deadline;
    bool outOfTime = false;
    int work = 0;

    bool limited() {
        if (outOfTime) return true;
        if (storedIds > 3300000 || nodes.size() > 240000 ||
            storedOptions > 650000 || storedEdges > 4000000 ||
            (work >= 1024 && double(clock()) / CLOCKS_PER_SEC >= deadline))
            outOfTime = true;
        if (work >= 1024) work = 0;
        return outOfTime;
    }

    Bounds initial(const vector<int>& b, int& mass) const {
        int cell = m.common[b[0]];
        bool good = false;
        double travel = 0, lb = 0;
        mass = 0;
        for (int x : b) {
            if (cell != m.common[x]) cell = -1;
            int w = m.weight[x], d = m.solo[x];
            if (d != INF) good = true;
            else d = 0;
            travel += double(w)*d;
            lb += double(w)*max(1,d);
            mass += w;
        }
        if (cell >= 0 || !good) return {};
        if (b.size() == 1) return {travel,travel,travel};
        double diagnosis = mass*(0.8+1.25*log2(double(b.size())));
        return {lb,max(lb,travel+diagnosis),UNBOUNDED};
    }

    int intern(vector<int>& b, int mass, Bounds value) {
        sort(b.begin(), b.end());
        auto [it, fresh] = cache.try_emplace(b, nodes.size());
        if (fresh) {
            storedIds += b.size();
            nodes.push_back({&it->first,{},mass,value});
        }
        return it->second;
    }

    Bounds evaluate(const Option& option) const {
        Bounds v{option.cost,option.cost,option.cost};
        for (int id : option.children) {
            const Bounds& child = nodes[id].value;
            v.low += child.low;
            v.estimate += child.estimate;
            v.upper = min(UNBOUNDED,v.upper+child.upper);
        }
        return v;
    }

    int refresh(int id) {
        Node& node = nodes[id];
        Bounds best{node.direct,node.direct,node.direct};
        int pose = node.directPose;
        for (const Option& o : node.options) {
            Bounds v = evaluate(o);
            best.low = min(best.low,v.low);
            best.upper = min(best.upper,v.upper);
            if (v.estimate < best.estimate) {
                best.estimate = v.estimate;
                pose = o.pose;
            }
        }
        best.low = min(best.low,node.frontier);
        node.value = best;
        return pose;
    }

    void expand(int id) {
        Node& node = nodes[id];
        const int k = node.belief->size(), mass = node.mass;
        vector<int> starts;
        for (int x : *node.belief) starts.push_back(m.representative[x]);
        int anchor = starts[0], ad = anchor&3, ap = anchor/4;
        vector<pair<int,int>> queue{{anchor,0}};
        stamp[anchor] = ++epoch;
        vector<int> current(k);
        array<vector<int>,100> groups;
        vector<int> observations;
        bool found = node.expanded;
        double bestGuess = found ? node.value.estimate : UNBOUNDED;
        double bestUpper = found ? node.value.upper : UNBOUNDED;
        double previousLimit = found ? node.frontier : 0;
        double extendThrough = found ? max(previousLimit+8*mass, bestGuess+3*mass) : 0;
        double frontier = UNBOUNDED;

        for (size_t qi = 0; qi < queue.size(); ++qi) {
            auto [s,len] = queue[qi];
            double cost = double(len)*mass;
            if (cost >= bestUpper) break;
            // Defer distant prefixes. Their length alone bounds their cost.
            // Reopen this frontier if it can compete with the refined plans.
            if (k > 2 && cost > max(extendThrough, bestGuess+5.0*mass)) {
                frontier = cost;
                break;
            }
            work += k+8;
            if (limited() && (found || id != 0)) {
                frontier = cost;
                break;
            }

            int p = s/4, rot = ((s&3)-ad+4)&3;
            int dy = p/m.cols-ap/m.cols, dx = p%m.cols-ap%m.cols;
            int delta[4];
            for (int j = 0; j < 4; ++j) {
                int d = (ad+j)&3;
                delta[d] = 4*(dy*m.cols+dx) + ((d+rot)&3)-d;
                int oldY = dy;
                dy = dx;
                dx = -oldY;
            }
            int observation = m.obs[s], cell = m.common[m.cls[s]];
            bool split = false;
            double travel = 0;
            for (int i = 0; i < k; ++i) {
                int t = starts[i]+delta[starts[i]&3];
                current[i] = t;
                int x = m.cls[t];
                split |= m.obs[t] != observation;
                if (m.common[x] != cell) cell = -1;
                if (m.solo[x] != INF) travel += double(m.solo[x])*m.weight[x];
            }
            if (cost+travel >= bestUpper) continue;
            if (cell >= 0) {
                if (cost < previousLimit) continue;
                node.direct = bestUpper = min(bestUpper,cost);
                node.directPose = s;
                bestGuess = min(bestGuess,cost);
                found = true;
            } else if (split) {
                if (cost < previousLimit) continue;
                observations.clear();
                for (int t : current) {
                    int d = m.obs[t];
                    if (groups[d].empty()) observations.push_back(d);
                    groups[d].push_back(m.cls[t]);
                }
                Option o{s,cost,{}};
                for (int d : observations) {
                    auto& group = groups[d];
                    int w;
                    Bounds v = initial(group,w);
                    if (v.low == v.upper) o.cost += v.upper;
                    else {
                        int child = intern(group,w,v);
                        const Bounds& cv = nodes[child].value;
                        if (cv.low == cv.upper) o.cost += cv.upper;
                        else o.children.push_back(child);
                    }
                    group.clear();
                }
                Bounds v = evaluate(o);
                if (v.low >= bestUpper) continue;
                if (o.children.empty()) {
                    node.direct = o.cost;
                    node.directPose = s;
                } else {
                    storedEdges += o.children.capacity();
                    node.options.push_back(std::move(o));
                    ++storedOptions;
                }
                bestUpper = min(bestUpper,v.upper);
                bestGuess = min(bestGuess,v.estimate);
                found = true;
            } else {
                for (int a : {LEFT,RIGHT,STEP}) {
                    if (a == STEP && !observation) continue;
                    int next = m.move(s,a);
                    if (stamp[next] == epoch) continue;
                    stamp[next] = epoch;
                    if (id == 0) {
                        rootParent[next] = s;
                        rootAction[next] = a;
                    }
                    queue.emplace_back(next,len+1);
                }
            }
        }
        if (found) {
            node.expanded = true;
            node.frontier = frontier;
            refresh(id);
        }
    }

    void refine(int id) {
        Node& node = nodes[id];
        if (!node.expanded) {
            expand(id);
            return;
        }
        refresh(id);
        if (node.value.low == node.value.upper) return;
        double priority = UNBOUNDED;
        int next = -1;
        for (const Option& o : node.options) {
            Bounds v = evaluate(o);
            if (v.low >= node.value.upper) continue;
            double score = 0.35*v.low+0.65*v.estimate;
            if (score >= priority) continue;
            priority = score;
            double largest = -1;
            for (int child : o.children) {
                const Node& ch = nodes[child];
                if (ch.value.low == ch.value.upper) continue;
                double gap = ch.value.estimate-ch.value.low + 0.5*ch.mass;
                if (gap > largest) {
                    largest = gap;
                    next = child;
                }
            }
        }
        if (node.frontier < node.value.upper &&
            (next < 0 || node.frontier + 2*node.mass < priority)) {
            expand(id);
            return;
        }
        if (next >= 0) refine(next);
        refresh(id);
    }
public:
    Planner(const Map& m, double deadline)
        : m(m), stamp(m.cls.size()), rootParent(m.cls.size()),
          rootAction(m.cls.size()), deadline(deadline) {}

    vector<int> plan(vector<int> belief) {
        int mass;
        Bounds v = initial(belief,mass);
        intern(belief,mass,v);
        expand(0);
        while (!limited() && nodes[0].value.low < nodes[0].value.upper) {
            work += 256;
            refine(0);
        }
        int end = refresh(0);
        int start = m.representative[nodes[0].belief->front()];
        vector<int> path;
        for (int s = end; s != start; s = rootParent[s])
            path.push_back(rootAction[s]);
        reverse(path.begin(),path.end());
        return path;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Map m;
    if (!(cin >> m.rows >> m.cols)) return 0;
    m.grid.resize(m.rows);
    for (auto& row : m.grid) if (!(cin >> row)) return 0;
    m.build();
    int d;
    if (!(cin >> d) || d < 0 || d >= 100) return 0;
    vector<int> b;
    for (int x = 0; x < (int)m.weight.size(); ++x)
        if (m.obs[m.representative[x]] == d) b.push_back(x);
    vector<int> plan;
    size_t index = 0;
    while (!b.empty()) {
        int cell = m.common[b[0]];
        bool possible = false;
        for (int x : b) {
            if (m.common[x] != cell) cell = -1;
            possible |= m.solo[x] != INF;
        }
        if (cell >= 0) {
            cout << "yes " << cell/m.cols+1 << ' ' << cell%m.cols+1 << endl;
            return 0;
        }
        if (!possible) {
            cout << "no" << endl;
            return 0;
        }

        int a = -1;
        if (b.size() == 1) {
            int x = b[0], s = m.representative[x];
            for (int t : {LEFT,RIGHT,STEP}) {
                if (t == STEP && !d) continue;
                if (m.solo[m.cls[m.move(s,t)]] == m.solo[x]-1) {
                    a = t;
                    break;
                }
            }
        } else {
            if (index == plan.size()) {
                double now = double(clock()) / CLOCKS_PER_SEC;
                double budget = clamp((7.4-now)*0.16,0.0,0.85);
                plan = Planner(m,now+budget).plan(b);
                index = 0;
            }
            a = plan[index++];
        }
        if (a < 0 || (a == STEP && !d)) return 0;
        cout << command[a] << endl;
        if (!(cin >> d) || d < 0 || d >= 100) return 0;
        size_t size = b.size(), keep = 0;
        for (int x : b) {
            int t = m.move(m.representative[x],a);
            if (m.obs[t] == d) b[keep++] = m.cls[t];
        }
        b.resize(keep);
        if (keep != size) index = plan.size();
    }
}

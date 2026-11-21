#include <bits/stdc++.h>
using namespace std;

struct Cell {
    int x, y;
};

constexpr int LIM = 1000;
unsigned char black[LIM + 1][LIM + 1];
constexpr int dx[8] = {-1,-1,-1,0,0,1,1,1};
constexpr int dy[8] = {-1,0,1,-1,1,-1,0,1};

bool markable(int x, int y) {
    return x >= 1 && x <= LIM && y >= 1 && y <= LIM;
}

bool white(int x, int y) {
    return x > 0 && y > 0 && (!markable(x, y) || !black[x][y]);
}

int degree(int x, int y) {
    int n = 0;
    for (int i = 0; i < 8; ++i)
        n += white(x + dx[i], y + dy[i]);
    return n;
}

bool transposed;
int x, y;

bool play(Cell p) {
    black[p.x][p.y] = 1;
    if (transposed) cout << p.y << ' ' << p.x << '\n';
    else cout << p.x << ' ' << p.y << '\n';
    cout.flush();

    int a, b;
    if (!(cin >> a >> b) || a <= 0 || b <= 0) return false;
    x = transposed ? b : a;
    y = transposed ? a : b;
    return true;
}

// A successful tactical search stores the whole strategy, not just its first move.
struct Node {
    Cell move;
    array<int, 8> child;
};

vector<Node> tree;
int work;
long long tacticalBudget = 65000000;

int addNode(Cell p) {
    Node n;
    n.move = p;
    n.child.fill(-1);
    tree.push_back(n);
    return int(tree.size()) - 1;
}

// A sufficient condition for the robot to survive: at move t it can
// choose from t+1 surviving continuations on the current board, whereas
// at most t of those destinations can have been newly marked.
int forcedBlock(int a, int b, int depth) {
    constexpr int C = 7, S = 15;
    unsigned char layer[2][S][S] = {};
    unsigned char available[S][S] = {};
    for (int i = -depth; i <= depth; ++i)
        for (int j = -depth; j <= depth; ++j)
            available[C+i][C+j] = white(a+i, b+j);

    int prev = 0;
    memset(layer[0], 1, sizeof(layer[0]));
    work -= (2*depth+1)*(2*depth+1);
    for (int t = depth; t >= 2; --t) {
        int next = prev ^ 1;
        for (int i = C-t+1; i <= C+t-1; ++i) {
            for (int j = C-t+1; j <= C+t-1; ++j) {
                int cnt = 0;
                if (available[i][j]) {
                    for (int k = 0; k < 8; ++k)
                        cnt += available[i+dx[k]][j+dy[k]] &&
                               layer[prev][i+dx[k]][j+dy[k]];
                }
                layer[next][i][j] = cnt > t;
            }
        }
        work -= 8*(2*t-1)*(2*t-1);
        prev = next;
    }

    int forced = -1;
    for (int k = 0; k < 8; ++k) {
        if (layer[prev][C+dx[k]][C+dy[k]]) {
            if (forced != -1) return -2;
            forced = k;
        }
    }
    return forced;
}

int solveLocal(int a, int b, int depth) {
    work -= 12;
    array<int, 8> moves{};
    int nm = 0;
    for (int i = 0; i < 8; ++i)
        if (white(a + dx[i], b + dy[i])) moves[nm++] = i;
    if (nm == 0) return addNode({a, b});
    if (nm == 1 && markable(a+dx[moves[0]], b+dy[moves[0]]))
        return addNode({a+dx[moves[0]], b+dy[moves[0]]});
    if (depth == 1 || work <= 0) return -1;

    int deg[8] = {};
    for (int j = 0; j < nm; ++j) {
        int i = moves[j];
        deg[i] = degree(a+dx[i], b+dy[i]);
    }
    work -= 8*nm;
    sort(moves.begin(), moves.begin()+nm, [&](int i, int j) {
        return deg[i] > deg[j];
    });

    array<pair<int, Cell>, 169> candidates{};
    int nc = 0;
    auto offer = [&](int px, int py, int score) {
        if (markable(px, py) && !black[px][py])
            candidates[nc++] = {score, {px, py}};
    };

    if (depth == 2) {
        int i = moves[0], u = a+dx[i], v = b+dy[i];
        offer(u, v, 10);
        if (deg[i] == 2) {
            for (int k = 0; k < 8; ++k)
                offer(u+dx[k], v+dy[k], 0);
        }
    } else {
        int forced = forcedBlock(a, b, depth);
        if (forced == -2 || work <= 0) return -1;
        if (forced >= 0) {
            offer(a+dx[forced], b+dy[forced], 0);
        } else {
            for (int i = -depth; i <= depth; ++i) {
                for (int j = -depth; j <= depth; ++j) {
                    int score = 0;
                    for (int k = 0; k < nm; ++k) {
                        int m = moves[k];
                        int d = max(abs(i-dx[m]), abs(j-dy[m]));
                        if (d == 0) score += 18 + deg[m];
                        else if (d == 1) score += 4;
                        else if (d < depth) ++score;
                    }
                    offer(a+i, b+j, score);
                }
            }
            work -= (2*depth+1)*(2*depth+1)*nm;
            sort(candidates.begin(), candidates.begin()+nc,
                 [](const auto& p, const auto& q) { return p.first > q.first; });
        }
    }

    for (int j = 0; j < nc && work > 0; ++j) {
        Cell p = candidates[j].second;
        int id = addNode(p);
        black[p.x][p.y] = 1;
        bool ok = true;
        for (int k = 0; k < nm; ++k) {
            int i = moves[k];
            int u = a+dx[i], v = b+dy[i];
            if (u == p.x && v == p.y) continue;
            int child = solveLocal(u, v, depth-1);
            if (child == -1) {
                ok = false;
                break;
            }
            tree[id].child[i] = child;
        }
        black[p.x][p.y] = 0;
        if (ok) return id;
        tree.resize(id);
    }
    return -1;
}

int tactical() {
    int allowance = int(min(200000LL, max(0LL, tacticalBudget)));
    work = allowance;
    int result = -1;
    tree.clear();
    for (int d = 1; d <= 6; ++d) {
        if (d > 1 && work <= 0) break;
        result = solveLocal(x, y, d);
        if (result != -1) break;
    }
    tacticalBudget -= max(0, allowance - work);
    return result;
}

void finish(int id) {
    while (id >= 0) {
        int oldx = x, oldy = y;
        if (!play(tree[id].move)) return;
        int next = -1;
        for (int k = 0; k < 8; ++k)
            if (x == oldx+dx[k] && y == oldy+dy[k])
                next = tree[id].child[k];
        id = next;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int a, b;
    if (!(cin >> a >> b) || a <= 0 || b <= 0) return 0;
    transposed = a > b;
    if (transposed) swap(a, b);
    x = a;
    y = b;

    const int H = b + 2*a + 17;
    int W = 0, seed = 0, need = 0;
    bool up = false;

    // Build the top wall with one hole per three cells. Stop when the
    // right edge is far enough away to establish a tracking interval.
    while (true) {
        int id = tactical();
        if (id >= 0) {
            finish(id);
            return 0;
        }

        int z = min(y, H-2);
        int su = min(z+1, H-1), sd = max(z-1, 1);
        int qu = su-1+2, qd = H-1-sd+2;
        up = qu <= qd;
        seed = up ? su : sd;
        need = min(qu, qd);

        int d = W-x;
        if (d > 0 && d-1-(d+1)/3 >= need) break;

        ++W;
        if (W % 3 == 0) ++W;
        if (!play({W, H})) return 0;
    }

    int lo = 0, hi = 0, built = 0;
    int left = 1, right = W-1, bottom = 1, top = H-1;
    int cut = 0;
    bool vertical = false;
    vector<vector<int>> cost;

    while (true) {
        int id = tactical();
        if (id >= 0) {
            finish(id);
            return 0;
        }

        // There is at most one top-wall hole in the three destinations.
        // On these turns min(y,H-2) does not change.
        int hole = 0;
        if (y == H-1) {
            for (int u = max(1, x-1); u <= min(W, x+1); ++u)
                if (!black[u][H]) hole = u;
        }
        if (hole) {
            if (!play({hole, H})) return 0;
            continue;
        }

        int z = min(y, H-2);
        if (built < need && (lo != 1 || hi != H-1)) {
            Cell p{W, seed};
            if (built == 0) {
                lo = hi = seed;
            } else if (up) {
                if (hi < z+1 && hi < H-1) p.y = ++hi;
                else if (lo > 1) p.y = --lo;
                else p.y = ++hi;
            } else {
                if (lo > z-1 && lo > 1) p.y = --lo;
                else if (hi < H-1) p.y = ++hi;
                else p.y = --lo;
            }
            ++built;
            if (!play(p)) return 0;
            continue;
        }

        // From here both outside walls can be maintained. A completed
        // interior cut lets us discard whichever side the robot left.
        if (cost.empty()) {
            cost.assign(W, vector<int>(H, 0));
            for (int w = 1; w < W; ++w) {
                for (int h = 1; h < H; ++h) {
                    int best = w*h;
                    for (int i = 0; i < w; ++i)
                        best = min(best, h + max(cost[i][h], cost[w-i-1][h]));
                    for (int i = 0; i < h; ++i)
                        best = min(best, w + max(cost[w][i], cost[w][h-i-1]));
                    cost[w][h] = best;
                }
            }
        }

        bool sent = false;
        while (!sent) {
            if (cut) {
                Cell p{0, 0};
                int dist = INT_MAX;
                if (vertical) {
                    for (int v = bottom; v <= top; ++v) {
                        if (!black[cut][v] && abs(v-y) < dist) {
                            p = {cut, v};
                            dist = abs(v-y);
                        }
                    }
                    if (!p.x) {
                        if (x < cut) right = cut-1;
                        else left = cut+1;
                        cut = 0;
                        continue;
                    }
                } else {
                    for (int u = left; u <= right; ++u) {
                        if (!black[u][cut] && abs(u-x) < dist) {
                            p = {u, cut};
                            dist = abs(u-x);
                        }
                    }
                    if (!p.x) {
                        if (y < cut) top = cut-1;
                        else bottom = cut+1;
                        cut = 0;
                        continue;
                    }
                }

                if (right == W-1) {
                    if (lo > max(1, z-1)) p = {W, --lo};
                    else if (hi < min(H-1, z+1)) p = {W, ++hi};
                }
                if (!play(p)) return 0;
                sent = true;
                continue;
            }

            int w = right-left+1, h = top-bottom+1;
            if (w == 1 && h == 1) {
                Cell p{x, y};
                if (right == W-1) {
                    if (lo > max(1, z-1)) p = {W, --lo};
                    else if (hi < min(H-1, z+1)) p = {W, ++hi};
                }
                if (!play(p)) return 0;
                sent = true;
                continue;
            }

            int delay = 0;
            if (right == W-1) delay += H-1-(hi-lo+1);
            if (top == H-1)
                for (int u = max(1,left-1); u <= min(W,right+1); ++u)
                    delay += !black[u][H];

            tuple<int,int,int> best{INT_MAX, INT_MAX, INT_MAX};
            auto consider = [&](bool v, int k, int missing, int first, int second) {
                int pos = v ? x : y;
                if (pos + missing + delay <= k) second = 0;
                if (pos - missing - delay >= k) first = 0;
                auto val = make_tuple(missing+max(first,second), missing, abs(pos-k));
                if (val < best) {
                    best = val;
                    cut = k;
                    vertical = v;
                }
            };

            for (int u = left; u <= right; ++u) {
                int missing = 0;
                for (int v = bottom; v <= top; ++v) missing += !black[u][v];
                consider(true, u, missing, cost[u-left][h], cost[right-u][h]);
            }
            for (int v = bottom; v <= top; ++v) {
                int missing = 0;
                for (int u = left; u <= right; ++u) missing += !black[u][v];
                consider(false, v, missing, cost[w][v-bottom], cost[w][top-v]);
            }
        }
    }
}

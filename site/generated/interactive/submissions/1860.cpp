#include <bits/stdc++.h>
using namespace std;

struct Rect {
    int r0, r1, c0, c1;
    int height() const { return r1 - r0 + 1; }
    int width() const { return c1 - c0 + 1; }
    bool contains(int r, int c) const {
        return r0 <= r && r <= r1 && c0 <= c && c <= c1;
    }
};

constexpr int DR[] = {-1, 0, 1, 0};
constexpr int DC[] = {0, -1, 0, 1};
constexpr char DIR[] = "^<v>";
constexpr int INCOMING = 8;

struct Game {
    int m, stride, center, r, c, pos;
    int delta[4];
    vector<uint16_t> cell;

    explicit Game(int n)
        : m(2*n+1), stride(m+2), center(n+1), r(center), c(center),
          pos(r*stride+c), cell(stride*stride, 0) {
        for (int d = 0; d < 4; ++d)
            delta[d] = DR[d]*stride + DC[d];
        for (int i = 0; i <= m+1; ++i) {
            cell[id(0,i)] = cell[id(m+1,i)] = 5;
            cell[id(i,0)] = cell[id(i,m+1)] = 5;
        }
        // No edge of the path can enter its starting cell.
        cell[pos] |= INCOMING;
        setArrow(pos, 0);
    }

    int id(int y, int x) const { return y*stride+x; }
    int code(int p) const { return cell[p] & 7; }
    static int ban(int d) { return 16 << d; }

    void setArrow(int p, int d) {
        cell[p] = (cell[p] & ~7) | (d+1);
        cell[p+delta[d]] |= INCOMING;
    }

    void step(int d) {
        cout << DIR[d] << '\n' << flush;
        char ch;
        if (!(cin >> ch) || ch == 'G') exit(0);
        r += DR[d];
        c += DC[d];
        pos += delta[d];
        if (ch == '.') {
            cell[pos] = (cell[pos] & ~7) | 5;
        } else {
            int e = 0;
            while (e < 4 && DIR[e] != ch) ++e;
            if (e == 4) exit(0);
            setArrow(pos, e);
        }
    }

    void walk(int y, int x) {
        while (r != y) step(r < y ? 2 : 0);
        while (c != x) step(c < x ? 3 : 1);
    }

    // Bounds on the signed crossing of the edge directed p -> q.
    pair<int,int> edge(int p, int d) const {
        int q = p+delta[d], e = d^2;
        int a = code(p), b = code(q);
        if (a == d+1) return {1,1};
        if (b == e+1) return {-1,-1};
        if (a == 5 || b == 5) return {0,0};
        int hi = !a && !(cell[p] & ban(d)) && !(cell[q] & INCOMING);
        int lo = !b && !(cell[q] & ban(e)) && !(cell[p] & INCOMING);
        return {-lo,hi};
    }

    bool possibleGoal(int p) const {
        if (code(p)) return false;
        if (cell[p] & INCOMING) return true;
        for (int d = 0; d < 4; ++d) {
            int q = p+delta[d], e = d^2;
            if (!code(q) && !(cell[q] & ban(e))) return true;
        }
        return false;
    }

    int outerNet(const Rect& a, int skip) const {
        int net = 0;
        for (int x = a.c0; x <= a.c1; ++x) {
            if (skip != 0) net += edge(id(a.r0,x),0).first;
            if (skip != 2) net += edge(id(a.r1,x),2).first;
        }
        for (int y = a.r0; y <= a.r1; ++y) {
            if (skip != 1) net += edge(id(y,a.c0),1).first;
            if (skip != 3) net += edge(id(y,a.c1),3).first;
        }
        return net;
    }
};

// A segment consists of edges between coordinates k and k+1.
// axis=0 means the transverse coordinate is a row; axis=1 a column.
struct Segment {
    int axis, k, a, b, sign;
    int uncertain = 0;
    vector<pair<int,int>> ranges;

    int trans(const Game& g) const { return axis ? g.c : g.r; }
    int along(const Game& g) const { return axis ? g.r : g.c; }
    int normal() const { return axis ? 3 : 2; }
    int parallel() const { return axis ? 2 : 3; }
    int lowerCell(const Game& g, int t) const {
        return axis ? g.id(t,k) : g.id(k,t);
    }
    pair<int,int> bounds(const Game& g, int t) const {
        auto [lo,hi] = g.edge(lowerCell(g,t),normal());
        return sign > 0 ? make_pair(lo,hi) : make_pair(-hi,-lo);
    }
    bool unresolved(int t) const {
        auto [lo,hi] = ranges[t-a];
        return lo != hi;
    }
};

class Scanner {
    Game& g;
    vector<Segment> segs;
    int base, sumLo = 0, sumHi = 0;

    bool possible(int s) const { return sumLo <= s && s <= sumHi; }
    bool ambiguous() const { return possible(base) && possible(base-1); }

    void step(int d) {
        g.step(d);
        for (auto& s : segs) {
            int z = s.trans(g);
            if (z < s.k-1 || z > s.k+2) continue;
            int t = s.along(g);
            for (int j = max(s.a,t-1); j <= min(s.b,t+1); ++j) {
                auto& old = s.ranges[j-s.a];
                if (old.first == old.second) continue;
                auto now = s.bounds(g,j);
                sumLo += now.first-old.first;
                sumHi += now.second-old.second;
                if (now.first == now.second) --s.uncertain;
                old = now;
            }
        }
    }

public:
    Scanner(Game& game, int b) : g(game), base(b) {}

    void add(int axis, int k, int a, int b, int sign = 1) {
        // Edges to the padding have zero flow.
        if (k < 1 || k >= g.m || a > b) return;
        Segment s{axis,k,a,b,sign,0,{}};
        s.ranges.reserve(b-a+1);
        for (int t = a; t <= b; ++t) {
            auto p = s.bounds(g,t);
            s.ranges.push_back(p);
            sumLo += p.first;
            sumHi += p.second;
            s.uncertain += (p.first != p.second);
        }
        segs.push_back(move(s));
    }

    bool resolve() {
        // The net flow is base if the goal is outside, base-1 if inside.
        while (ambiguous()) {
            int best = INT_MAX, which = -1, first = 0, last = 0;
            for (int i = 0; i < (int)segs.size(); ++i) {
                auto& s = segs[i];
                if (!s.uncertain) continue;
                int l = s.a, h = s.b;
                while (!s.unresolved(l)) ++l;
                while (!s.unresolved(h)) --h;
                int p = s.along(g), z = s.trans(g);
                if (abs(p-h) < abs(p-l)) swap(l,h);
                int cost = abs(p-l) + max({0,s.k-z,z-s.k-1});
                if (cost < best) {
                    best = cost;
                    which = i;
                    first = l;
                    last = h;
                }
            }
            if (which < 0) exit(0); // Cannot happen for a valid field.
            auto& s = segs[which];
            int z = clamp(s.trans(g),s.k,s.k+1);
            while (ambiguous() && s.uncertain && s.trans(g) != z)
                step(s.normal() ^ (s.trans(g) > z ? 2 : 0));

            int inc = first <= last ? 1 : -1;
            for (int t = first; ambiguous() && s.uncertain; t += inc) {
                if (s.unresolved(t)) {
                    while (ambiguous() && s.uncertain && s.along(g) != t)
                        step(s.parallel() ^ (s.along(g) > t ? 2 : 0));
                    if (ambiguous() && s.unresolved(t)) {
                        // Our cell on this edge has already been read.
                        step(s.normal() ^ (s.trans(g) == s.k ? 0 : 2));
                    }
                }
                if (t == last) break;
            }
        }

        bool inside = possible(base-1);
        int exact = base-int(inside);
        // If just one of two consecutive integers is possible, it is
        // an endpoint of the bounds. Each edge must attain its own bound.
        bool takeHigh = exact == sumHi;
        for (const auto& s : segs) {
            for (int t = s.a; t <= s.b; ++t) {
                auto [lo,hi] = s.ranges[t-s.a];
                if (lo == hi) continue;
                int p = s.lowerCell(g,t), d = s.normal(), q = p+g.delta[d];
                int v = s.sign * (takeHigh ? hi : lo);
                if (v > 0) g.setArrow(p,d);
                else if (v < 0) g.setArrow(q,d^2);
                else {
                    g.cell[p] |= Game::ban(d);
                    g.cell[q] |= Game::ban(d^2);
                }
            }
        }
        return inside;
    }
};

struct Cut {
    int axis = 0, k = 0, base = 0;
    double score = 1e100;
};

Cut chooseCut(const Game& g, const Rect& a) {
    Cut best;
    for (int axis = 0; axis < 2; ++axis) {
        int start = axis ? a.c0 : a.r0;
        int len = axis ? a.width() : a.height();
        if (len < 2) continue;
        int p0 = axis ? a.r0 : a.c0, p1 = axis ? a.r1 : a.c1;
        int z = axis ? g.c : g.r, p = axis ? g.r : g.c;
        int minSide = max(1,len/4);
        vector<int> sizes;
        auto put = [&](int v) {
            sizes.push_back(clamp(v,minSide,len-minSide));
        };
        for (int j = 4; j <= 12; ++j) put((len*j+8)/16);
        for (int j = -2; j <= 2; ++j) {
            put(len/2+j);
            put(z-start+1+j);
        }
        sort(sizes.begin(),sizes.end());
        sizes.erase(unique(sizes.begin(),sizes.end()),sizes.end());

        for (int left : sizes) {
            int k = start+left-1;
            Rect low = a;
            if (axis) low.c1 = k;
            else low.r1 = k;
            Segment s{axis,k,p0,p1,1,0,{}};
            int base = int(low.contains(g.center,g.center))-g.outerNet(low,s.normal());
            int lo = 0, hi = 0, first = p1+1, last = p0-1, both = 0, one = 0;
            for (int t = p0; t <= p1; ++t) {
                auto [l,h] = s.bounds(g,t);
                lo += l;
                hi += h;
                if (l != h) {
                    first = min(first,t);
                    last = t;
                    if (l < 0 && h > 0) ++both;
                    else ++one;
                }
            }
            double cost = 0;
            if (lo <= base-1 && base <= hi) {
                cost = max({0,k-z,z-k-1}) + min(abs(p-first),abs(p-last))
                     + (last-first) + 0.34*both + 0.48*one;
            }
            int right = len-left;
            double gain = 0.70*min(left,right) + 0.30*(2.0*left*right/len);
            double score = cost - 3.35*gain;
            if (score < best.score) best = {axis,k,base,score};
        }
    }
    return best;
}

// Each portion of the path inside a rectangle starts at the original
// source or at an incoming boundary edge. Try a nearby unread end of
// such a portion before paying for another separator.
void tryNearbyTrail(Game& g, const Rect& a) {
    int budget = clamp((a.height()+a.width())/42,3,120);
    int best = budget/3+1, br = -1, bc = -1;
    auto consider = [&](int y, int x) {
        while (a.contains(y,x)) {
            int value = g.code(g.id(y,x));
            if (value == 0) {
                int dist = abs(g.r-y)+abs(g.c-x);
                if (dist < best) {
                    best = dist;
                    br = y;
                    bc = x;
                }
                return;
            }
            if (value == 5) return;
            int d = value-1;
            y += DR[d];
            x += DC[d];
        }
    };
    if (a.contains(g.center,g.center)) consider(g.center,g.center);
    for (int x = a.c0; x <= a.c1; ++x) {
        if (g.code(g.id(a.r0-1,x)) == 3) consider(a.r0,x);
        if (g.code(g.id(a.r1+1,x)) == 1) consider(a.r1,x);
    }
    for (int y = a.r0; y <= a.r1; ++y) {
        if (g.code(g.id(y,a.c0-1)) == 4) consider(y,a.c0);
        if (g.code(g.id(y,a.c1+1)) == 2) consider(y,a.c1);
    }
    if (br < 0) return;
    g.walk(br,bc);
    for (int left = budget-best; left > 0; --left) {
        int d = g.code(g.pos)-1;
        if (d < 0 || d >= 4 || !a.contains(g.r+DR[d],g.c+DC[d])) break;
        g.step(d);
    }
}

bool finishSmall(Game& g, const Rect& a) {
    int area = a.height()*a.width();
    if (area > 100) return false;
    bool committed = false;
    for (;;) {
        int count = 0, best = INT_MAX, by = -1, bx = -1;
        for (int y = a.r0; y <= a.r1; ++y) {
            for (int x = a.c0; x <= a.c1; ++x) {
                int p = g.id(y,x);
                if (!g.possibleGoal(p)) continue;
                ++count;
                int score = 2*(abs(g.r-y)+abs(g.c-x)) - bool(g.cell[p]&INCOMING);
                if (score < best) {
                    best = score;
                    by = y;
                    bx = x;
                }
            }
        }
        if (!committed && area > 12 && count > 3) return false;
        if (!count) exit(0); // A valid field always has a remaining candidate.
        committed = true;
        g.walk(by,bx);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1 || n > 2000) return 0;
    Game g(n);
    Rect area{1,g.m,1,g.m};
    Rect trail{g.r,g.r,g.c,g.c};

    int used = 0;
    int soft = max(5,n);
    int hard = max(soft,2*n+min(80,n/8));
    while (used < hard) {
        g.step(g.code(g.pos)-1);
        ++used;
        trail.r0 = min(trail.r0,g.r);
        trail.r1 = max(trail.r1,g.r);
        trail.c0 = min(trail.c0,g.c);
        trail.c1 = max(trail.c1,g.c);
        int displacement = abs(g.r-g.center)+abs(g.c-g.center);
        if (used >= soft && 3*displacement < 2*used) break;
    }

    // If the observed trail is very compact, testing its neighborhood
    // can be much cheaper than scanning a full-width separator.
    if (n >= 60 && trail.height()*trail.width() <= 14*used) {
        int pad = max(2,min(n/24,(trail.height()+trail.width())/6));
        Rect box{max(1,trail.r0-pad),min(g.m,trail.r1+pad),
                 max(1,trail.c0-pad),min(g.m,trail.c1+pad)};
        if (5*(box.height()+box.width()) <= 2*n) {
            Scanner scan(g,1);
            scan.add(0,box.r0-1,box.c0,box.c1,-1);
            scan.add(0,box.r1,box.c0,box.c1);
            scan.add(1,box.c0-1,box.r0,box.r1,-1);
            scan.add(1,box.c1,box.r0,box.r1);
            if (scan.resolve()) area = box;
        }
    }

    while (!finishSmall(g,area)) {
        Cut cut = chooseCut(g,area);
        Scanner scan(g,cut.base);
        if (cut.axis) scan.add(1,cut.k,area.r0,area.r1);
        else scan.add(0,cut.k,area.c0,area.c1);
        bool low = scan.resolve();
        if (cut.axis) {
            if (low) area.c1 = cut.k;
            else area.c0 = cut.k+1;
        } else {
            if (low) area.r1 = cut.k;
            else area.r0 = cut.k+1;
        }
        if (area.height()*area.width() > 12) tryNearbyTrail(g,area);
    }
    return 0;
}

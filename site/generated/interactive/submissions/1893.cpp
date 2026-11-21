#include <bits/stdc++.h>
using namespace std;

int n, k;
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

void moveWheel(int i, int d) {
    if (d == 0) return;
    cout << i << ' ' << d << endl;
    if (!(cin >> k) || k <= 1 || k > n) exit(0);
}

struct Hyp {
    int x, single;
    double w;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n >> k) || k <= 1 || k > n) return 0;

    vector<int> id(n), order(n);
    iota(id.begin(), id.end(), 1);
    iota(order.begin(), order.end(), 0);
    shuffle(id.begin(), id.end(), rng);
    shuffle(order.begin() + 1, order.end(), rng);

    // Avoid a nearly featureless occupancy pattern on a large, dense board.
    if (n >= 16 && 100*k > 89*n) {
        for (int t = 0; t < n && 100*k > 79*n; ++t)
            moveWheel(id[t], uniform_int_distribution<int>(1, n-1)(rng));
    }

    // Coordinates are relative to the scanning wheel's starting symbol.
    vector<int> obs(n, -1), base(n);
    int a = 0, initial = k;
    obs[0] = k;
    for (int t = 1;; ++t) {
        int options = 0;
        bool determined = false;
        for (int b : {initial-1, initial}) {
            if (b < 1 || b >= n) continue;
            int yes = 0, no = 0;
            bool ok = true;
            for (int v : obs) {
                if (v == b) ++yes;
                else if (v == b+1) ++no;
                else if (v != -1) ok = false;
            }
            if (!ok || yes > b || no > n-b) continue;
            ++options;
            determined = yes == b || no == n-b;
            if (determined)
                for (int z = 0; z < n; ++z)
                    base[z] = obs[z] == b || (obs[z] == -1 && no == n-b);
        }
        if (options == 1 && determined) break;
        int z = order[t];
        moveWheel(id[0], z-a);
        a = z;
        obs[z] = k;
    }

    int target = base[0] ? 0 : (base[a] ? a : int(find(base.begin(), base.end(), 1)-base.begin()));
    vector<int> lo = base, hi(n), known(n);
    for (int z = 0; z < n; ++z) hi[z] = base[z] ? n-1 : 0;
    known[a] = 1;

    for (int i = 1; i < n;) {
        int rem = n-i;
        int sl = accumulate(lo.begin(), lo.end(), 0);
        int su = accumulate(hi.begin(), hi.end(), 0);
        for (int z = 0; z < n; ++z) {
            int l = lo[z], h = hi[z];
            lo[z] = max(l, rem - (su-h));
            hi[z] = min(h, rem - (sl-l));
        }

        vector<int> S(n);
        for (int z = 0; z < n; ++z) S[z] = known[z] || lo[z];
        // An unmarked site is either certainly occupied or certainly empty.
        // If the last known wheel leaves a site, its move reveals which.
        bool ambiguousPeriod = false;
        for (int d = 1; d < n && !ambiguousPeriod; ++d) {
            bool period = true, overlap = false;
            for (int z = 0; z < n && period; ++z) {
                int y = (z+d)%n;
                period = S[z] == S[y];
                overlap |= hi[z] && hi[y];
            }
            ambiguousPeriod = period && overlap;
        }

        if (ambiguousPeriod) {
            // Toggle one bit of the periodic pattern using the scanner.
            bool dup = known[a] > 1 || lo[a] > 0;
            int z = -1;
            for (int y : order)
                if (y != a && S[y] == !dup && (z == -1 || y == target)) z = y;
            int old = k;
            moveWheel(id[0], z-a);
            bool single = old + (1-S[z]) - k;
            if (--known[a] == 0) {
                if (single) lo[a] = hi[a] = 0;
                else lo[a] = max(lo[a], 1);
            }
            ++known[z];
            a = z;
            continue;
        }

        int extra = rem - accumulate(lo.begin(), lo.end(), 0);
        int freeSites = 0;
        for (int z = 0; z < n; ++z) freeSites += hi[z] > lo[z];
        vector<Hyp> H;
        for (int z = 0; z < n; ++z) if (hi[z]) {
            double avg = min<double>(hi[z], lo[z] + (hi[z] > lo[z] ?
                               double(extra)/max(1, freeSites) : 0.0));
            avg = max(avg, 0.01);
            bool canSingle = !known[z] && lo[z] <= 1;
            bool canDup = known[z] || hi[z] >= 2;
            double p = !canSingle ? 0 : !canDup ? 1 :
                       clamp(exp(-max(0.0, avg-1))/max(1.0,avg), 0.03, 0.97);
            if (canSingle) H.push_back({z,1,avg*p});
            if (canDup) H.push_back({z,0,avg*(1-p)});
        }

        int start = k, off = 0;
        auto outcome = [&](const Hyp &h, int d) {
            return d ? 1-h.single-S[(h.x+d)%n] : 0;
        };
        auto test = [&](int d) {
            if (d == off) return;
            moveWheel(id[i], d-off);
            off = d;
            H.erase(remove_if(H.begin(), H.end(), [&](const Hyp &h) {
                return start + outcome(h,d) != k;
            }), H.end());
        };

        while (H.front().x != H.back().x) {
            double best = -1;
            int choice = -1;
            for (int d = 1; d < n; ++d) {
                double mass[3]{}, total = 0;
                int who[3] = {-1,-1,-1};
                for (auto &h : H) {
                    int b = outcome(h,d)+1;
                    mass[b] += h.w;
                    total += h.w;
                    if (who[b] == -1) who[b] = h.x;
                    else if (who[b] != h.x) who[b] = -2;
                }
                if ((mass[0]>0)+(mass[1]>0)+(mass[2]>0) < 2) continue;
                double score = 0;
                for (int b = 0; b < 3; ++b) if (mass[b]) {
                    double p = mass[b]/total;
                    score -= p*log2(p);
                    if (who[b] >= 0 && (who[b]+d)%n == target) score += p;
                }
                if (score > best) best = score, choice = d;
            }
            test(choice);
        }
        int x = H[0].x;
        test((target-x+n)%n);
        if (H.size() == 1 && !known[x]) {
            if (H[0].single) lo[x] = hi[x] = 1;
            else lo[x] = max(lo[x], 2);
        }
        lo[x] = max(0, lo[x]-1);
        --hi[x];
        ++known[target];
        ++i;
    }
    moveWheel(id[0], target-a);
    return 0;
}

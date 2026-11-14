#include <bits/stdc++.h>
using namespace std;

struct Solver {
    int n, k, actions = 0;
    function<int(int,int)> interact;
    vector<int> occupied, label;
    mt19937 rng;
    Solver(int n_, int k_, function<int(int,int)> f, unsigned seed = 712367821)
        : n(n_), k(k_), interact(move(f)), rng(seed) {
        label.resize(n);
        iota(label.begin(), label.end(), 0);
        shuffle(label.begin(), label.end(), rng);
    }
    int norm(int x) { x %= n; return x < 0 ? x+n : x; }
    void act(int i, int d) {
        d = norm(d);
        if (!d || k == 1) return;
        ++actions;
        k = interact(label[i], d);
    }
    bool periodic() {
        for (int d=1; d<n; ++d) {
            bool same=true;
            for (int x=0; x<n; ++x)
                if (occupied[x] != occupied[(x+d)%n]) { same=false; break; }
            if (same) return true;
        }
        return false;
    }
    void rebase(int d) {
        vector<int> next(n);
        for (int x=0; x<n; ++x) next[norm(x-d)] = occupied[x];
        occupied.swap(next);
    }
    void run() {
        // Sweep wheel zero; the minimum response is the number of symbols
        // on all the other wheels. A minimum at d means d is occupied.
        vector<int> response(n);
        response[0]=k;
        for (int d=1; d<n; ++d) {
            act(0,1);
            if (k==1) return;
            response[d]=k;
        }
        int low=*min_element(response.begin(),response.end());
        occupied.resize(n);
        for (int d=0; d<n; ++d) occupied[d]=(response[d]==low);
        occupied[n-1]=1;
        rebase(n-1);

        for (int wheel=1; wheel<n && k>1; ++wheel) {
            int marker=-1;
            // A periodic occupancy mask cannot distinguish shifts by its
            // period. Place a known wheel at an empty symbol to break it.
            if (periodic()) {
                int empty=-1;
                for (int x=0; x<n; ++x) if (!occupied[x]) { empty=x; break; }
                if (empty==-1) {
                    // Every symbol is occupied, so every wheel is unique.
                    act(0,1);
                    if (k==1) return;
                    occupied[0]=0;
                    rebase(1);
                } else if (wheel==1) {
                    int before=k;
                    act(0,empty);
                    if (k==1) return;
                    if (k==before) occupied[0]=0;
                    occupied[empty]=1;
                    rebase(empty);
                } else {
                    marker=empty;
                    act(0,marker);
                    if (k==1) return;
                    occupied[marker]=1;
                }
            }
            int baseline=k, remaining=n-wheel;
            int free_symbols=0;
            for (int x=0; x<n; ++x)
                if (occupied[x] && x!=0 && x!=marker) ++free_symbols;
            // (original position, whether it is the only wheel there).
            vector<pair<int,int>> candidates;
            for (int x=0; x<n; ++x) if (occupied[x]) {
                bool fixed=(x==0 || x==marker);
                if (!fixed) candidates.push_back({x,1});
                if (remaining>free_symbols) candidates.push_back({x,0});
            }
            int displacement=0;
            while (true) {
                bool unique=true;
                for (auto [x,s]:candidates)
                    if (x!=candidates[0].first) unique=false;
                if (unique) break;
                int best=-1, bestmax=INT_MAX, bestsquare=INT_MAX;
                int ties=0;
                for (int d=1; d<n; ++d) {
                    int bucket[3]={};
                    for (auto [x,s]:candidates) {
                        int delta=1-occupied[(x+d)%n]-s;
                        ++bucket[delta+1];
                    }
                    int mx=max({bucket[0],bucket[1],bucket[2]});
                    int square=0;
                    for (int b:bucket) square+=b*b;
                    if (mx<bestmax || (mx==bestmax && square<bestsquare)) {
                        best=d; bestmax=mx; bestsquare=square; ties=1;
                    } else if (mx==bestmax && square==bestsquare) {
                        if (rng()%++ties==0) best=d;
                    }
                }
                act(wheel,best-displacement);
                if (k==1) return;
                displacement=best;
                vector<pair<int,int>> next;
                for (auto [x,s]:candidates)
                    if (baseline+1-occupied[(x+best)%n]-s==k)
                        next.push_back({x,s});
                candidates.swap(next);
            }
            int x=candidates[0].first;
            act(wheel,-x-displacement);
            if (k==1) return;
            if (x!=0 && k==baseline-1) occupied[x]=0;
            if (marker!=-1) {
                act(0,-marker);
                if (k==1) return;
                occupied[marker]=0;
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n,k;
    if (!(cin>>n>>k) || k<=1) return 0;
    unsigned seed=chrono::steady_clock::now().time_since_epoch().count();
    Solver solver(n,k,[&](int i,int d) {
        cout << i+1 << ' ' << d << endl;
        int answer;
        if (!(cin>>answer) || answer<=1) exit(0);
        return answer;
    },seed);
    solver.run();
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

int ask(const vector<int>& q) {
    cout << "? " << q.size();
    for (int i : q) cout << ' ' << i;
    cout << endl;
    int r;
    if (!(cin >> r) || (r != 0 && r != 1)) exit(0);
    return r;
}

struct Base {
    vector<int> ids;
    bool complement;
};

struct Solver {
    int n, total, known = 0;
    vector<int> p, pos, order;
    vector<vector<int>> dom;
    vector<Base> bases;
    vector<vector<int>> id, omit;
    vector<int> mods;

    void fix(int i, int v) {
        p[i] = v;
        pos[v] = i;
        dom[i] = {v};
        ++known;
    }

    void clean(int i) {
        auto& d = dom[i];
        d.erase(remove_if(d.begin(), d.end(), [&](int v){ return pos[v] != 0; }), d.end());
    }

    // A template consists entirely of known positions. We may omit one
    // of them, add the tested position, and optionally take the complement.
    void add(const vector<int>& b, bool comp, bool drop) {
        int c = (int)b.size() - int(drop);
        int k = comp ? n-c-1 : c+1;
        if (k < 2 || k >= n) return;
        if (id[k].empty()) {
            id[k].assign(k,-1);
            omit[k].assign(k,0);
            mods.push_back(k);
        }
        int sum = 0;
        for (int j : b) sum += p[j];
        int bid = -1;
        auto record = [&](int j) {
            int r = ((comp ? total : 0) - sum + (j ? p[j] : 0)) % k;
            if (r < 0) r += k;
            if (id[k][r] != -1) return;
            if (bid < 0) {
                bid = (int)bases.size();
                bases.push_back({b, comp});
            }
            id[k][r] = bid;
            omit[k][r] = j;
        };
        if (drop) for (int j : b) record(j);
        else record(0);
    }

    void build() {
        vector<int> a;
        for (int i : order) if (p[i]) a.push_back(i);
        int h = a.size();
        bases.clear();
        mods.clear();
        id.assign(n+1,{});
        omit.assign(n+1,{});
        if (h <= 11) {
            for (int m=0; m<(1<<h); ++m) {
                vector<int> b;
                for (int j=0;j<h;++j) if (m>>j&1) b.push_back(a[j]);
                add(b,false,false);
                add(b,true,false);
            }
        } else {
            for (int k=2;k<=h;++k) {
                for (int t=0;t<10;++t) {
                    shuffle(a.begin(),a.end(),rng);
                    vector<int> b;
                    if (t==0) {
                        vector<bool> used(k), took(n+1);
                        for (int j:a) if (!used[p[j]%k]) {
                            used[p[j]%k]=true;
                            took[j]=true;
                            b.push_back(j);
                        }
                        for (int j:a) {
                            if ((int)b.size()==k) break;
                            if (!took[j]) b.push_back(j);
                        }
                    } else b.assign(a.begin(),a.begin()+k);
                    add(b,false,true);
                    if (find(id[k].begin(),id[k].end(),-1)==id[k].end() || k==h) break;
                }
            }
            add(a,false,false);
        }
        sort(mods.begin(),mods.end());
    }

    void process(int i) {
        clean(i);
        auto& d=dom[i];
        while (d.size()>1) {
            int m=d.size(), best=0, bk=0, br=0;
            int hist[801];
            for (int k:mods) {
                fill(hist,hist+k,0);
                for (int v:d) ++hist[v%k];
                for (int r=0;r<k;++r) if (id[k][r]>=0) {
                    int s=min(hist[r],m-hist[r]);
                    if (s>best) best=s,bk=k,br=r;
                }
                if (best>=m/2-max(0,m/80) && best) break;
            }
            // Postpone expensive splits until more reference values are known.
            if (!best || (m>7 && 5*best<m)) return;
            const Base& b=bases[id[bk][br]];
            vector<int> q;
            if (!b.complement) {
                for (int j:b.ids) if (j!=omit[bk][br]) q.push_back(j);
                q.push_back(i);
            } else {
                vector<bool> skip(n+1);
                for (int j:b.ids) if (j!=omit[bk][br]) skip[j]=true;
                skip[i]=true;
                for (int j=1;j<=n;++j) if (!skip[j]) q.push_back(j);
            }
            int yes=ask(q);
            d.erase(remove_if(d.begin(),d.end(),[&](int v){
                return (v%bk==br)!=yes;
            }),d.end());
        }
        fix(i,d[0]);
    }

    bool forced() {
        vector<int> cnt(n+1), last(n+1);
        bool changed=false;
        for (int i:order) if (!p[i]) {
            clean(i);
            if (dom[i].size()==1) {
                fix(i,dom[i][0]);
                changed=true;
            } else for (int v:dom[i]) ++cnt[v],last[v]=i;
        }
        for (int v=1;v<=n;++v) if (!pos[v] && cnt[v]==1 && !p[last[v]]) {
            fix(last[v],v);
            changed=true;
        }
        return changed;
    }

    // The unknown extremes of the first unfinished symmetric interval
    // can always be found, independently of template availability.
    void acquire() {
        int l=1,r=n;
        while (pos[l] && pos[r]) ++l,--r;
        while (true) {
            int best=0, num=0, den=1;
            for (int i:order) if (!p[i]) {
                auto& d=dom[i];
                int t=binary_search(d.begin(),d.end(),l)+binary_search(d.begin(),d.end(),r);
                if (t*den>num*(int)d.size()) best=i,num=t,den=d.size();
            }
            int hit=1;
            if (num!=den) {
                vector<bool> skip(n+1);
                for (int v=1;v<l;++v) skip[pos[v]]=true;
                for (int v=r+1;v<=n;++v) skip[pos[v]]=true;
                skip[best]=true;
                vector<int> q;
                for (int i=1;i<=n;++i) if (!skip[i]) q.push_back(i);
                hit=ask(q);
            }
            auto& d=dom[best];
            d.erase(remove_if(d.begin(),d.end(),[&](int v){return (v==l || v==r)!=hit;}),d.end());
            if (d.size()==1) {
                fix(best,d[0]);
                return;
            }
            if (hit) {
                // The endpoints have opposite parity.
                int j=pos[1];
                int evenWithOne=ask({best,j});
                fix(best,((l%2)==evenWithOne)?l:r);
                return;
            }
            if (forced()) return;
        }
    }

    void run() {
        if (!(cin>>n) || n<2) return;
        if (n==2) { cout<<"! 1 2"<<endl; return; }
        total=n*(n+1)/2;
        order.resize(n);
        iota(order.begin(),order.end(),1);
        shuffle(order.begin(),order.end(),rng);
        p.resize(n+1);
        pos.resize(n+1);
        dom.resize(n+1);
        for (int i:order) {
            dom[i].resize(n);
            iota(dom[i].begin(),dom[i].end(),1);
        }
        // Choose our working orientation by naming the first endpoint 1.
        for (int z=0;;++z) {
            int i=order[z];
            if (z==n-2) {
                fix(i,1);
                fix(order[z+1],n);
                break;
            }
            vector<int> q;
            for (int j=1;j<=n;++j) if (j!=i) q.push_back(j);
            if (ask(q)) { fix(i,1); break; }
            dom[i].pop_back();
            dom[i].erase(dom[i].begin());
        }
        while (known<n) {
            while (forced()) {}
            if (known==n) break;
            int start=known;
            build();
            vector<int> todo;
            for (int i:order) if (!p[i]) todo.push_back(i);
            stable_sort(todo.begin(),todo.end(),[&](int a,int b){return dom[a].size()<dom[b].size();});
            for (int i:todo) {
                process(i);
                if (known-start>=max(1,start/3)) break;
            }
            if (known==start && !forced()) acquire();
        }
        bool rev=p[1]>n/2;
        cout<<"!";
        for (int i=1;i<=n;++i) cout<<' '<<(rev?n+1-p[i]:p[i]);
        cout<<endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver s;
    s.run();
}

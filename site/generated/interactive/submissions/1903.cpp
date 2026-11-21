#include <bits/stdc++.h>
using namespace std;

struct Solver {
    int n, m, s, pos, start;
    int d[4];
    const string dirs = "^>v<";
    // Padded board: -2 = empty (also used outside), -1 = unseen.
    vector<signed char> a;

    int dist(int u, int v) const {
        return abs(u / s - v / s) + abs(u % s - v % s);
    }

    void step(int k) {
        cout << dirs[k] << '\n' << flush;
        char c;
        if (!(cin >> c) || c == 'G') exit(0);
        pos += d[k];
        if (c == '.') a[pos] = -2;
        else {
            auto i = dirs.find(c);
            if (i == string::npos) exit(0);
            a[pos] = (int)i;
        }
    }

    void go(int t) {
        while (pos != t) {
            int v = pos / s == t / s ? -1 : (t > pos ? 2 : 0);
            int h = pos % s == t % s ? -1 : (t % s > pos % s ? 1 : 3);
            if (v < 0) step(h);
            else if (h < 0) step(v);
            else if (a[pos] == h || (a[pos] != v && a[pos+d[h]] == -1 &&
                                       a[pos+d[v]] != -1)) step(h);
            else step(v);
        }
    }

    // Does the unseen neighbor u point to the observed cell v?
    // k is the direction v -> u. 2 means undetermined.
    int incoming(int v, int k) const {
        if (a[v] < 0 || v == start || a[v] == k) return 0;
        bool other = false;
        for (int j = 0; j < 4; ++j) if (j != k && j != a[v]) {
            int z = a[v+d[j]];
            if (z == (j^2)) return 0;
            if (z == -1) other = true;
        }
        return other ? 2 : 1;
    }

    // Net path flow from u to its neighbor v, or 2 if not yet known.
    int flux(int u, int k) const {
        int v = u + d[k];
        if (a[u] == -1 && a[v] == -1) return 2;
        int f = (a[u] == k), g = (a[v] == (k^2));
        if (a[u] == -1) f = incoming(v,k^2);
        if (a[v] == -1) g = incoming(u,k);
        return f == 2 || g == 2 ? 2 : f-g;
    }

    void run() {
        if (!(cin >> n) || n < 1) return;
        m = 2*n+1;
        s = m+2;
        d[0]=-s; d[1]=1; d[2]=s; d[3]=-1;
        a.assign(s*s,-2);
        for (int y=1; y<=m; ++y)
            fill_n(a.begin()+y*s+1,m,-1);
        pos = start = (n+1)*s+n+1;
        a[pos]=0;

        int l=1,r=m,t=1,b=m;
        for (;;) {
            int w=r-l+1,h=b-t+1;
            auto inside = [&](int p) {
                return p%s>=l && p%s<=r && p/s>=t && p/s<=b;
            };

            // Take cheap chances to reach the goal directly, especially
            // when the footprints make steady forward progress.
            int seed=-1, best=100;
            for (int dy=-2;dy<=2;++dy) for(int dx=-2;dx<=2;++dx) {
                int cost=abs(dx)+abs(dy);
                if(cost>2 || pos%s+dx<1 || pos%s+dx>m ||
                   pos/s+dy<1 || pos/s+dy>m) continue;
                int p=pos+dy*s+dx;
                if(a[p]<0) continue;
                int q=p+d[(int)a[p]];
                if(!inside(q) || a[q]!=-1) continue;
                if(cost<best) seed=p,best=cost;
            }
            if(seed!=-1) {
                int z=seed+d[(int)a[seed]];
                go(z);
                int base=3+min(w,h)/5;
                for(int i=0; a[pos]>=0 && i<base+2*dist(pos,z) &&
                                    i<base+2*max(w,h); ++i) {
                    int k=a[pos];
                    if(!inside(pos+d[k])) break;
                    step(k);
                }
            }

            if (w==1 || h==1 || w*h<=12) {
                // Only unseen cells can be the goal, and the goal must
                // admit a predecessor.
                for (;;) {
                    int dst=-1, sc=INT_MAX;
                    for(int y=t;y<=b;++y) for(int x=l;x<=r;++x) {
                        int p=y*s+x;
                        if(a[p]!=-1) continue;
                        bool possible=false, marked=false;
                        for(int j=0;j<4;++j) {
                            int z=a[p+d[j]];
                            marked |= z==(j^2);
                            possible |= z==-1;
                        }
                        if(!possible && !marked) continue;
                        int val=4*dist(pos,p)-2*marked;
                        if(val<sc) sc=val,dst=p;
                    }
                    if(dst<0) return;
                    go(dst);
                }
            }

            // Select a balanced separator, accounting for the approach
            // distance and the parts of the separator already resolved.
            struct Cut {int k=0,c=0,p=0,inc=0,len=0; bool rev=false;} cut;
            double score=1e100;
            for(int k : {1,2}) {
                int lo=k==1?l:t, len=k==1?w:h, breadth=k==1?h:w;
                vector<int> cs;
                for(int f=3;f<=7;++f)
                    cs.push_back(lo+clamp((len*f+5)/10,1,len-1)-1);
                int here=k==1?pos%s:pos/s;
                for(int off : {-1,0}) {
                    int c=here+off;
                    if(c>=lo && c<lo+len-1 &&
                       min(c-lo+1,lo+len-1-c)*4>=len) cs.push_back(c);
                }
                sort(cs.begin(),cs.end());
                cs.erase(unique(cs.begin(),cs.end()),cs.end());
                for(int c:cs) {
                    int p=k==1?t*s+c:c*s+l, inc=k==1?s:1;
                    int first=-1,last=-1,cnt=0;
                    for(int i=0;i<breadth;++i) if(flux(p+i*inc,k)==2) {
                        if(first<0) first=i;
                        last=i; ++cnt;
                    }
                    auto approach=[&](int i) {
                        int u=p+i*inc,v=u+d[k];
                        return min(a[u]==-1?dist(pos,u):INT_MAX,
                                   a[v]==-1?dist(pos,v):INT_MAX);
                    };
                    bool rev=false;
                    double work=0;
                    if(cnt) {
                        int left=approach(first), right=approach(last);
                        rev=right<left;
                        work=min(left,right)+last-first+0.1*cnt+1;
                    }
                    double frac=double(c-lo+1)/len;
                    double gain=len*frac*(1-frac);
                    double val=(work+0.01)/gain;
                    if(val<score) {
                        score=val;
                        cut={k,c,p,inc,breadth,rev};
                    }
                }
            }

            auto [k,c,p,inc,len,rev]=cut;
            for(int j=0;j<len;++j) {
                int u=p+(rev?len-1-j:j)*inc,v=u+d[k];
                if(flux(u,k)!=2) continue;
                int dest;
                if(a[u]!=-1) dest=v;
                else if(a[v]!=-1) dest=u;
                else dest=dist(pos,u)<=dist(pos,v)?u:v;
                go(dest);
                if(flux(u,k)==2) go(dest==u?v:u);
            }

            // All boundary edge flows of both children are now known.
            // Divergence summed over a rectangle is [start] - [goal].
            int rr=k==1?c:r, bb=k==2?c:b, sum=0;
            for(int x=l;x<=rr;++x)
                sum+=flux(t*s+x,0)+flux(bb*s+x,2);
            for(int y=t;y<=bb;++y)
                sum+=flux(y*s+l,3)+flux(y*s+rr,1);
            int src=(start%s>=l && start%s<=rr &&
                     start/s>=t && start/s<=bb);
            if(src-sum==1) r=rr,b=bb;
            else if(k==1) l=c+1;
            else t=c+1;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver solver;
    solver.run();
}

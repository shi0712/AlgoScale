#include <bits/stdc++.h>
using namespace std;

int n;

/*------------------------------------------------------------*/
/*  interaction                                               */
long long ask(int i, int j, int k) {
    cout << "? " << i << ' ' << j << ' ' << k << "\n";
    cout.flush();
    long long x;
    if (!(cin >> x)) exit(0);
    if (x == -1) exit(0);
    return x;
}
void answer(const vector<int> &a) {
    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << a[i];
    cout << "\n";
    cout.flush();
    exit(0);
}
void answer_minus_one() {
    cout << "! -1\n";
    cout.flush();
    exit(0);
}

/*------------------------------------------------------------*/
/*  pre‑computed tables                                        */
long long F[5][5][5];                     // F[x][y][z] = 16·area²
vector<pair<int,int>> goodPairs;          // all good unordered pairs
unordered_map<long long,int> homo;        // answer → value for (v,v,v)

/*------------------------------------------------------------*/
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;

    /* pre‑compute F */
    for (int x = 1; x <= 4; ++x)
        for (int y = 1; y <= 4; ++y)
            for (int z = 1; z <= 4; ++z) {
                long long a = x + y + z;
                long long b = x + y - z;
                long long c = x - y + z;
                long long d = -x + y + z;
                long long val = a * b * c * d;
                if (val < 0) val = 0;
                F[x][y][z] = val;
            }

    /* good pairs */
    for (int x = 1; x <= 4; ++x)
        for (int y = x; y <= 4; ++y) {
            bool ok = true;
            unordered_map<long long,int> mp;
            for (int z = 1; z <= 4; ++z) {
                long long v = F[x][y][z];
                if (mp.count(v)) { ok = false; break; }
                mp[v] = z;
            }
            if (ok) goodPairs.emplace_back(x, y);
        }

    /* homogeneous triples */
    for (int v = 1; v <= 4; ++v) homo[F[v][v][v]] = v;

    /*--------------------------------------------------------*/
    /* small n : brute force                                   */
    if (n <= 7) {
        vector<tuple<int,int,int>> triples;
        for (int i = 1; i <= n; ++i)
            for (int j = i+1; j <= n; ++j)
                for (int k = j+1; k <= n; ++k)
                    triples.emplace_back(i,j,k);
        vector<long long> ans(triples.size());
        for (size_t t = 0; t < triples.size(); ++t) {
            auto [i,j,k] = triples[t];
            ans[t] = ask(i,j,k);
        }
        vector<int> cur(n+1);
        vector<int> best;
        int cnt = 0;
        function<void(int)> dfs = [&](int pos){
            if (pos > n) {
                for (size_t t = 0; t < triples.size(); ++t) {
                    auto [i,j,k] = triples[t];
                    if (F[cur[i]][cur[j]][cur[k]] != ans[t]) return;
                }
                ++cnt;
                if (cnt == 1) best = cur;
                return;
            }
            for (int v = 1; v <= 4; ++v) {
                cur[pos] = v;
                dfs(pos+1);
                if (cnt > 1) return;
            }
        };
        dfs(1);
        if (cnt == 1) answer(best);
        else answer_minus_one();
    }

    /*--------------------------------------------------------*/
    /* large n : main algorithm                                */
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> dist(1, n);

    vector<int> ones;               // three indices known to be 1
    bool haveNonOneTriple = false;
    int i0=0,i1=0,i2=0;
    long long tripleAns = 0;

    const int MAX_ATTEMPTS = 200;
    for (int it = 0; it < MAX_ATTEMPTS && !haveNonOneTriple; ++it) {
        int a = dist(rng);
        int b = dist(rng);
        while (b == a) b = dist(rng);
        int c = dist(rng);
        while (c == a || c == b) c = dist(rng);
        long long v = ask(a,b,c);
        if (v != 0) {
            if (v == 3) {
                if (ones.empty()) ones = {a,b,c};
            } else {
                i0=a; i1=b; i2=c; tripleAns=v;
                haveNonOneTriple = true;
                break;
            }
        }
    }
    if (!haveNonOneTriple) {
        int lim = min(n, 10);
        for (int a = 1; a <= lim && !haveNonOneTriple; ++a)
            for (int b = a+1; b <= lim && !haveNonOneTriple; ++b)
                for (int c = b+1; c <= lim && !haveNonOneTriple; ++c) {
                    long long v = ask(a,b,c);
                    if (v != 0) {
                        if (v == 3) {
                            if (ones.empty()) ones = {a,b,c};
                        } else {
                            i0=a; i1=b; i2=c; tripleAns=v;
                            haveNonOneTriple = true;
                        }
                    }
                }
    }

    /* all answers were 3 → all numbers are 1 */
    if (!haveNonOneTriple && !ones.empty()) {
        vector<int> a(n+1,1);
        answer(a);
    }

    /*--------------------------------------------------------*/
    vector<int> a(n+1, -1);          // final values, -1 = unknown
    int ref1=-1, ref2=-1;            // two reference indices (both ≥2)
    int X=-1, Y=-1;                  // unordered pair of their values

    if (!ones.empty()) {
        // we have three indices known to be 1
        vector<int> ge2;            // indices with value ≥2
        for (int k = 1; k <= n; ++k) {
            if (k==ones[0]||k==ones[1]||k==ones[2]) continue;
            long long v = ask(ones[0], ones[1], k);
            if (v == 0) ge2.push_back(k);
            else a[k] = 1;          // answer 3 → value 1
        }
        if (ge2.empty()) {          // all are 1
            vector<int> ansAll(n+1,1);
            answer(ansAll);
        }
        if ((int)ge2.size() == 1) { // exactly one number ≥2 → ambiguous
            answer_minus_one();
        }
        ref1 = ge2[0];
        ref2 = ge2[1];
    } else {
        // we have a non‑zero, non‑all‑1 triple (i0,i1,i2)
        if (homo.count(tripleAns)) {
            // homogeneous triple of value v≥2
            int v = homo[tripleAns];
            ref1 = i0; ref2 = i1;
            X = Y = v;
        } else {
            // need a good pair among the three indices
            vector<int> tri = {i0,i1,i2};
            bool found = false;
            for (int p = 0; p < 3 && !found; ++p)
                for (int q = p+1; q < 3 && !found; ++q) {
                    int u = tri[p], v = tri[q];
                    // query all other indices, filter candidates
                    vector<pair<int,int>> cand = goodPairs;
                    vector<long long> tmpAns(n+1,-1);
                    bool ok = true;
                    for (int k = 1; k <= n; ++k) {
                        if (k==u||k==v) continue;
                        long long ans = ask(u,v,k);
                        tmpAns[k]=ans;
                        vector<pair<int,int>> nxt;
                        for (auto pr:cand){
                            int x=pr.first, y=pr.second;
                            bool poss=false;
                            for (int z=1;z<=4;++z)
                                if (F[x][y][z]==ans){poss=true;break;}
                            if (poss) nxt.push_back(pr);
                        }
                        cand.swap(nxt);
                        if (cand.empty()){ok=false;break;}
                    }
                    if (ok && cand.size()==1){
                        ref1=u; ref2=v;
                        X=cand[0].first; Y=cand[0].second;
                        // fill known values
                        for (int k=1;k<=n;++k)
                            if (k!=ref1 && k!=ref2){
                                long long ans=tmpAns[k];
                                for (int z=1;z<=4;++z)
                                    if (F[X][Y][z]==ans){a[k]=z;break;}
                            }
                        found=true;
                    }
                }
            if (!found) answer_minus_one(); // should not happen
        }
    }

    /*--------------------------------------------------------*/
    // If X,Y still unknown, determine them now
    if (X==-1 && Y==-1) {
        vector<pair<int,int>> cand = goodPairs;
        vector<long long> ansK(n+1,-1);
        for (int k = 1; k <= n; ++k) {
            if (k==ref1||k==ref2) continue;
            long long ans = ask(ref1, ref2, k);
            ansK[k]=ans;
            vector<pair<int,int>> nxt;
            for (auto pr:cand){
                int x=pr.first, y=pr.second;
                bool poss=false;
                for (int z=1;z<=4;++z)
                    if (F[x][y][z]==ans){poss=true;break;}
                if (poss) nxt.push_back(pr);
            }
            cand.swap(nxt);
            if (cand.empty()) answer_minus_one();
        }
        // now cand may contain >1 element
        if (cand.size()>1){
            // need a value occurring at least twice among known indices
            int freq[5]={0};
            for (int i=1;i<=n;++i) if (i!=ref1 && i!=ref2 && a[i]!=-1) ++freq[a[i]];
            int v = -1;
            for (int val=2;val<=4;++val)
                if (freq[val]>=2){ v=val; break; }
            if (v==-1) answer_minus_one(); // ambiguous
            // pick two indices with that value
            int p=-1,q=-1;
            for (int i=1;i<=n;++i)
                if (i!=ref1 && i!=ref2 && a[i]==v){
                    if (p==-1) p=i;
                    else if (q==-1){ q=i; break; }
                }
            long long ans1 = ask(ref1,p,q);
            long long ans2 = ask(ref2,p,q);
            vector<pair<int,int>> nxt;
            for (auto pr:cand){
                int x=pr.first, y=pr.second;
                bool ok=false;
                // ordering 1: ref1=x , ref2=y
                if (F[x][v][v]==ans1 && F[y][v][v]==ans2) ok=true;
                // ordering 2: ref1=y , ref2=x
                if (F[y][v][v]==ans1 && F[x][v][v]==ans2) ok=true;
                if (ok) nxt.push_back(pr);
            }
            cand.swap(nxt);
            if (cand.size()!=1) answer_minus_one();
        }
        X=cand[0].first; Y=cand[0].second;
        // fill the remaining values
        for (int k=1;k<=n;++k)
            if (k!=ref1 && k!=ref2){
                long long ans = ansK[k];
                for (int z=1;z<=4;++z)
                    if (F[X][Y][z]==ans){a[k]=z;break;}
            }
    }

    /*--------------------------------------------------------*/
    // count frequencies (including the two references)
    int cnt[5]={0};
    for (int i=1;i<=n;++i){
        if (i==ref1||i==ref2) continue;
        ++cnt[a[i]];
    }
    cnt[X]++; cnt[Y]++;

    int total_ge2 = cnt[2]+cnt[3]+cnt[4];
    if (total_ge2==0){
        vector<int> all1(n+1,1);
        answer(all1);
    }
    bool hasDouble = false;
    for (int v=2;v<=4;++v) if (cnt[v]>=2) hasDouble=true;
    if (!hasDouble) answer_minus_one();

    /*--------------------------------------------------------*/
    // finally determine the two reference values
    if (X==Y){
        a[ref1]=a[ref2]=X;
    }else{
        int r=-1;
        for (int i=1;i<=n;++i)
            if (i!=ref1 && i!=ref2 && a[i]==X){ r=i; break; }
        if (r!=-1){
            long long ans = ask(ref1, r, ref2);
            int val=-1;
            for (int z=1;z<=4;++z) if (F[X][Y][z]==ans){val=z;break;}
            a[ref1]=val;
            a[ref2]=X+Y-val;
        }else{
            for (int i=1;i<=n;++i)
                if (i!=ref1 && i!=ref2 && a[i]==Y){ r=i; break; }
            if (r!=-1){
                long long ans = ask(ref1, r, ref2);
                int val=-1;
                for (int z=1;z<=4;++z) if (F[X][Y][z]==ans){val=z;break;}
                a[ref1]=val;
                a[ref2]=X+Y-val;
            }else{
                // need a value occurring at least twice (guaranteed by hasDouble)
                int v=-1;
                for (int val=2;val<=4;++val) if (cnt[val]>=2 && val!=X && val!=Y){ v=val; break; }
                if (v==-1) answer_minus_one(); // should not happen
                int p=-1,q=-1;
                for (int i=1;i<=n;++i)
                    if (i!=ref1 && i!=ref2 && a[i]==v){
                        if (p==-1) p=i;
                        else if (q==-1){ q=i; break; }
                    }
                long long ans1 = ask(ref1,p,q);
                int val=-1;
                for (int z=1;z<=4;++z) if (F[v][v][z]==ans1){val=z;break;}
                a[ref1]=val;
                a[ref2]=X+Y-val;
            }
        }
    }

    answer(a);
    return 0;
}

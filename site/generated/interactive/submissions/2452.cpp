#include <bits/stdc++.h>
using namespace std;

static int F[5][5][5];
static int G[5][5][5];
static int n;
static vector<int> val;
static vector<int> reps[5];
static int cntv[5];
static long long nq = 0;
static long long LIM = 0;  // 0 = automatic
static int USE_TREE = 1;
static int UNIP = 0;
static double PCNT = 1.0;
static int FREEZE = 1;
static int PARK = 0;
static double TGUARD = 1.0;
static int WMAX = 16;
static double MIXB = 0.5;
static int AGE = 24;
static vector<int> sampleIdx;
static mt19937 rng(12345);
static size_t SAMP = 1 << 30;
#ifdef SIM
static vector<int> hidden;
#endif

static int area(int a, int b, int c) {
    if (a + b > c && b + c > a && a + c > b) return (a + b + c) * (-a + b + c) * (a - b + c) * (a + b - c);
    return 0;
}
static int ask(int i, int j, int k) {
    int t[3] = {i, j, k};
    sort(t, t + 3);
    nq++;
#ifdef SIM
    return area(hidden[t[0]], hidden[t[1]], hidden[t[2]]);
#else
    printf("? %d %d %d\n", t[0], t[1], t[2]);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
#endif
}
static int repsSig() { int s = 0; for (int v = 1; v <= 4; v++) s = s * 3 + (int)reps[v].size(); return s; }
static void setKnown(int idx, int v) {
    val[idx] = v; cntv[v]++;
    if (reps[v].size() < 2) reps[v].push_back(idx);
}
static void prior(double pr[5]) {
    double tot = 0;
    for (int v = 1; v <= 4; v++) { pr[v] = UNIP ? 1.0 : cntv[v] + PCNT; tot += pr[v]; }
    for (int v = 1; v <= 4; v++) pr[v] /= tot;
}

struct Q { int it[3]; };  // >=0: position, <0: known value -v

static void probeSets(vector<int>& k1, vector<pair<int,int>>& k2) {
    for (int v = 1; v <= 4; v++) if (reps[v].size() >= 1) k1.push_back(v);
    for (int v = 1; v <= 4; v++) for (int w = v; w <= 4; w++) {
        if (v == w && reps[v].size() < 2) continue;
        if (v != w && (reps[v].empty() || reps[w].empty())) continue;
        k2.push_back({v, w});
    }
}
static vector<Q> buildQueries(int m) {
    vector<Q> qs;
    vector<int> k1; vector<pair<int,int>> k2;
    probeSets(k1, k2);
    for (int a = 0; a < m; a++) {
        for (auto& p : k2) qs.push_back({{a, -p.first, -p.second}});
        for (int b = a + 1; b < m; b++) {
            for (int v : k1) qs.push_back({{a, b, -v}});
            for (int c = b + 1; c < m; c++) qs.push_back({{a, b, c}});
        }
    }
    return qs;
}
static void issue(const Q& q, const vector<int>& E, int& r) {
    int idx[3]; int used[5] = {0};
    for (int t = 0; t < 3; t++) {
        if (q.it[t] >= 0) idx[t] = E[q.it[t]];
        else { int v = -q.it[t]; idx[t] = reps[v][used[v]++]; }
    }
    r = ask(idx[0], idx[1], idx[2]);
}

// ---------------- parked set P (independent of window) ----------------
static vector<int> P;
static vector<vector<uint8_t>> Pc;  // list of assignments (values 1..4)

static void solveP() {
    while (Pc.size() > 1) {
        int m = P.size();
        double pr[5]; prior(pr);
        size_t Cn = Pc.size();
        vector<double> w(Cn, 1.0);
        double Wt = 0;
        for (size_t c = 0; c < Cn; c++) { for (int p = 0; p < m; p++) w[c] *= pr[Pc[c][p]]; Wt += w[c]; }
        vector<Q> qs = buildQueries(m);
        double best = -1; int bq = -1;
        for (size_t qi = 0; qi < qs.size(); qi++) {
            const Q& q = qs[qi];
            double h[13] = {0}; int cn[13] = {0};
            for (size_t c = 0; c < Cn; c++) {
                int v[3];
                for (int t = 0; t < 3; t++) v[t] = q.it[t] >= 0 ? Pc[c][q.it[t]] : -q.it[t];
                int g = G[v[0]][v[1]][v[2]];
                h[g] += w[c]; cn[g]++;
            }
            int nz = 0; double ent = 0;
            for (int r = 0; r < 13; r++) if (cn[r]) { nz++; double p = h[r] / Wt; ent += MIXB * (-p * log(p > 0 ? p : 1)) + (1 - MIXB) * (-p * log((double)cn[r] / Cn)); }
            if (nz < 2) continue;
            if (ent > best) { best = ent; bq = qi; }
        }
        if (bq < 0) break;
        const Q& q = qs[bq];
        int r; issue(q, P, r);
        vector<vector<uint8_t>> nc;
        for (auto& a : Pc) {
            int v[3];
            for (int t = 0; t < 3; t++) v[t] = q.it[t] >= 0 ? a[q.it[t]] : -q.it[t];
            if (F[v[0]][v[1]][v[2]] == r) nc.push_back(a);
        }
        Pc.swap(nc);
        if (Pc.empty()) exit(0);
    }
    // remove fixed
    for (size_t p = 0; p < P.size();) {
        bool same = true;
        for (auto& a : Pc) if (a[p] != Pc[0][p]) { same = false; break; }
        if (same) {
            setKnown(P[p], Pc[0][p]);
            P.erase(P.begin() + p);
            for (auto& a : Pc) a.erase(a.begin() + p);
        } else p++;
    }
    sort(Pc.begin(), Pc.end());
    Pc.erase(unique(Pc.begin(), Pc.end()), Pc.end());
    if (P.empty()) Pc.assign(1, {});
}

// ---------------- window ----------------
static vector<int> W;
static vector<long long> born;
static vector<vector<uint8_t>> col;   // col[p][c] value 1..4
static size_t C = 1;

static void addElem(int idx) {
    size_t C2 = C * 4;
    for (auto& v : col) {
        vector<uint8_t> nv(C2);
        for (size_t c = 0; c < C; c++) for (int t = 0; t < 4; t++) nv[c * 4 + t] = v[c];
        v.swap(nv);
    }
    vector<uint8_t> nv(C2);
    for (size_t c = 0; c < C; c++) for (int t = 0; t < 4; t++) nv[c * 4 + t] = t + 1;
    col.push_back(nv);
    W.push_back(idx);
    born.push_back(nq);
    C = C2;
}
static void filterBy(const vector<char>& keep) {
    size_t k = 0;
    for (size_t c = 0; c < C; c++) if (keep[c]) {
        for (auto& v : col) v[k] = v[c];
        k++;
    }
    for (auto& v : col) v.resize(k);
    C = k;
}
static void eraseCol(size_t p) {
    W.erase(W.begin() + p);
    born.erase(born.begin() + p);
    col.erase(col.begin() + p);
}
static void removeResolved() {
    for (size_t p = 0; p < W.size();) {
        bool same = true;
        uint8_t v0 = col[p][0];
        for (size_t c = 1; c < C; c++) if (col[p][c] != v0) { same = false; break; }
        if (same) { setKnown(W[p], v0); eraseCol(p); }
        else p++;
    }
}
// park elements known to be non-1 that probes cannot split and that are independent of the rest
static bool tryPark() {
    if (!PARK) return false;
    bool any = false;
    vector<int> k1; vector<pair<int,int>> k2;
    probeSets(k1, k2);
    for (size_t p = 0; p < W.size();) {
        int msk = 0;
        for (size_t c = 0; c < C; c++) msk |= 1 << col[p][c];
        bool ok = !(msk & 2) && __builtin_popcount(msk) >= 2;
        if (ok) {
            for (auto& pp : k2) {
                int first = -1; bool split = false;
                for (int v = 2; v <= 4; v++) if (msk >> v & 1) {
                    int g = G[v][pp.first][pp.second];
                    if (first < 0) first = g; else if (g != first) { split = true; break; }
                }
                if (split) { ok = false; break; }
            }
        }
        if (ok) {
            // factorization check
            unordered_map<string, int> seen;
            string key;
            size_t m = W.size();
            for (size_t c = 0; c < C; c++) {
                key.clear();
                for (size_t q = 0; q < m; q++) if (q != p) key.push_back((char)col[q][c]);
                seen[key] |= 1 << col[p][c];
            }
            for (auto& kv : seen) if (kv.second != msk) { ok = false; break; }
            if (ok) {
                // keep one row per rest-assignment
                vector<char> keep(C, 0);
                unordered_set<string> done;
                for (size_t c = 0; c < C; c++) {
                    key.clear();
                    for (size_t q = 0; q < m; q++) if (q != p) key.push_back((char)col[q][c]);
                    if (done.insert(key).second) keep[c] = 1;
                }
                filterBy(keep);
                // add to P
                vector<vector<uint8_t>> nc;
                for (auto& a : Pc) for (int v = 2; v <= 4; v++) if (msk >> v & 1) { auto b = a; b.push_back(v); nc.push_back(b); }
                Pc.swap(nc);
                P.push_back(W[p]);
                eraseCol(p);
                any = true;
                continue;
            }
        }
        p++;
    }
    // group parking: all non-1 multi-valued elements jointly
    {
        vector<int> S;
        for (size_t p = 0; p < W.size(); p++) {
            int msk = 0;
            for (size_t c = 0; c < C; c++) msk |= 1 << col[p][c];
            if (!(msk & 2) && __builtin_popcount(msk) >= 2) S.push_back(p);
        }
        if (S.size() >= 2 && S.size() <= 6) {
            vector<char> inS(W.size(), 0);
            for (int p : S) inS[p] = 1;
            size_t m = W.size();
            map<string, set<string>> mp;
            set<string> sset;
            string kr, ks;
            for (size_t c = 0; c < C; c++) {
                kr.clear(); ks.clear();
                for (size_t q = 0; q < m; q++) (inS[q] ? ks : kr).push_back((char)col[q][c]);
                mp[kr].insert(ks); sset.insert(ks);
            }
            bool ok = true;
            for (auto& kv : mp) if (kv.second.size() != sset.size()) { ok = false; break; }
            if (ok) {
                // S-joint must be unsplittable by queries within S + probes
                vector<Q> qs = buildQueries(S.size());
                vector<string> sl(sset.begin(), sset.end());
                for (auto& q : qs) {
                    int first = -1; bool split = false;
                    for (auto& a : sl) {
                        int v[3];
                        for (int t = 0; t < 3; t++) v[t] = q.it[t] >= 0 ? a[q.it[t]] : -q.it[t];
                        int g = G[v[0]][v[1]][v[2]];
                        if (first < 0) first = g; else if (g != first) { split = true; break; }
                    }
                    if (split) { ok = false; break; }
                }
            }
            if (ok) {
                vector<char> keep(C, 0);
                set<string> done;
                for (size_t c = 0; c < C; c++) {
                    kr.clear();
                    for (size_t q = 0; q < m; q++) if (!inS[q]) kr.push_back((char)col[q][c]);
                    if (done.insert(kr).second) keep[c] = 1;
                }
                filterBy(keep);
                vector<vector<uint8_t>> nc;
                for (auto& a : Pc) for (auto& b : sset) { auto x = a; for (char ch : b) x.push_back((uint8_t)ch); nc.push_back(x); }
                Pc.swap(nc);
                for (int p : S) P.push_back(W[p]);
                for (int i = (int)S.size() - 1; i >= 0; i--) eraseCol(S[i]);
                any = true;
            }
        }
    }
    return any;
}

static double scoreQ(const Q& q, const vector<double>& w, double Wt) {
    const uint8_t* ptr[3]; int fv[3];
    for (int t = 0; t < 3; t++) {
        if (q.it[t] >= 0) { ptr[t] = col[q.it[t]].data(); fv[t] = 0; }
        else { ptr[t] = nullptr; fv[t] = -q.it[t]; }
    }
    double h[13] = {0}; int cn[13] = {0};
    const size_t S = sampleIdx.size();
    const int* id = sampleIdx.data();
    if (ptr[0] && ptr[1] && ptr[2]) {
        for (size_t k = 0; k < S; k++) { int c = id[k]; int g = G[ptr[0][c]][ptr[1][c]][ptr[2][c]]; h[g] += w[c]; cn[g]++; }
    } else if (ptr[0] && ptr[1]) {
        for (size_t k = 0; k < S; k++) { int c = id[k]; int g = G[ptr[0][c]][ptr[1][c]][fv[2]]; h[g] += w[c]; cn[g]++; }
    } else {
        for (size_t k = 0; k < S; k++) { int c = id[k]; int g = G[ptr[0][c]][fv[1]][fv[2]]; h[g] += w[c]; cn[g]++; }
    }
    int nz = 0; double ent = 0; double Cs = 0; for (int r = 0; r < 13; r++) Cs += cn[r];
    Wt = 0; for (int r = 0; r < 13; r++) Wt += h[r];
    for (int r = 0; r < 13; r++) if (cn[r] > 0) { nz++; double p = h[r] / Wt; ent += MIXB * (-p * log(p > 0 ? p : 1)) + (1 - MIXB) * (-p * log((double)cn[r] / Cs)); }
    if (nz < 2) return -1;
    return ent;
}

// returns false if no splitting query
static bool step() {
    if (C <= 1) return false;
    int m = W.size();
    double pr[5]; prior(pr);
    vector<double> w(C, 1.0);
    for (int p = 0; p < m; p++) for (size_t c = 0; c < C; c++) w[c] *= pr[col[p][c]];
    double Wt = 0; for (auto x : w) Wt += x;
    vector<Q> qs = buildQueries(m);
    sampleIdx.resize(C);
    for (size_t c = 0; c < C; c++) sampleIdx[c] = c;
    if (C > SAMP) {
        for (size_t k = 0; k < SAMP; k++) { size_t j = k + rng() % (C - k); swap(sampleIdx[k], sampleIdx[j]); }
        sampleIdx.resize(SAMP);
        sort(sampleIdx.begin(), sampleIdx.end());
    }
    vector<char> frozen(m, 0);
    bool anyFrozen = false;
    if (FREEZE && !reps[1].empty()) {
        vector<int> k1; vector<pair<int,int>> k2;
        probeSets(k1, k2);
        for (int p = 0; p < m; p++) {
            int msk = 0;
            for (size_t c = 0; c < C; c++) msk |= 1 << col[p][c];
            if ((msk & 2) || __builtin_popcount(msk) < 2) continue;
            bool split = false;
            for (auto& pp : k2) {
                int first = -1;
                for (int v = 2; v <= 4 && !split; v++) if (msk >> v & 1) {
                    int g = G[v][pp.first][pp.second];
                    if (first < 0) first = g; else if (g != first) split = true;
                }
                if (split) break;
            }
            if (!split) { frozen[p] = 1; anyFrozen = true; }
        }
    }
    int forced = -1;
    for (int p = 0; p < m; p++) if (!frozen[p] && nq - born[p] > AGE) { forced = p; break; }
    double best = -1; int bq = -1;
    for (int pass = 0; pass < 3 && bq < 0; pass++) {
        if (pass == 1) { if (forced < 0) continue; born[forced] = nq; forced = -1; }
        if (pass == 2) { if (!anyFrozen) break; anyFrozen = false; }
        for (size_t qi = 0; qi < qs.size(); qi++) {
            const Q& q = qs[qi];
            if (forced >= 0 && q.it[0] != forced && q.it[1] != forced && q.it[2] != forced) continue;
            if (anyFrozen && ((q.it[0] >= 0 && frozen[q.it[0]]) || (q.it[1] >= 0 && frozen[q.it[1]]) || (q.it[2] >= 0 && frozen[q.it[2]]))) continue;
            double e = scoreQ(q, w, Wt);
            if (e > best) { best = e; bq = qi; }
        }
    }
    if (bq < 0) return false;
    const Q& q = qs[bq];
    int r; issue(q, W, r);
    vector<char> keep(C);
    for (size_t c = 0; c < C; c++) {
        int vv[3];
        for (int t = 0; t < 3; t++) vv[t] = q.it[t] >= 0 ? col[q.it[t]][c] : -q.it[t];
        keep[c] = F[vv[0]][vv[1]][vv[2]] == r;
    }
    filterBy(keep);
    if (C == 0) exit(0);
    removeResolved();
    return true;
}

// ---------------- optimal precomputed decision trees for small n ----------------
static const char* TREE5 = "05629134783437747471131474187878313377837191713987839211881278231188138329141419668124396661338324411391136668331131362422233416423271319342833311335125623714141777121121831856134277771281131111222214677783223332292333323335242222214224293333422214235523325552252933333333333432525251843239612122121111222214222422933334214224224293333212222";
static const char* TREE6 = "07>6?@19;A354B2244<2:22852B233::;8=BA:;BA:;31B132A2:334:1323:33:3342:2:A12;B2B922;23:2CB;92;4:;B219C@2243=:244;4=182:4A4244244=422;A924332B28@8A<42;3AB229228@2<249B41A<11;4;388@12:32B441B=435;8;5;@1;8155:38@1B444B44451;:1B3=A26182:64?21?4:2;<133A1<31B38:=99A44?22124;2<A23:5;=1;;<=A1281C22@11:536::=1@2?33342;=12?;?A111:1AA::1?A53:=45@4@422::B;2BB212B1<C55:3611262232:1=;;<C115=;=1?;?A1115:A111:A:5:1=;;1:<14836311313333>B28@345=34491319:4255122@152158:1@4:1A<11:1A@1=3@3>B24=@6686693313:625@225@411@1A1@1=::A1@1486684433=8512993<62644B1411226@14414414464843A3@1C514>148866AC644<:>4:>><=84444444A8444813813A848=4<195363:99>A1>:66>A44>444?495@924B95:22524=6355>444>444>B15C8482;11352322A25A:31;361116:3418664118@@@@6484394454?318@@@@B844494454B843?148864883C123783231212781787878787>B26:349;94431316:2455144;154153;1;2;1?C11;1?;1:2;2<1626<33<163C11211>618341211?23A67>B24:;9966633318:925;245;211;1?1;1::;?1;<<>6311211113C6>9111>614411361>9111>61233332333323333233337:3514666C64622B1211449;12212212292323<1;111;;;>123333?<922C;<2;<<<<<112118<9311>?861112;22111>338119464A3?331626333381111C111136115333?333233335333?333@3363336363635333?333233332333373:2C165666=66>?1>=66>?22>222A265;624B65;24524:9255>222>2223821411A4>61211316116232?3A63>A1121211:>6?1>1?6>1>1>?:11221131113A?31A63?31617>B15<3234@11832842?45?;31@891119;8?9819238638>A=112211>63122111361>16111136178213332183138181138131533333?32333323333533333?3233335A33A3?2222A533333?31533333?323333533333?323333533333?3@36336333636323333712333233383181318313813";
static const char* TREE7 = "0O;G>DKL2H71B5?551<3337@5639131JH715696969545?119AN56A6N46A4N1216@862287156161112<<<<33339M1:1BN25669MBN56M523323<23B1N21:1?I323<L12IJ6JII1J3M232131?<22323?23<2236M=1111=1M=MM==B888===K9L1EAH6251J233256A2811?19K1EF3?BHA11?1==?3188MAH56252181=6EC2=Q681M?1818==88?6C6A6192<EI169L16<A??181888==?18181818==?I61311C1ICIICC6@@@CCC1K6H957QH22732<2A?2=N=2LNN2?L?7L32L88K16HQH7LNN333<22AL2?1N?27166H35M7<I7<7?12@12267<JIRN1L2N27JC56@222?A5N3<2<3666<18F6?:<<<3133662L47572712222MRE37513751:132112N@3<3<3666N3<<323<6667J@32:?5?5522:2J:JJ::?555:::3NI6?R=BHE=H814@=21C1E6671419E1MB=9IMMIQ62A62==62I7722K@427IBBN3?E=H77R=2@B2IMM<2422C1136ID=96?=5BM6?=9@F5:232CF2:3:A?H=2@K57J@C57135311?@5IJRM1L3M3@L:6<511176<?2@562@2B24E255N221M=1:1125B495A39365A3D9936HM251C2:5?C1@25:5M221?M2212@@2559N6D1FCCFC5F11<D@12<1@2<1:@1211>2BH>1BHAH1B2252B13@351335A693A19N4R57?571I577R353>:3K1553131155469@2CF33?A?ABFC25222223J52A55FC2522H?::A:BFC2125522522<52FF?255252F5222A::H?::7J@2F3:?:?25C52:25@A1F65223FL1FLFL3FF233FL1FLFL2FF3AF3FFG3NI6><E=QE8>B245E1292=?6A2426=2K8=2IKKIR71761EE?1IA511M5415I8857:77?D62<1D<11617?@D3:H273?2>MKE2J24357J25KGN36=E><<QE1581IKKD14119223?I<=2<?=3K8<?=25:776A??D<77>K<?>IE43<?>?1975A35559L3575:555233933=E5:1::1992F111@:6A1:67775:3Q1I2KKILJ>N53ILQP6J33@A<6?7<L?CD<@<@676DJ1:55552554K18>7757A3G555EN:55:77333753A847777FL?2<66A<262626262=E75:5J:53333:555351:775E55515E5551515:7733GN<?45115157213619:359:36195:22H18>H28>7>2811@182353<23367<23716HKM=1L1435HI=43253HM:=7674575354366D17D<4352<61253GN4Q@56592I@5:Q383H63M2<<32322<<4<2AL15MA56D17<<7436677<<2<6159:67672323=ENA555J:5:555333351:77515:7733E55515E5551=75:J:55:77555M:59595N:N=3:555355@:2A5:2393333@F1A95923235L:A1F1:48B9?C15?CB59BB1981B8B8B888588;GN36?R<2@19DCQD<?195<9KII6<?1E=:1<:91C1=D6?568@21511261<J6L5?<B11J2:52N>I2?11@<?6?1?@2??BF2241214@2559IK4F27C@?I4574FLL@@GNHI151156D?625=?I16?275261655222224K99622822295581596622C8G:5AK9118C81A58;GK61C<D9I77L?:95EDD=M1=1<92=2<<66??8DA3JI?AD5:J:?N2?1<161B9<EN3721M7FC2955<2@N3722<?@5FL@@@456GN151D2?25K611I?I462I552826E65I55222262EK9:6666@2222622155E65I55@2222C511I5;EK915C8I8155?1I518I1??1?K?B1C?5?4CK5I11?1?1155EK55154I9518488;GJR9D3CPI88B7781515192L<6<696L41226L9112=52=4?6D5418EDDNA??4?1MK4?18448N?K4154=F>43241EQ11B=1?2K2==5:J:?L:113122J2124?N2?A111<116A1IL2F2337C@1?I@423371?B2<92?@4343C2@5FL@@@F6J11L222@11I23355FKM5KL26515GN157111D22?5752K61115755I?G7155216GIJ1:133A9525LM5??17?CJ1411277AK1B17154KI552521I856582659215555;CK<6I5555565556:1K975557LE1E1BE1B8=GA111?1AA??1M<?445665?1EBBK7E1G6B21??@CM5N5KA1K?K155G62556165G61571656GN222565??656266255CKK515C555?;G6?N:D758=5>JJJJ112JJ1>I2C1>IC1K2I8CE1EE3M291I8E@192>I11ELE2921?44@4943K5K?2==98987K=C22A21K7C1127?C72N>KC162235171NC131N1111111N1231?C7N1213A11312257C2CMJ491A1N?1A122K2GMA91117K?A3??26951A5GNHM91511362A25M1A11?2@1@GN9132551617GN12222227737711?16662717GN21325561?62726951GN21235521611G711311155A161@A91111;GJ9?KN341524B6B6151:88>P13H@81455MK43445B1254?J8=245?M1@?7111?I?31137@1?96N125?2F6BM?1K2155E15656525I75I@95@615GI632235562127G5125561216227C5151GN21551251155;?CK9I55IE1E115KEEE1C8EE15I???155E555EK95515E555155;FM9K7555556765M542464KG1622IE1E1BK1E1B8?1L2EE1E88=1GBG@E1E1E1E1BBCM5N5KK?A1KFI5M5@12K?2@G61151?5G722325711??5G5156255G675651G7555651155P:?C61121?C612111552561525551553I91155?455A7575:?C61121?C612114554651525551553I91155?455A7575:?C61121?C61211155256152555155>?C71311DII???@?357D55@615155155155553I91155?5?5A7575;?498CIC1515I815K4E8BC44E4511EE155I???E51988E5115?15E555155OPQR1232323R13RR21Q2RQRQR;QQ3;;4P1Q2RQPQRQPPQRQ1::P::Q:PQ;;Q;;;PQ4:QPP::Q:P:::QPO;GN<DI435117?5863@112?1E??D3518421621222=?6?26Q762326?59?7AA176?C175555N>?3?2132??297A2?2R@::P1211F?1113>1@@J2@23?:J1@AAAAGNH636123166167=A3173232@A931:11111P>11111Q11113339:111112QG55L>151327636345B188;2B5M1551741G1122121;G17Q8291<19262626P15151515P:4<;881Q114821515>311:1D1:11:11373737P:1115:111652:111515P17=?G2?548@46242;11;11>?13511121:112;12;12P:;E144237;112>31123737;B15111P:121515:12159:1D:12P:1237:1237:129:D337P4848489CO;G>D926?1333A7517=66EB1E9?1<EC???1626?6637766:5757N3?1?9?9R1?3N3::9H?2@@111??AN3:::?3A:@AAAA@A:GN36266667>A11333P373711A1<=37371111137=>15111137111372673=373713711127663348B8;G179=8219<91262626;G=B233G112212121;B1111;G7J1=3821??12<11C2G261116226;G1119111G122;G7@1?51?12>?2129@11C21G1G5126G12;PD1B15:9911G1CC;BE111;G111911212G1P:1115:1115:11165215;B1111O;=>1232333231293191312213>331223112>13113131233=>33232311132334412121212;R22224PQRQ111P11Q1;=1122=2=2P;41Q4P11Q1P111Q49A45@1267888788878882OQ78P86887888122122K3212OO2333@2226777O7627E222?@22QRI13R12PQ213133R2PO3121212121;2144421;2144421:GB2C??2C;?2C?;@E1E?G;@@22G8622122=24241146888M222244P86M2224481DM4222G42224K42222442222E821121122221RG22M2C2ERC?;;;O@11R2R@G@QR?22=?2G=@RC1@G@EG?;@;==?;@;@11@??C1@G@ABBBBHA811111B111144;:1:31J1JJ61131D131A;41;41;41B111:BBB341144N44DG781811Q88722N3113338M4M>M4222211222G111GM444M4M4DK111444>111>111M44QF48:53J1N1571BG4441K417333711375121D5211F511121F11F311F11CQ38:171744N=141437137113821F111D11>343333QO=4811;1:1113D3131FFQ:4814814481371333DF11311FF1>378844313731319>?2343434343434F423223GG43443443443443G2422=21F2333<21<2<23333122DG221122D212@1112DG312221C21221221<<3333122H3411111:31::11D1H7;4;411111;4:3:11JJJ111:1H343333DG111444>111>111M44DG1114M4G111>121222GMMDM1433731444N>M412144M11MH343333H7444337773DM13313:44NN1144R4H74443377739HJM>C5211212431;<134;234;<13243:K168K76868:611D167525757;36>52613K@L71R1525K@755524KL47446256225586<E18E85425>61524D6J42P@3116?@343333F2333<35F3216JFG141;2P32:2LL2CC1=CJM72:262CC72<JM1:3:33:==2==11C166<4=1=JF<;32222<2146?@322J=::322=11CC2J<;13:22<11CC2?21336H:BJ22R81218:33515:JK2:51JJ5H78121111;4111;4:31:311J21J11:1H:;11183337377HJ;=322F1R12<1KK4;;1G;1737:411:<2121D2211J211121J11J311J11?DM143:<:1444R>M;12144M11MDG324J2JJ221>22>JMM1MM21?12211N>121>22222222222M4222M1N>1121G>1111MDK1742F17:88E114N4222>2M1N418>34111311331131DM:4:1<:4M11M11M;1121N14>R4DKF43F:::34=1N>G1111M414R>4H741318737773O;MA6JNH151318N3N357C@==G451E:=<2464886881>19@1431KI131N5L7719937<C1227597:3H5?:3?@D>H<9>:9:3=9337371:34:3371379:G3DI1117:<1123G32C77A221<12327631GN16:3127663348;5845;=98G612;596K25691212;461R1251122G22248;58;G5M81B1911222G7G1G111C12;F282G=2G122;=122112EE2291112G1112G11221G12;=>1EG121G1122121;15E11;9E11122GE2G1;G1121GG1G2;B1E119>123434343434:12212:22:<32112:2212>:344:11::31:1>343333Q<44433<<<3>:;111:11::11:1>343333Q<444333<3<<>3433339>?2343434343434F423232G4GO43P243443443441<2<23333?2<2<21311<<3333DG312261C11:21122122D2@2211D@2222122H7;4;41;4111:1133:J1D1JJ1HA8;4114131113D:131JJDM13:13344RN1144N4DM3431734M11M11M41121R14>N4QF;14433<<3H74443337377DGM4MG4M>22221G114DM444M44>11>114H3433339H17=24G422444222L423232M4M744222422O43P243434444D211F21F62<232<23D222CJ=11511122I161JM31221122J=222621J<2=2261?222>:344:11:D1:1H3;81;414;41:31333DJ11:11JJ1Q<44433<<<3DGJ43J3333481R>K1111M414N>4DG131333414N41R14N4>343333DM444M44>11>114DG4M4GM4G11>22122M344444N4O;<P1Q23223233Q9144444222=P111==2333333=31213=3223323=3332334412121212;R22224PQ=Q111P11Q1;=112==2=2;42=224;222=;22=24";
static vector<int> RV;  // response values sorted
static void treeMode(int nn, const char* tr) {
    int N = 1 << (2 * nn);
    vector<array<int,3>> qs;
    for (int a = 0; a < nn; a++) for (int b = a + 1; b < nn; b++) for (int c = b + 1; c < nn; c++) qs.push_back({a, b, c});
    int QN = qs.size();
    vector<vector<uint8_t>> R(QN, vector<uint8_t>(N));
    for (int q = 0; q < QN; q++) for (int c = 0; c < N; c++) {
        int v[3];
        for (int t = 0; t < 3; t++) v[t] = ((c >> (2 * qs[q][t])) & 3) + 1;
        R[q][c] = G[v[0]][v[1]][v[2]];
    }
    struct Node { int q; int child[13]; };
    vector<Node> nodes;
    size_t pos = 0, len = strlen(tr);
    function<int(const vector<int>&)> parse = [&](const vector<int>& S) -> int {
        if (S.size() <= 1) return -1;
        bool any = false;
        for (int q = 0; q < QN && !any; q++) for (size_t k = 1; k < S.size(); k++) if (R[q][S[k]] != R[q][S[0]]) { any = true; break; }
        if (!any) return -1;
        if (pos >= len) return -2;
        int q = tr[pos++] - '0';
        int id = nodes.size();
        Node nd; nd.q = q; for (int r = 0; r < 13; r++) nd.child[r] = -1;
        nodes.push_back(nd);
        vector<int> part[13];
        for (int c : S) part[R[q][c]].push_back(c);
        for (int r = 0; r < 13; r++) if (!part[r].empty()) { int ch = parse(part[r]); nodes[id].child[r] = ch; }
        return id;
    };
    vector<int> S(N);
    for (int c = 0; c < N; c++) S[c] = c;
    int node = parse(S);
    while (S.size() > 1) {
        int q;
        if (node >= 0) q = nodes[node].q;
        else {
            // fallback (should not happen): any splitting query
            q = -1;
            for (int qq = 0; qq < QN && q < 0; qq++) for (size_t k = 1; k < S.size(); k++) if (R[qq][S[k]] != R[qq][S[0]]) { q = qq; break; }
            if (q < 0) break;
        }
        int r = ask(qs[q][0] + 1, qs[q][1] + 1, qs[q][2] + 1);
        int g = lower_bound(RV.begin(), RV.end(), r) - RV.begin();
        if (g >= (int)RV.size() || RV[g] != r) exit(0);
        vector<int> ns;
        for (int c : S) if (R[q][c] == g) ns.push_back(c);
        S.swap(ns);
        if (S.empty()) exit(0);
        node = node >= 0 ? nodes[node].child[g] : -1;
        if (node == -1 && S.size() > 1) {
            bool any = false;
            for (int qq = 0; qq < QN && !any; qq++) for (size_t k = 1; k < S.size(); k++) if (R[qq][S[k]] != R[qq][S[0]]) { any = true; break; }
            if (!any) break;
            node = -3;  // use fallback
        }
    }
    if (S.size() == 1) {
        for (int i = 0; i < nn; i++) val[i + 1] = ((S[0] >> (2 * i)) & 3) + 1;
    } else {
        P.push_back(1);  // mark ambiguous
    }
}

int main(int argc, char** argv) {
    {
        vector<int> rs;
        for (int a = 1; a <= 4; a++) for (int b = 1; b <= 4; b++) for (int c = 1; c <= 4; c++) {
            F[a][b][c] = area(a, b, c); rs.push_back(F[a][b][c]);
        }
        sort(rs.begin(), rs.end()); rs.erase(unique(rs.begin(), rs.end()), rs.end());
        for (int a = 1; a <= 4; a++) for (int b = 1; b <= 4; b++) for (int c = 1; c <= 4; c++)
            G[a][b][c] = lower_bound(rs.begin(), rs.end(), F[a][b][c]) - rs.begin();
        RV = rs;
    }
    if (scanf("%d", &n) != 1) return 0;
#ifdef SIM
    hidden.assign(n + 1, 0);
    for (int i = 1; i <= n; i++) if (scanf("%d", &hidden[i]) != 1) return 1;
    if (argc > 1) LIM = atoll(argv[1]);
    if (argc > 2) WMAX = atoi(argv[2]);
    if (argc > 3) MIXB = atof(argv[3]);
    if (argc > 4) AGE = atoi(argv[4]);
    if (argc > 5) SAMP = atoll(argv[5]);
    if (argc > 6) USE_TREE = atoi(argv[6]);
    if (argc > 7) UNIP = atoi(argv[7]);
    if (argc > 8) PCNT = atof(argv[8]);
    if (argc > 9) FREEZE = atoi(argv[9]);
    if (argc > 10) PARK = atoi(argv[10]);
    if (argc > 11) TGUARD = atof(argv[11]);
#endif
    if (LIM <= 0) {
        long long L = 6000000LL / n;
        L = max(2048LL, min(16384LL, L));
        if (n <= 8) L = 65536;
        LIM = L;
    }
    val.assign(n + 1, 0);
    Pc.assign(1, {});
    int nxt = 1;
    int lastSig = repsSig();
    auto t0 = chrono::steady_clock::now();
    bool smallTree = (n >= 5 && n <= 7) && USE_TREE;
    if (smallTree) { treeMode(n, n == 5 ? TREE5 : n == 6 ? TREE6 : TREE7); nxt = n + 1; }
    while (!smallTree) {
        {
            double el = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
            if (el > 6.5 * TGUARD) LIM = min<long long>(LIM, 128);
            else if (el > 4.5 * TGUARD) LIM = min<long long>(LIM, 512);
        }
        while (nxt <= n && C * 4 <= (size_t)LIM && (int)W.size() < WMAX) addElem(nxt++);
        bool progressed = step();
        bool parked = tryPark();
        if (parked || (repsSig() != lastSig && !P.empty())) {
            lastSig = repsSig();
            solveP();
        }
        lastSig = repsSig();
        if (progressed || parked) continue;
        if (nxt <= n) { addElem(nxt++); continue; }
        break;
    }
    // final: merge window into P and solve
    if (!W.empty()) {
        vector<vector<uint8_t>> nc;
        for (auto& a : Pc) for (size_t c = 0; c < C; c++) {
            auto b = a;
            for (size_t p = 0; p < W.size(); p++) b.push_back(col[p][c]);
            nc.push_back(b);
        }
        for (int x : W) P.push_back(x);
        Pc.swap(nc);
        W.clear();
    }
    if (!smallTree && !P.empty()) solveP();
#ifdef SIM
    bool amb = !P.empty();
    int c[5] = {0};
    for (int j = 1; j <= n; j++) c[hidden[j]]++;
    bool trueAmb;
    if (n == 3) trueAmb = !(hidden[1] == hidden[2] && hidden[2] == hidden[3]);
    else trueAmb = (c[1] < n && c[2] <= 1 && c[3] <= 1 && c[4] <= 1) || (n == 4 && c[1] == 1 && c[2] == 2 && c[4] == 1);
    bool ok = (amb == trueAmb);
    if (!amb) for (int j = 1; j <= n; j++) if (val[j] != hidden[j]) ok = false;
    printf("%s %lld %d %.4f\n", ok ? "OK" : "WRONG", nq, n, (double)nq / n);
#else
    if (!P.empty()) printf("! -1\n");
    else { printf("!"); for (int j = 1; j <= n; j++) printf(" %d", val[j]); printf("\n"); }
    fflush(stdout);
#endif
}

#include <bits/stdc++.h>
using namespace std;

int n;
long long total_sum;
int mod_n2;
int r2;

int ask(int k, const vector<int>& pos) {
    cout << "? " << k;
    for (int x : pos) cout << " " << x;
    cout << "\n";
    cout.flush();
    int ans;
    if (!(cin >> ans)) exit(0);
    if (ans == -1) exit(0);
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    if (n == 2) {
        cout << "! 1 2\n";
        cout.flush();
        return 0;
    }
    total_sum = 1LL * n * (n + 1) / 2;
    mod_n2 = n - 2;
    r2 = (mod_n2 > 0) ? (int)(total_sum % mod_n2) : 0;

    // parity relative to p1
    vector<int> sameParity(n + 1, 0);
    sameParity[1] = 1;
    for (int i = 2; i <= n; ++i) {
        int ans = ask(2, {1, i});
        sameParity[i] = ans;
    }

    // find positions of numbers 1 and n using n-1 queries
    vector<int> cand;
    vector<int> allPos(n);
    iota(allPos.begin(), allPos.end(), 1);
    for (int i = 1; i <= n; ++i) {
        vector<int> S;
        S.reserve(n - 1);
        for (int x : allPos) if (x != i) S.push_back(x);
        int ans = ask(n - 1, S);
        if (ans == 1) cand.push_back(i);
    }
    if ((int)cand.size() != 2) return 0;
    int candA = cand[0], candB = cand[1];

    // query parity between position 1 and candA
    int ansAB = ask(2, {1, candA});

    int parity_p1 = -1; // 0 even, 1 odd
    int pos_one = -1, pos_n = -1;
    for (int assumed = 0; assumed < 2; ++assumed) {
        vector<int> parityAbs(n + 1);
        for (int i = 1; i <= n; ++i) {
            parityAbs[i] = sameParity[i] ? assumed : 1 - assumed;
        }
        // determine which candidate is odd (value 1)
        int oddCand = (parityAbs[candA] == 1) ? candA : candB;
        int evenCand = (oddCand == candA) ? candB : candA;
        int parityValCandA = (candA == oddCand) ? 1 : 0;
        int expectedAns = (assumed == parityValCandA) ? 1 : 0;
        if (expectedAns == ansAB) {
            parity_p1 = assumed;
            pos_one = oddCand; // value 1
            pos_n = evenCand;  // value n
            break;
        }
    }
    if (parity_p1 == -1) return 0;

    // absolute parity of each position
    vector<int> parityAbs(n + 1);
    for (int i = 1; i <= n; ++i) {
        parityAbs[i] = sameParity[i] ? parity_p1 : 1 - parity_p1;
    }

    // known positions and values
    vector<int> knownPos;
    vector<int> knownVal;
    vector<char> isKnown(n + 1, 0);
    knownPos.push_back(pos_one);
    knownVal.push_back(1);
    isKnown[pos_one] = 1;
    knownPos.push_back(pos_n);
    knownVal.push_back(n);
    isKnown[pos_n] = 1;
    vector<int> posToVal(n + 1, -1);
    posToVal[pos_one] = 1;
    posToVal[pos_n] = n;

    // function to add a new known value using partner method
    auto add_new_known = [&]() -> bool {
        for (size_t idx = 0; idx < knownPos.size(); ++idx) {
            int pos_v = knownPos[idx];
            int val_v = knownVal[idx];
            int target = (r2 - val_v) % mod_n2;
            if (target < 0) target += mod_n2;
            // compute candidate numbers with this residue
            vector<int> candidates;
            int cand1 = target;
            if (cand1 == 0) cand1 = mod_n2;
            if (cand1 >= 1 && cand1 <= n) candidates.push_back(cand1);
            int cand2 = cand1 + mod_n2;
            if (cand2 >= 1 && cand2 <= n) candidates.push_back(cand2);
            // filter out already known numbers
            vector<int> unknownCandidates;
            for (int c : candidates) {
                bool known = false;
                for (int kv : knownVal) if (kv == c) { known = true; break; }
                if (!known) unknownCandidates.push_back(c);
            }
            if (unknownCandidates.empty()) continue;
            int neededVal = unknownCandidates[0];
            // scan unknown positions
            for (int j = 1; j <= n; ++j) {
                if (isKnown[j]) continue;
                vector<int> S;
                S.reserve(n - 2);
                for (int x = 1; x <= n; ++x) if (x != pos_v && x != j) S.push_back(x);
                int ans = ask(n - 2, S);
                if (ans == 1) {
                    knownPos.push_back(j);
                    knownVal.push_back(neededVal);
                    isKnown[j] = 1;
                    posToVal[j] = neededVal;
                    return true;
                }
            }
        }
        return false;
    };

    // primes to use (product > n)
    vector<int> primes = {2, 3, 5, 7, 11, 13};
    long long prod = 1;
    vector<int> usedPrimes;
    for (int p : primes) {
        if (prod > n) break;
        usedPrimes.push_back(p);
        prod *= p;
    }
    int maxPrime = usedPrimes.back();
    int requiredKnown = min(maxPrime + 1, n);
    while ((int)knownPos.size() < requiredKnown) {
        if (!add_new_known()) break;
    }

    // Ensure for each prime we have enough distinct residues among known values
    struct PrimeInfo {
        int p;
        vector<int> basePos; // size p+1
        vector<int> baseVal;
    };
    vector<PrimeInfo> primeInfos;
    for (int pi = 0; pi < (int)usedPrimes.size(); ++pi) {
        int p = usedPrimes[pi];
        PrimeInfo info;
        info.p = p;
        // need at least p distinct residues
        while (true) {
            unordered_map<int, pair<int,int>> mp; // residue -> (pos,val)
            for (size_t i = 0; i < knownPos.size(); ++i) {
                int val = knownVal[i];
                int res = val % p;
                if (mp.find(res) == mp.end())
                    mp[res] = {knownPos[i], val};
                if ((int)mp.size() == p) break;
            }
            if ((int)mp.size() == p) {
                // ensure we have at least p+1 known values total
                if ((int)knownPos.size() < p + 1) {
                    if (!add_new_known()) break;
                    continue;
                }
                // fill basePos and baseVal
                for (auto &kv : mp) {
                    info.basePos.push_back(kv.second.first);
                    info.baseVal.push_back(kv.second.second);
                }
                // add one extra known (any not already in basePos)
                for (size_t i = 0; i < knownPos.size(); ++i) {
                    int pos = knownPos[i];
                    if (find(info.basePos.begin(), info.basePos.end(), pos) == info.basePos.end()) {
                        info.basePos.push_back(pos);
                        info.baseVal.push_back(knownVal[i]);
                        break;
                    }
                }
                // ensure size is p+1
                while ((int)info.basePos.size() < p + 1) {
                    // add any remaining known
                    for (size_t i = 0; i < knownPos.size(); ++i) {
                        int pos = knownPos[i];
                        if (find(info.basePos.begin(), info.basePos.end(), pos) == info.basePos.end()) {
                            info.basePos.push_back(pos);
                            info.baseVal.push_back(knownVal[i]);
                            break;
                        }
                    }
                }
                break;
            } else {
                if (!add_new_known()) break;
            }
        }
        primeInfos.push_back(info);
    }

    int numPrimes = usedPrimes.size();
    vector<vector<int>> residues(n + 1, vector<int>(numPrimes, -1));
    // prime 2 residues are just parityAbs (0 even,1 odd)
    for (int i = 1; i <= n; ++i) residues[i][0] = parityAbs[i];

    // For each other prime, compute residues
    for (int pi = 1; pi < numPrimes; ++pi) {
        int p = usedPrimes[pi];
        const PrimeInfo &info = primeInfos[pi];
        // precompute a map from position to value for quick lookup
        // posToVal already contains known values; unknown positions have -1
        for (int i = 1; i <= n; ++i) {
            // build cur of size p not containing i
            vector<int> cur;
            cur.reserve(p);
            for (int pos : info.basePos) {
                if (pos != i) {
                    cur.push_back(pos);
                    if ((int)cur.size() == p) break;
                }
            }
            if ((int)cur.size() < p) {
                for (size_t idx = 0; idx < knownPos.size() && (int)cur.size() < p; ++idx) {
                    int pos = knownPos[idx];
                    if (pos != i && find(cur.begin(), cur.end(), pos) == cur.end()) {
                        cur.push_back(pos);
                    }
                }
            }
            // compute total sum of values in cur modulo p
            int totalSum = 0;
            vector<int> curVals(p);
            for (int k = 0; k < p; ++k) {
                int val = posToVal[cur[k]];
                curVals[k] = val;
                totalSum = (totalSum + (val % p)) % p;
            }
            int foundResidue = -1;
            for (int t = 0; t < p; ++t) {
                int sumWithout = (totalSum - (curVals[t] % p) + p) % p;
                // query with i and cur except t
                vector<int> queryPos;
                queryPos.reserve(p);
                queryPos.push_back(i);
                for (int k = 0; k < p; ++k) {
                    if (k == t) continue;
                    queryPos.push_back(cur[k]);
                }
                int ans = ask(p, queryPos);
                if (ans == 1) {
                    int residue = (p - sumWithout) % p;
                    foundResidue = residue;
                    break;
                }
            }
            if (foundResidue == -1) {
                // compute missing residue
                vector<char> possible(p, 0);
                for (int t = 0; t < p; ++t) {
                    int sumWithout = (totalSum - (curVals[t] % p) + p) % p;
                    int residue = (p - sumWithout) % p;
                    possible[residue] = 1;
                }
                for (int r = 0; r < p; ++r) if (!possible[r]) { foundResidue = r; break; }
            }
            residues[i][pi] = foundResidue;
        }
    }

    // assign numbers to positions by matching residues
    vector<int> perm(n + 1, -1);
    vector<char> usedNum(n + 1, 0);
    // precompute residues of numbers
    vector<vector<int>> numResidues(numPrimes, vector<int>(n + 1));
    for (int x = 1; x <= n; ++x) {
        numResidues[0][x] = x % 2;
        for (int pi = 1; pi < numPrimes; ++pi) {
            int p = usedPrimes[pi];
            numResidues[pi][x] = x % p;
        }
    }
    for (int i = 1; i <= n; ++i) {
        for (int x = 1; x <= n; ++x) {
            if (usedNum[x]) continue;
            bool ok = true;
            for (int pi = 0; pi < numPrimes; ++pi) {
                if (residues[i][pi] != numResidues[pi][x]) {
                    ok = false; break;
                }
            }
            if (ok) {
                perm[i] = x;
                usedNum[x] = 1;
                break;
            }
        }
        if (perm[i] == -1) {
            // should not happen
            perm[i] = 1;
        }
    }

    // output answer
    cout << "! ";
    for (int i = 1; i <= n; ++i) {
        cout << perm[i];
        if (i < n) cout << " ";
    }
    cout << "\n";
    cout.flush();
    return 0;
}

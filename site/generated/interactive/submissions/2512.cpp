#include <bits/stdc++.h>
using namespace std;

int n;
long long query_count = 0;

int ask(const vector<int>& positions) {
    cout << "? " << positions.size();
    for (int x : positions) cout << ' ' << x + 1;
    cout << endl;
    ++query_count;
    int response;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    return response;
}

struct Class {
    int mod;
    int residue;
    vector<int> pos;
    bool resolved = false;
    bool flipped = false;
};

vector<Class> classes;
vector<int> class_of;
vector<int> calibrated_values;

void recover_next_bit(const Class& cl, vector<int>& bit) {
    int c = cl.pos.size();
    int k = 2 * cl.mod;
    vector<bitset<801>> equations;
    vector<int> rhs;

    auto add_equation = [&](const vector<int>& group, int value) {
        bitset<801> eq;
        for (int x : group) eq.set(x);
        equations.push_back(eq);
        rhs.push_back(value);
    };

    for (int i = 1; i < c; ++i) {
        vector<int> group = {cl.pos[0], cl.pos[i]};
        for (int j = 0; (int)group.size() < k; ++j) {
            if (j != 0 && j != i) group.push_back(cl.pos[j]);
        }
        add_equation(group, ask(group) ^ 1);
    }

    int rows = equations.size();
    vector<int> pivot_col(rows, -1);
    int r = 0;
    for (int col = 0; col < c; ++col) {
        int pivot = r;
        while (pivot < rows && !equations[pivot][col]) ++pivot;
        if (pivot == rows) continue;
        swap(equations[pivot], equations[r]);
        swap(rhs[pivot], rhs[r]);
        for (int i = 0; i < rows; ++i) {
            if (i != r && equations[i][col]) {
                equations[i] ^= equations[r];
                rhs[i] ^= rhs[r];
            }
        }
        pivot_col[r] = col;
        ++r;
    }
    bit.assign(c, 0);
    for (int i = 0; i < r; ++i) {
        if (pivot_col[i] != -1) bit[pivot_col[i]] = rhs[i];
    }
    if (count(bit.begin(), bit.end(), 1) != c / 2) {
        for (int& x : bit) x ^= 1;
    }
}

void binary_recovery() {
    classes.push_back({1, 0, {}});
    for (int i = 0; i < n; ++i) classes[0].pos.push_back(i);
    for (int at = 0; at < (int)classes.size(); ++at) {
        Class cl = classes[at];
        int c = cl.pos.size();
        if (!cl.resolved && c == 2) continue;
        if (c < 2 * cl.mod) continue;
        vector<int> bit;
        recover_next_bit(cl, bit);
        if (!cl.resolved && c % 2 == 0) {
            vector<int> values[2];
            for (int orientation = 0; orientation < 2; ++orientation) {
                for (int i = 0; i < c; ++i) {
                    int q = 2 * (i / 2) + (bit[i] ^ orientation);
                    values[orientation].push_back(cl.residue + 1 + cl.mod * q);
                }
            }
            int chosen = -1;
            for (int mask = 1; mask < (1 << min(c, 20)) && chosen == -1; ++mask) {
                int k = __builtin_popcount((unsigned)mask);
                if (k % 2 == 0 || k == 1) continue;
                int sum[2] = {0, 0};
                for (int i = 0; i < c && i < 20; ++i) {
                    if (mask >> i & 1) {
                        sum[0] += values[0][i];
                        sum[1] += values[1][i];
                    }
                }
                if ((sum[0] % k == 0) != (sum[1] % k == 0)) chosen = mask;
            }
            if (chosen == -1) {
                for (int excluded = 0; excluded < c && chosen == -1; ++excluded) {
                    int sum[2] = {0, 0};
                    for (int i = 0; i < c; ++i) {
                        if (i == excluded) continue;
                        sum[0] += values[0][i];
                        sum[1] += values[1][i];
                    }
                    if ((sum[0] % (c - 1) == 0) != (sum[1] % (c - 1) == 0)) {
                        chosen = (1 << 20) + excluded;
                    }
                }
            }
            if (chosen == -1) {
                cerr << "Orientation failed\n";
                exit(1);
            }
            vector<int> group;
            int expected[2] = {0, 0};
            if (chosen < (1 << 20)) {
                for (int i = 0; i < c && i < 20; ++i) {
                    if (chosen >> i & 1) group.push_back(cl.pos[i]);
                }
            } else {
                int excluded = chosen - (1 << 20);
                for (int i = 0; i < c; ++i) if (i != excluded) group.push_back(cl.pos[i]);
            }
            for (int x : group) {
                int i = find(cl.pos.begin(), cl.pos.end(), x) - cl.pos.begin();
                expected[0] += values[0][i];
                expected[1] += values[1][i];
            }
            int response = ask(group);
            int orientation = response == (expected[1] % group.size() == 0);
            if (response == (expected[0] % group.size() == 0)) orientation = 0;
            if (orientation) for (int& x : bit) x ^= 1;
        }
        Class next[2] = {{2 * cl.mod, cl.residue, {}}, {2 * cl.mod, cl.residue + cl.mod, {}}};
        next[0].resolved = next[1].resolved = true;
        for (int i = 0; i < c; ++i) next[bit[i]].pos.push_back(cl.pos[i]);
        classes[at].pos.clear();
        classes.push_back(next[0]);
        classes.push_back(next[1]);
    }
    class_of.assign(n, -1);
    for (int i = 0; i < (int)classes.size(); ++i) {
        for (int pos : classes[i].pos) class_of[pos] = i;
    }
}

vector<int> candidates(int pos) {
    const Class& cl = classes[class_of[pos]];
    vector<int> result;
    int residue = (cl.residue + (cl.resolved && cl.flipped ? cl.mod / 2 : 0)) % cl.mod;
    for (int value = residue + 1; value <= n; value += cl.mod) result.push_back(value);
    if (!cl.resolved) {
        int other = (cl.residue + cl.mod / 2) % cl.mod;
        for (int value = other + 1; value <= n; value += cl.mod) result.push_back(value);
    }
    sort(result.begin(), result.end());
    return result;
}

void resolve_class(int pos, int value) {
    Class& cl = classes[class_of[pos]];
    if (cl.resolved) return;
    cl.flipped = (value - 1) % cl.mod != cl.residue;
    cl.resolved = true;
}

using State = vector<int>;

vector<State> make_states(const vector<int>& group) {
    vector<State> states(1);
    for (int i = 0; i < (int)group.size(); ++i) {
        vector<State> next;
        vector<int> cand = candidates(group[i]);
        for (const State& state : states) {
            for (int value : cand) {
                if (group[i] == 0 && value > n / 2) continue;
                bool used = false;
                for (int j = 0; j < i; ++j) {
                    if (class_of[group[j]] != class_of[group[i]]) continue;
                    const Class& cl = classes[class_of[group[i]]];
                    int half = cl.mod / 2;
                    int orientation = ((value - 1 - cl.residue) % cl.mod + cl.mod) % cl.mod / half;
                    int previous = ((state[j] - 1 - cl.residue) % cl.mod + cl.mod) % cl.mod / half;
                    if (state[j] == value || orientation != previous) {
                        used = true;
                        break;
                    }
                }
                if (!used) {
                    State copy = state;
                    copy.push_back(value);
                    next.push_back(copy);
                }
            }
        }
        states.swap(next);
    }
    return states;
}

int response_for(const State& state, const vector<int>& indices) {
    int sum = 0;
    for (int i : indices) sum += state[i];
    return sum % indices.size() == 0;
}

vector<int> calibrate() {
    vector<int> group;
    for (int target = 4; target <= min(8, n); ++target) {
        group.clear();
        for (int i = 0; i < target; ++i) group.push_back(i);
        vector<State> states = make_states(group);
        if(n==8){State id(n);iota(id.begin(),id.end(),1);cerr<<target<<':'<<states.size()<<':'<<(find(states.begin(),states.end(),id)!=states.end())<<'\n';for(int i=0;i<n;++i){for(int x:candidates(i))cerr<<x;cerr<<'/';}cerr<<'\n';}
        while (states.size() > 1) {
            int best_mask = -1;
            int best_balance = -1;
            int total = 1 << target;
            for (int mask = 1; mask < total; ++mask) {
                int k = __builtin_popcount((unsigned)mask);
                if (k < 2) continue;
                vector<int> indices;
                for (int i = 0; i < target; ++i) {
                    if (mask >> i & 1) indices.push_back(i);
                }
                int ones = 0;
                for (const State& state : states) ones += response_for(state, indices);
                int balance = min(ones, (int)states.size() - ones);
                if (balance > best_balance) {
                    best_balance = balance;
                    best_mask = mask;
                }
            }
            if (best_balance == 0) break;
            vector<int> indices, positions;
            for (int i = 0; i < target; ++i) {
                if (best_mask >> i & 1) {
                    indices.push_back(i);
                    positions.push_back(group[i]);
                }
            }
            int answer = ask(positions);
            vector<State> filtered;
            for (const State& state : states) {
                if (response_for(state, indices) == answer) filtered.push_back(state);
            }
            states.swap(filtered);
        }
        if (states.size() == 1) {
            calibrated_values = states[0];
            return group;
        }
    }
    return {};
}

vector<int> solve_with_anchors(const vector<int>& anchors) {
    vector<int> answer(n, -1);
    for (int i = 0; i < (int)anchors.size(); ++i) {
        answer[anchors[i]] = calibrated_values[i];
        resolve_class(anchors[i], calibrated_values[i]);
    }

    for (int pos = 0; pos < n; ++pos) {
        if (answer[pos] != -1) continue;
        vector<int> cand = candidates(pos);
        while (cand.size() > 1) {
            int best_mask = -1;
            int best_balance = -1;
            int subsets = 1 << anchors.size();
            for (int mask = 0; mask < subsets; ++mask) {
                int k = __builtin_popcount((unsigned)mask) + 1;
                if (k < 2) continue;
                int sum = 0;
                for (int i = 0; i < (int)anchors.size(); ++i) {
                    if (mask >> i & 1) sum += answer[anchors[i]];
                }
                int ones = 0;
                for (int value : cand) ones += (value + sum) % k == 0;
                int balance = min(ones, (int)cand.size() - ones);
                if (balance > best_balance) {
                    best_balance = balance;
                    best_mask = mask;
                }
            }
            if (best_balance == 0) {
                cerr << "No distinguishing anchor query at position " << pos + 1 << '\n';
                exit(1);
            }
            vector<int> positions = {pos};
            int sum = 0;
            for (int i = 0; i < (int)anchors.size(); ++i) {
                if (best_mask >> i & 1) {
                    positions.push_back(anchors[i]);
                    sum += answer[anchors[i]];
                }
            }
            int response = ask(positions);
            vector<int> filtered;
            for (int value : cand) {
                if (((value + sum) % positions.size() == 0) == response) {
                    filtered.push_back(value);
                }
            }
            cand.swap(filtered);
        }
        answer[pos] = cand[0];
        resolve_class(pos, answer[pos]);
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    if (n == 2) {
        cout << "! 1 2" << endl;
        return 0;
    }
    binary_recovery();
    vector<int> anchors = calibrate();
    if (anchors.empty()) {
        cerr << "Calibration failed\n";
        return 1;
    }
    vector<int> answer = solve_with_anchors(anchors);
    if (answer[0] > n / 2) {
        for (int& value : answer) value = n + 1 - value;
    }
    cout << '!';
    for (int value : answer) cout << ' ' << value;
    cout << endl;
    return 0;
}

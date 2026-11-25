#include <bits/stdc++.h>
using namespace std;

int n;
bitset<200> full_mask;
unordered_map<string, int> cache;

string key_of(const bitset<200>& s) {
    string key(n, '0');
    for (int i = 0; i < n; ++i) {
        if (s[i]) key[i] = '1';
    }
    return key;
}

bool is_known(const bitset<200>& s) {
    return s.none() || s == full_mask || cache.contains(key_of(s));
}

int query(const bitset<200>& s) {
    if (s.none() || s == full_mask) return 0;
    string key = key_of(s);
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;

    cout << "? " << key << '\n' << flush;
    int answer;
    if (!(cin >> answer) || answer < 0) exit(0);
    cache.emplace(move(key), answer);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (n <= 1) {
        cout << "! 1\n" << flush;
        return 0;
    }
    for (int i = 0; i < n; ++i) full_mask.set(i);

    bitset<200> connected;
    connected.set(0);
    vector<int> outside;
    for (int i = 1; i < n; ++i) outside.push_back(i);

    int boundary = query(connected);
    while (!outside.empty()) {
        if (boundary == 0) {
            cout << "! 0\n" << flush;
            return 0;
        }

        vector<int> candidates = outside;
        bool contains_all_neighbors = true;
        int chosen = -1;
        int new_boundary = -1;

        while (chosen == -1) {
            int m = (int)candidates.size();
            if (m == 1 || (contains_all_neighbors && boundary == m)) {
                chosen = candidates[0];
                break;
            }

            bitset<200> first, second;
            vector<int> first_vertices, second_vertices;
            int first_size = m / 2;
            for (int i = 0; i < m; ++i) {
                if (i < first_size) {
                    first.set(candidates[i]);
                    first_vertices.push_back(candidates[i]);
                } else {
                    second.set(candidates[i]);
                    second_vertices.push_back(candidates[i]);
                }
            }

            int first_cost = !is_known(first) + !is_known(connected | first);
            int second_cost = !is_known(second) + !is_known(connected | second);
            bitset<200> tested = first;
            vector<int>* tested_vertices = &first_vertices;
            vector<int>* other_vertices = &second_vertices;
            if (second_cost < first_cost) {
                tested = second;
                tested_vertices = &second_vertices;
                other_vertices = &first_vertices;
            }

            int tested_query = query(tested);
            int joined_query = query(connected | tested);
            int cross = boundary + tested_query - joined_query;

            if (cross == 0) {
                candidates = *other_vertices;
            } else {
                candidates = *tested_vertices;
                contains_all_neighbors = false;
                if (candidates.size() == 1) {
                    chosen = candidates[0];
                    new_boundary = joined_query;
                }
            }
        }

        connected.set(chosen);
        outside.erase(find(outside.begin(), outside.end(), chosen));
        boundary = new_boundary == -1 ? query(connected) : new_boundary;
    }

    cout << "! 1\n" << flush;
    return 0;
}

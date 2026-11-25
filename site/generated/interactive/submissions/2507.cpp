#include <bits/stdc++.h>
using namespace std;

struct Group {
    int a, b;
};

struct MixedPair {
    int a, b;
};

int status_of[1001];
vector<int> counterfeit;
vector<Group> uniform_groups;
vector<MixedPair> mixed_pairs;
int reference_coin = -1;

int ask(int a, int b) {
    cout << "? " << a << ' ' << b << '\n' << flush;
    int answer;
    if (!(cin >> answer)) exit(0);
    return answer;
}

void set_kind(int coin, int kind) {
    if (status_of[coin] != -1) return;
    status_of[coin] = kind;
    if (kind == 1) counterfeit.push_back(coin);
}

void set_group_kind(int index, int kind) {
    set_kind(uniform_groups[index].a, kind);
    if (uniform_groups[index].b != -1) set_kind(uniform_groups[index].b, kind);
}

[[noreturn]] void finish() {
    cout << '!';
    for (int coin : counterfeit) cout << ' ' << coin;
    cout << '\n' << flush;
    exit(0);
}

int majority_candidate = -1;
int majority_balance = 0;
int majority_processed = 0;
vector<int> candidate_cluster;
vector<int> candidate_opposites;

void make_reference() {
    if (reference_coin != -1) return;
    if (majority_processed == 0 && uniform_groups.size() < 6) return;

    while (majority_processed < 11 && majority_processed < (int)uniform_groups.size()) {
        int index = majority_processed++;
        if (majority_balance == 0) {
            majority_candidate = index;
            majority_balance = 1;
            candidate_cluster = {index};
            candidate_opposites.clear();
        } else {
            int same = ask(uniform_groups[majority_candidate].a,
                           uniform_groups[index].a) == 0;
            if (same) {
                ++majority_balance;
                candidate_cluster.push_back(index);
            } else {
                --majority_balance;
                candidate_opposites.push_back(index);
            }
        }

        if (majority_processed == 6 && candidate_cluster.size() == 6) {
            for (int member : candidate_cluster) set_group_kind(member, 0);
            reference_coin = uniform_groups[majority_candidate].a;
            return;
        }
    }

    if (majority_processed == 11) {
        for (int member : candidate_cluster) set_group_kind(member, 0);
        for (int member : candidate_opposites) set_group_kind(member, 1);
        reference_coin = uniform_groups[majority_candidate].a;
        if (counterfeit.size() == 10) finish();
    }
}

void resolve_mixed_pairs() {
    for (const auto &pair : mixed_pairs) {
        if (ask(pair.a, reference_coin) == 1) {
            set_kind(pair.a, 1);
            set_kind(pair.b, 0);
        } else {
            set_kind(pair.a, 0);
            set_kind(pair.b, 1);
        }
        if (counterfeit.size() == 10) finish();
    }
    mixed_pairs.clear();
}

void resolve_remaining_groups() {
    erase_if(uniform_groups, [](const Group &group) {
        return status_of[group.a] != -1;
    });
    int n = uniform_groups.size();
    while (true) {
        int remaining = 10 - (int)counterfeit.size();
        if (remaining == 0) finish();

        vector<array<bool, 11>> prefix(n + 1), suffix(n + 1);
        for (int i = 0; i <= n; ++i) prefix[i].fill(false), suffix[i].fill(false);
        prefix[0][0] = true;
        for (int i = 0; i < n; ++i) {
            prefix[i + 1] = prefix[i];
            int weight = uniform_groups[i].b == -1 ? 1 : 2;
            for (int sum = 0; sum + weight <= remaining; ++sum)
                if (prefix[i][sum]) prefix[i + 1][sum + weight] = true;
        }
        suffix[n][0] = true;
        for (int i = n - 1; i >= 0; --i) {
            suffix[i] = suffix[i + 1];
            int weight = uniform_groups[i].b == -1 ? 1 : 2;
            for (int sum = 0; sum + weight <= remaining; ++sum)
                if (suffix[i + 1][sum]) suffix[i][sum + weight] = true;
        }

        int choice = -1;
        bool all_forced = true;
        for (int i = 0; i < n; ++i) {
            int weight = uniform_groups[i].b == -1 ? 1 : 2;
            bool can_be_genuine = false, can_be_counterfeit = false;
            for (int left = 0; left <= remaining; ++left) {
                if (prefix[i][left] && suffix[i + 1][remaining - left])
                    can_be_genuine = true;
                if (weight <= remaining && left + weight <= remaining &&
                    prefix[i][left] && suffix[i + 1][remaining - left - weight])
                    can_be_counterfeit = true;
            }
            if (can_be_genuine && can_be_counterfeit) {
                all_forced = false;
                if (choice == -1 || weight == 2) choice = i;
            } else if (can_be_counterfeit) {
                set_group_kind(i, 1);
            } else {
                set_group_kind(i, 0);
            }
        }
        if (all_forced) finish();

        int answer = ask(uniform_groups[choice].a, reference_coin);
        set_group_kind(choice, answer);
        erase_if(uniform_groups, [](const Group &group) {
            return status_of[group.a] != -1;
        });
        n = uniform_groups.size();
        if (counterfeit.size() == 10) finish();
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    fill(begin(status_of), end(status_of), -1);

    int coin = 1;
    while (coin + 1 <= n) {
        int a = coin, b = coin + 1;
        coin += 2;
        if (ask(a, b) == 0) {
            uniform_groups.push_back({a, b});
        } else {
            mixed_pairs.push_back({a, b});
        }

        make_reference();
        if (reference_coin != -1 && !mixed_pairs.empty()) resolve_mixed_pairs();
    }

    if (coin <= n) uniform_groups.push_back({coin, -1});
    make_reference();
    if (reference_coin != -1 && !mixed_pairs.empty()) resolve_mixed_pairs();

    resolve_remaining_groups();
}

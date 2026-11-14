#include <bits/stdc++.h>
using namespace std;

struct Node {
    long long value = -(1LL << 60);
    int count = INT_MAX;
};

static inline void relax(Node &a, const Node &b) {
    if (b.value > a.value || (b.value == a.value && b.count < a.count)) {
        a = b;
    }
}

struct Window {
    bool valid;
    bool left;
    bool right;
};

static inline Window getWindow(const string &s, int start, int len) {
    char target = 0;
    bool valid = false;
    if (len == 3) {
        char a = s[start], b = s[start + 1], c = s[start + 2];
        valid = (a == b || a == c || b == c) && !(a == b && b == c);
        if (a == b || a == c) target = a;
        else target = b;
    } else if (len == 4) {
        char a = s[start], b = s[start + 1], c = s[start + 2], d = s[start + 3];
        valid = a == d && (a == b || c == d);
        target = a;
    } else {
        char a = s[start], b = s[start + 1], c = s[start + 2];
        valid = a == b && a == s[start + 3] && a == s[start + 4] && a != c;
        target = a;
    }
    if (!valid) return {false, false, false};
    int end = start + len;
    return {true, start > 0 && s[start - 1] == target,
            end < (int)s.size() && s[end] == target};
}

static Node solvePenalty(const string &s, int penalty) {
    int n = s.size();
    vector<Node> best(n + 1);
    vector<array<Node, 2>> ending(n + 1);
    best[0] = {0, 0};

    for (int i = 0; i < n; ++i) {
        relax(best[i + 1], best[i]);
        for (int len = 3; len <= 5 && i + len <= n; ++len) {
            Window w = getWindow(s, i, len);
            if (!w.valid) continue;
            Node base = best[i];
            if (w.left) {
                base = best[i - 1];
                relax(base, ending[i][0]);
            }
            Node candidate{base.value + len - penalty, base.count + 1};
            relax(ending[i + len][w.right], candidate);
        }
        relax(best[i + 1], ending[i + 1][0]);
        relax(best[i + 1], ending[i + 1][1]);
    }
    return best[n];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        string s;
        int K;
        cin >> s >> K;

        Node initial = solvePenalty(s, 0);
        long long answer;
        if (initial.count <= K) {
            answer = initial.value;
        } else {
            int low = 0, high = 6;
            while (high - low > 1) {
                int mid = (low + high) / 2;
                if (solvePenalty(s, mid).count <= K) high = mid;
                else low = mid;
            }
            Node result = solvePenalty(s, high);
            answer = result.value + 1LL * high * K;
        }
        cout << answer << '\n';
    }
    return 0;
}

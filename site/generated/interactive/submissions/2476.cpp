#include <iostream>
#include <vector>

using namespace std;

class ZeroFinder {
    int n;
    vector<int> sum;

    int ask(int l, int r) {
        cout << "? " << l << ' ' << r << endl;
        int response;
        if (!(cin >> response) || response < 0) exit(0);
        return response;
    }

    int find(int v, int l, int r, int k) {
        if (l == r) {
            sum[v] = 1;
            return l;
        }

        int m = (l + r) / 2;
        int left = v * 2;
        int right = left + 1;

        if (sum[left] == -1) {
            if (sum[v] != -1 && sum[right] != -1)
                sum[left] = sum[v] - sum[right];
            else
                sum[left] = ask(l, m);
        }

        if (sum[v] != -1 && sum[right] == -1)
            sum[right] = sum[v] - sum[left];
        if (sum[v] == -1 && sum[right] != -1)
            sum[v] = sum[left] + sum[right];

        int leftZeros = m - l + 1 - sum[left];
        int answer;
        if (k <= leftZeros) {
            answer = find(left, l, m, k);
        } else {
            if (sum[v] == -1) sum[v] = ask(l, r);
            if (sum[right] == -1) sum[right] = sum[v] - sum[left];
            answer = find(right, m + 1, r, k - leftZeros);
        }

        if (sum[v] != -1) {
            ++sum[v];
        } else if (sum[left] != -1 && sum[right] != -1) {
            sum[v] = sum[left] + sum[right];
        }
        return answer;
    }

public:
    explicit ZeroFinder(int size) : n(size), sum(4 * size + 4, -1) {}

    int find(int k) {
        if (k == n) {
            sum[1] = 1;
            return n;
        }
        return find(1, 1, n, k);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t)) return 0;

    ZeroFinder finder(n);
    while (t--) {
        int k;
        if (!(cin >> k)) return 0;
        int answer = finder.find(k);
        cout << "! " << answer << endl;
    }
    return 0;
}

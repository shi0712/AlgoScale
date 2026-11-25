#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

bool lessThan(int i, int j) {
    if (i == j) return false;
    cout << "? " << i << ' ' << j << endl;
    int answer;
    if (!(cin >> answer) || answer == -1) exit(0);
    return answer == 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> values(n);
    for (int i = 0; i < n; ++i) values[i] = i + 1;
    if (values.size() > 2) stable_sort(values.begin(), values.end(), lessThan);

    bool preferFront = false;
    bool batchMode = false;
    while (values.size() > 1) {
        if (batchMode) {
            int m = values.size();
            int pairs = m / 2;
            vector<int> next;
            next.reserve(pairs + m % 2);
            for (int k = 0; k < pairs; ++k) {
                cout << "+ " << values[k] << ' ' << values[m - 1 - k] << endl;
                int sum;
                if (!(cin >> sum) || sum == -1) return 0;
                next.push_back(sum);
            }
            if (m % 2) next.push_back(values[pairs]);
            values = move(next);
            if (values.size() > 2) stable_sort(values.begin(), values.end(), lessThan);
            continue;
        }

        int i = values.front();
        int j = values.back();
        cout << "+ " << i << ' ' << j << endl;
        int sum;
        if (!(cin >> sum) || sum == -1) return 0;

        values.erase(values.begin());
        values.pop_back();

        if (values.size() <= 1) {
            values.push_back(sum);
            continue;
        }

        if (preferFront) {
            if (lessThan(sum, values.front())) {
                values.insert(values.begin(), sum);
                continue;
            }
            if (!lessThan(sum, values.back())) {
                values.push_back(sum);
                preferFront = false;
                continue;
            }
        } else {
            if (!lessThan(sum, values.back())) {
                values.push_back(sum);
                continue;
            }
            if (lessThan(sum, values.front())) {
                values.insert(values.begin(), sum);
                preferFront = true;
                continue;
            }
        }

        auto position = lower_bound(values.begin() + 1, values.end() - 1,
                                    sum, lessThan);
        values.insert(position, sum);
        batchMode = true;
    }

    cout << '!' << endl;
    return 0;
}

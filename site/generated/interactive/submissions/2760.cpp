#include <bits/stdc++.h>
using namespace std;

class Interactor {
public:
    bool lessValue(int a, int b) {
        if (a == b) return false;

        cout << "? " << a << ' ' << b << endl;
        int response;
        if (!(cin >> response) || response == -1) exit(0);
        return response == 1;
    }

    int add(int a, int b) {
        cout << "+ " << a << ' ' << b << endl;
        int newLabel;
        if (!(cin >> newLabel) || newLabel == -1) exit(0);
        return newLabel;
    }
};

Interactor interactor;

void mergeSort(vector<int>& a, vector<int>& buffer, int left, int right) {
    if (right - left <= 1) return;

    int middle = left + (right - left) / 2;
    mergeSort(a, buffer, left, middle);
    mergeSort(a, buffer, middle, right);

    int i = left, j = middle, k = left;
    while (i < middle && j < right) {
        if (interactor.lessValue(a[i], a[j])) {
            buffer[k++] = a[i++];
        } else {
            buffer[k++] = a[j++];
        }
    }
    while (i < middle) buffer[k++] = a[i++];
    while (j < right) buffer[k++] = a[j++];

    for (int p = left; p < right; ++p) {
        a[p] = buffer[p];
    }
}

void insertSorted(vector<int>& a, int label) {
    int left = 0, right = static_cast<int>(a.size());

    while (left < right) {
        int middle = left + (right - left) / 2;
        if (interactor.lessValue(a[middle], label)) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }

    a.insert(a.begin() + left, label);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N) || N == -1) return 0;

    vector<int> values(N), buffer(N);
    iota(values.begin(), values.end(), 1);

    mergeSort(values, buffer, 0, N);

    /*
       Let m and M be the current minimum and maximum, and let T be the
       sum of all current board values.

       If m <= 0 <= M, then m + M is between m and M, hence is legal.
       If all values are nonnegative, then m + M <= T <= R.
       If all values are nonpositive, then m + M >= T >= -R.

       Thus adding the current minimum and maximum is always legal.
    */
    while (values.size() > 2) {
        int minimum = values.front();
        int maximum = values.back();

        values.erase(values.begin());
        values.pop_back();

        int sumLabel = interactor.add(minimum, maximum);

        if (values.size() == 1) {
            // Only two values will remain, so their relative order is irrelevant.
            values.push_back(sumLabel);
        } else {
            insertSorted(values, sumLabel);
        }
    }

    int a = values[0];
    int b = values[1];
    interactor.add(a, b);

    cout << "!" << endl;
    return 0;
}

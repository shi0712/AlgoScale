#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 2000;

static unsigned char memo[MAXN + 2][MAXN + 2];

int ask(int l, int r) {
    if (l >= r) return 0;

    unsigned char &cached = memo[l][r];
    if (cached != 0) return cached - 1;

    printf("? %d %d\n", l, r);
    fflush(stdout);

    int response;
    if (scanf("%d", &response) != 1 || response == -1) exit(0);
    if (response != 0 && response != 1) exit(0);

    cached = static_cast<unsigned char>(response + 1);
    return response;
}

// Returns 1 exactly when p[a] > p[b].
int inversionBetween(int a, int b) {
    if (a == b) return 0;
    if (a > b) swap(a, b);

    return ask(a, b)
         ^ ask(a + 1, b)
         ^ ask(a, b - 1)
         ^ ask(a + 1, b - 1);
}

bool greaterValue(int a, int b) {
    return inversionBetween(a, b) != 0;
}

void mergeSort(vector<int> &a, vector<int> &buffer, int left, int right) {
    if (right - left <= 1) return;

    int middle = (left + right) / 2;
    mergeSort(a, buffer, left, middle);
    mergeSort(a, buffer, middle, right);

    int i = left;
    int j = middle;
    int k = left;

    while (i < middle && j < right) {
        if (!greaterValue(a[i], a[j])) {
            buffer[k++] = a[i++];
        } else {
            buffer[k++] = a[j++];
        }
    }

    while (i < middle) buffer[k++] = a[i++];
    while (j < right) buffer[k++] = a[j++];

    for (int pos = left; pos < right; ++pos) {
        a[pos] = buffer[pos];
    }
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;

    vector<int> order(n);
    vector<int> buffer(n);
    iota(order.begin(), order.end(), 1);

    mergeSort(order, buffer, 0, n);

    vector<int> answer(n + 1);
    for (int i = 0; i < n; ++i) {
        answer[order[i]] = i + 1;
    }

    printf("!");
    for (int i = 1; i <= n; ++i) {
        printf(" %d", answer[i]);
    }
    printf("\n");
    fflush(stdout);

    return 0;
}

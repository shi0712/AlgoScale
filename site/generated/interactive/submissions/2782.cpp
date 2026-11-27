#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int M;
    if (!(cin >> M) || M == -1) return 0;

    int nextValue = 999;
    int N = 0;
    vector<int> leafValue(M, -1);

    for (int leaf = 0; leaf < M; ++leaf) {
        int lastSuccessfulValue = -1;

        while (true) {
            int x = (nextValue >= 0 ? nextValue : 0);
            cout << "? 1 " << leaf << ' ' << x << endl;

            int response;
            if (!(cin >> response) || response == -1) return 0;

            if (response == 1) {
                lastSuccessfulValue = x;
                --nextValue;
                ++N;
            } else {
                leafValue[leaf] = lastSuccessfulValue;
                break;
            }
        }
    }

    cout << "? 2" << endl;

    int s;
    if (!(cin >> s) || s == -1) return 0;

    vector<int> sequence(s);
    for (int &x : sequence) {
        if (!(cin >> x) || x == -1) return 0;
    }

    if (s != N) return 0;

    vector<int> parentIndex(s, -1);
    vector<int> st;
    st.reserve(s);

    for (int i = 0; i < s; ++i) {
        while (!st.empty() && sequence[i] > sequence[st.back()]) {
            st.pop_back();
        }

        if (i > 0) {
            if (st.empty()) return 0;
            parentIndex[i] = st.back();
        }

        st.push_back(i);
    }

    vector<int> valueToLeaf(1001, -1);
    for (int leaf = 0; leaf < M; ++leaf) {
        if (leafValue[leaf] < 0) return 0;
        valueToLeaf[leafValue[leaf]] = leaf;
    }

    vector<int> label(s, -1);
    label[0] = N - 1;

    int nextInternalLabel = M;
    for (int i = 1; i < s; ++i) {
        int leaf = valueToLeaf[sequence[i]];
        if (leaf != -1) {
            label[i] = leaf;
        } else {
            if (nextInternalLabel >= N - 1) return 0;
            label[i] = nextInternalLabel++;
        }
    }

    vector<int> reportedParent(N, -1);
    for (int i = 1; i < s; ++i) {
        reportedParent[label[i]] = label[parentIndex[i]];
    }

    cout << "! " << N;
    for (int i = 0; i < N - 1; ++i) {
        if (reportedParent[i] == -1) return 0;
        cout << ' ' << reportedParent[i];
    }
    cout << endl;

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

int queryCount = 0;

int ask(int parent, int edge) {
    int id = ++queryCount;
    cout << "? " << parent << ' ' << edge << endl;

    int response;
    if (!(cin >> response)) exit(0);
    if (response == -1) exit(0);
    return response;
}

int findGroup(int edge, int numberOfGroups,
              const vector<int>& prefixQuery) {
    int low = 1;
    int high = numberOfGroups;

    int step = 1;
    while (step < high) {
        int response = ask(prefixQuery[step], edge);
        if (response == 0) {
            high = step;
            break;
        }

        low = step + 1;
        if (step > numberOfGroups / 2) {
            step = numberOfGroups;
        } else {
            step *= 2;
        }
    }

    while (low < high) {
        int middle = low + (high - low) / 2;
        int response = ask(prefixQuery[middle], edge);

        if (response == 0) {
            high = middle;
        } else {
            low = middle + 1;
        }
    }

    return low;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m)) return 0;

    vector<int> groupOf(m + 1, -1);
    vector<int> representatives;
    vector<int> groupLength;

    vector<int> prefixQuery(1, 0);

    for (int edge = 1; edge <= m; ++edge) {
        int numberOfGroups = static_cast<int>(representatives.size());
        int currentQuery = prefixQuery[numberOfGroups];

        int response = ask(currentQuery, edge);

        if (response == 1) {
            int group = numberOfGroups;
            representatives.push_back(edge);
            groupLength.push_back(1);
            groupOf[edge] = group;
            prefixQuery.push_back(queryCount);
        } else if (numberOfGroups == 0) {
            groupOf[edge] = -1;
        } else {
            int singleEdgeResponse = ask(0, edge);

            if (singleEdgeResponse == 0) {
                groupOf[edge] = -1;
            } else {
                int group = findGroup(edge, numberOfGroups, prefixQuery);
                groupOf[edge] = group;
                ++groupLength[group - 1];
            }
        }
    }

    cout << "!";
    for (int edge = 1; edge <= m; ++edge) {
        int answer;
        if (groupOf[edge] == -1) {
            answer = -1;
        } else {
            answer = groupLength[groupOf[edge]];
            if (answer > 14) answer = 0;
        }
        cout << ' ' << answer;
    }
    cout << endl;

    int confirmation;
    cin >> confirmation;
    return 0;
}

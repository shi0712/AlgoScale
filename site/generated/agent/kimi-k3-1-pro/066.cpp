#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

using namespace std;

using Real = long double;
const Real INF = numeric_limits<Real>::infinity();

struct Edge {
    int from, to, weight;
};

int n, m;
vector<Edge> edges;
vector<char> useful;

Real valueAt(Real mu) {
    vector<Real> previous(n, INF), current(n);
    previous[0] = 0;
    Real answer = INF;

    for (int length = 1; length < n; ++length) {
        fill(current.begin(), current.end(), INF);
        for (const Edge &edge : edges) {
            int x = edge.from;
            int y = edge.to;
            if (!useful[x] || !useful[y] || previous[x] == INF) continue;
            Real difference = edge.weight - mu;
            Real candidate = previous[x] + difference * difference;
            current[y] = min(current[y], candidate);
        }
        if (current[n - 1] != INF) {
            answer = min(answer, current[n - 1] / length);
        }
        previous.swap(current);
    }

    vector<vector<Real>> dp(n + 1, vector<Real>(n, INF));
    for (int vertex = 0; vertex < n; ++vertex) {
        if (useful[vertex]) dp[0][vertex] = 0;
    }
    for (int length = 1; length <= n; ++length) {
        for (const Edge &edge : edges) {
            int x = edge.from;
            int y = edge.to;
            if (!useful[x] || !useful[y] || dp[length - 1][x] == INF) continue;
            Real difference = edge.weight - mu;
            Real candidate = dp[length - 1][x] + difference * difference;
            dp[length][y] = min(dp[length][y], candidate);
        }
    }

    Real cycleAnswer = INF;
    for (int vertex = 0; vertex < n; ++vertex) {
        if (dp[n][vertex] == INF) continue;
        Real maximum = -INF;
        for (int length = 0; length < n; ++length) {
            if (dp[length][vertex] == INF) continue;
            maximum = max(maximum,
                          (dp[n][vertex] - dp[length][vertex]) / (n - length));
        }
        cycleAnswer = min(cycleAnswer, maximum);
    }
    answer = min(answer, cycleAnswer);
    return max<Real>(0, answer);
}

struct Interval {
    Real left, right, middle;
    Real leftValue, rightValue, middleValue, lowerBound;

    bool operator<(const Interval &other) const {
        return lowerBound > other.lowerBound;
    }
};

Real intervalLowerBound(Real left, Real right, Real leftValue,
                        Real rightValue) {
    Real leftLinear = leftValue - left * left;
    Real rightLinear = rightValue - right * right;
    Real slope = (rightLinear - leftLinear) / (right - left);
    Real point = clamp(-slope / 2, left, right);
    Real linearValue = leftLinear + slope * (point - left);
    return point * point + linearValue;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m;
    edges.resize(m);
    vector<vector<int>> reverseGraph(n);
    for (Edge &edge : edges) {
        cin >> edge.from >> edge.to >> edge.weight;
        --edge.from;
        --edge.to;
        reverseGraph[edge.to].push_back(edge.from);
    }

    vector<char> reachable(n, false), canFinish(n, false);
    vector<int> stack = {0};
    reachable[0] = true;
    while (!stack.empty()) {
        int vertex = stack.back();
        stack.pop_back();
        for (const Edge &edge : edges) {
            if (edge.from == vertex && !reachable[edge.to]) {
                reachable[edge.to] = true;
                stack.push_back(edge.to);
            }
        }
    }
    stack = {n - 1};
    canFinish[n - 1] = true;
    while (!stack.empty()) {
        int vertex = stack.back();
        stack.pop_back();
        for (int previous : reverseGraph[vertex]) {
            if (!canFinish[previous]) {
                canFinish[previous] = true;
                stack.push_back(previous);
            }
        }
    }
    if (!reachable[n - 1]) {
        cout << -1 << '\n';
        return 0;
    }
    useful.resize(n);
    for (int vertex = 0; vertex < n; ++vertex) {
        useful[vertex] = reachable[vertex] && canFinish[vertex];
    }

    const Real left = 0;
    const Real right = 20;
    const Real middle = 10;
    Real leftValue = valueAt(left);
    Real rightValue = valueAt(right);
    Real middleValue = valueAt(middle);
    Real answer = min({leftValue, rightValue, middleValue});

    priority_queue<Interval> intervals;
    intervals.push({left, right, middle, leftValue, rightValue, middleValue,
                    intervalLowerBound(left, right, leftValue, rightValue)});

    const Real tolerance = 2e-11L;
    while (!intervals.empty()) {
        Interval interval = intervals.top();
        if (interval.lowerBound >= answer - tolerance) break;
        intervals.pop();

        Real firstMiddle = (interval.left + interval.middle) / 2;
        Real secondMiddle = (interval.middle + interval.right) / 2;
        Real firstValue = valueAt(firstMiddle);
        Real secondValue = valueAt(secondMiddle);
        answer = min({answer, firstValue, secondValue});

        intervals.push({interval.left, interval.middle, firstMiddle,
                        interval.leftValue, interval.middleValue, firstValue,
                        intervalLowerBound(interval.left, interval.middle,
                                           interval.leftValue,
                                           interval.middleValue)});
        intervals.push({interval.middle, interval.right, secondMiddle,
                        interval.middleValue, interval.rightValue, secondValue,
                        intervalLowerBound(interval.middle, interval.right,
                                           interval.middleValue,
                                           interval.rightValue)});
    }

    cout << fixed << setprecision(15) << answer << '\n';
    return 0;
}

#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<vector<int>> dist(n, vector<int>(n, -1));
    for (int i = 0; i < n; ++i) dist[i][i] = 0;

    auto query = [&](int a, int b) {
        if (dist[a][b] != -1) return dist[a][b];
        cout << "? " << a << ' ' << b << endl;
        int d;
        if (!(cin >> d)) exit(0);
        dist[a][b] = dist[b][a] = d;
        return d;
    };

    for (int i = 1; i < n; ++i) query(0, i);

    int b = 0;
    for (int i = 1; i < n; ++i) {
        if (dist[0][i] > dist[0][b]) b = i;
    }
    for (int i = 0; i < n; ++i) query(b, i);

    int c = b;
    for (int i = 0; i < n; ++i) {
        if (dist[b][i] > dist[b][c]) c = i;
    }
    int diameter = dist[b][c];
    for (int i = 0; i < n; ++i) query(c, i);

    vector<int> projection(n);
    for (int i = 0; i < n; ++i) {
        projection[i] = diameter + dist[b][i] - dist[c][i];
    }

    int left = 0, right = 2 * diameter;
    bool exact_center = false;
    for (int i = 0; i < n; ++i) {
        if (projection[i] == diameter) exact_center = true;
        if (projection[i] < diameter) left = max(left, projection[i]);
        if (projection[i] > diameter) right = min(right, projection[i]);
    }

    vector<int> centers;
    if (exact_center) centers.push_back(diameter / 2);
    else {
        centers.push_back(left / 2);
        centers.push_back(right / 2);
    }

    if (centers.size() == 2) {
        auto middle_count = [&](int center) {
            return count(projection.begin(), projection.end(), 2 * center);
        };
        if (middle_count(centers[1]) < middle_count(centers[0])) {
            swap(centers[0], centers[1]);
        }
    }

    int radius = -1;
    for (int center : centers) {
        int city_radius = 0;
        for (int i = 0; i < n; ++i) {
            int d;
            if (projection[i] <= 2 * center) {
                d = dist[c][i] - diameter + center;
            } else {
                d = dist[b][i] - center;
            }
            city_radius = max(city_radius, d);
        }
        if (radius == -1) radius = city_radius;
        else radius = min(radius, city_radius);
    }

    bool balanced = false;
    for (int center : centers) {
        int city_radius = 0;
        int before = 0, after = 0;
        vector<int> middle;
        for (int i = 0; i < n; ++i) {
            int d;
            if (projection[i] < 2 * center) {
                ++before;
                d = dist[c][i] - diameter + center;
            } else if (projection[i] > 2 * center) {
                ++after;
                d = dist[b][i] - center;
            } else {
                middle.push_back(i);
                d = dist[b][i] - center;
            }
            city_radius = max(city_radius, d);
        }
        if (city_radius != radius || max(before, after) > n / 2) continue;

        bool center_balanced = true;
        if ((int)middle.size() > n / 2) {
            int candidate = -1, votes = 0;
            for (int town : middle) {
                if (votes == 0) {
                    candidate = town;
                    votes = 1;
                } else {
                    int d = query(candidate, town);
                    bool same = d < dist[b][candidate] + dist[b][town] - 2 * center;
                    votes += same ? 1 : -1;
                }
            }

            int same_count = 1;
            for (int i = 0; i < (int)middle.size(); ++i) {
                int town = middle[i];
                if (town == candidate) continue;
                int d = query(candidate, town);
                same_count += d < dist[b][candidate] + dist[b][town] - 2 * center;
                int remaining = (int)middle.size() - i - 1;
                if (same_count > n / 2) break;
                if (same_count + remaining <= n / 2) break;
            }
            center_balanced = same_count <= n / 2;
        }
        balanced |= center_balanced;
        if (balanced) break;
    }

    cout << "! " << (balanced ? radius : -radius) << endl;
    return 0;
}

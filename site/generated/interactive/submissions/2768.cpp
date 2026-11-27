#include <bits/stdc++.h>
using namespace std;

static constexpr int N = 100000;
static constexpr long double HIT_EPS = 1e-7L;

long double ask(int x1, int y1, int x2, int y2) {
    cout << "query " << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << endl;

    string token;
    if (!(cin >> token)) exit(0);

    char* end = nullptr;
    long double value = strtold(token.c_str(), &end);
    if (end == token.c_str() || value < 0.0L) exit(0);

    return value;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
    seed ^= static_cast<uint64_t>(random_device{}()) << 32;
    mt19937_64 rng(seed);

    vector<int> rows;
    for (int y = 102; y <= 99801; y += 199) {
        rows.push_back(y);
    }
    shuffle(rows.begin(), rows.end(), rng);

    int y0 = -1;
    long double firstLength = 0.0L;

    for (int y : rows) {
        long double length = ask(0, y, N, y);
        if (length > HIT_EPS) {
            y0 = y;
            firstLength = length;
            break;
        }
    }

    if (y0 == -1) return 0;

    int lowRow, highRow;
    long double lowLength, highLength;

    long double nextLength = ask(0, y0 + 1, N, y0 + 1);
    if (nextLength > HIT_EPS) {
        lowRow = y0;
        highRow = y0 + 1;
        lowLength = firstLength;
        highLength = nextLength;
    } else {
        long double previousLength = ask(0, y0 - 1, N, y0 - 1);
        if (previousLength <= HIT_EPS) return 0;

        lowRow = y0 - 1;
        highRow = y0;
        lowLength = previousLength;
        highLength = firstLength;
    }

    long double aLow = (lowLength / 2.0L) * (lowLength / 2.0L);
    long double aHigh = (highLength / 2.0L) * (highLength / 2.0L);

    long double cyEstimate =
        static_cast<long double>(lowRow) + 0.5L + (aHigh - aLow) / 2.0L;
    long long cy = llround(cyEstimate);

    long long dLow = lowRow - cy;
    long long dHigh = highRow - cy;
    long double rSquared =
        (aLow + static_cast<long double>(dLow * dLow) +
         aHigh + static_cast<long double>(dHigh * dHigh)) / 2.0L;

    long long radius = llround(sqrtl(rSquared));

    long long d0 = y0 - cy;
    long long chordSquared = 1LL * radius * radius - d0 * d0;
    long double halfChord = sqrtl(static_cast<long double>(chordSquared));

    long long centerX;
    if (2 * radius == N) {
        centerX = N / 2;
    } else {
        int lowThreshold = static_cast<int>(
            floorl(static_cast<long double>(radius) - halfChord)
        );
        int highThreshold = static_cast<int>(
            floorl(static_cast<long double>(N - radius) - halfChord)
        ) + 1;

        bool haveHighResponse = false;
        long double highResponse = 0.0L;

        while (highThreshold - lowThreshold > 1) {
            int middle = (lowThreshold + highThreshold) / 2;
            long double response = ask(0, y0, middle, y0);

            if (response > HIT_EPS) {
                highThreshold = middle;
                highResponse = response;
                haveHighResponse = true;
            } else {
                lowThreshold = middle;
            }
        }

        if (!haveHighResponse) {
            highResponse = ask(0, y0, highThreshold, y0);
        }

        long double centerXEstimate =
            static_cast<long double>(highThreshold) - highResponse + halfChord;
        centerX = llround(centerXEstimate);
    }

    cout << "answer " << centerX << ' ' << cy << ' ' << radius << endl;
    return 0;
}

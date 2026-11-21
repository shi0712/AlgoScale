#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    const long long MAXV = 100000000000000LL; // 1e14
    long long B = 1;
    long long L = 0;
    long long R = MAXV;
    auto query = [&](long long X) -> string {
        cout << "? " << X << "\n";
        cout.flush();
        string resp;
        if(!(cin >> resp)) exit(0);
        return resp;
    };
    // initial query X=1
    string resp = query(1);
    if (resp == "Fired!") return 0;
    if (resp == "Fraudster!") {
        // M = 0
        cout << "! 0\n";
        cout.flush();
        return 0;
    } else if (resp == "Lucky!") {
        B += 1; // B = 2
        L = 1;
    } else {
        // unknown response
        return 0;
    }
    while (L < R) {
        long long mid = (L + R + 1) / 2;
        // ensure B >= mid + 1
        while (B < mid + 1) {
            if (B == 1) {
                // query 1, safe
                resp = query(1);
                if (resp == "Lucky!") {
                    B += 1;
                } else if (resp == "Fraudster!") {
                    // Should not happen because M >= L >= 1
                    B -= 1;
                    // update R?
                    // but this case shouldn't happen
                } else {
                    // Fired!
                    return 0;
                }
            } else {
                long long X = B - 1;
                if (X > MAXV) X = MAXV;
                resp = query(X);
                if (resp == "Lucky!") {
                    B += X;
                    if (X > L) L = X;
                } else if (resp == "Fraudster!") {
                    B -= X;
                    if (X - 1 < R) R = X - 1;
                } else {
                    // Fired!
                    return 0;
                }
            }
        }
        // now B >= mid + 1
        resp = query(mid);
        if (resp == "Lucky!") {
            B += mid;
            L = mid;
        } else if (resp == "Fraudster!") {
            B -= mid;
            R = mid - 1;
        } else {
            // Fired!
            return 0;
        }
    }
    // L == R
    cout << "! " << L << "\n";
    cout.flush();
    return 0;
}

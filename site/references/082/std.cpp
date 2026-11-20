// Compatibility edit: include the standard assertion header explicitly.
#include <cassert>
// Contestant submission 264517605, https://codeforces.com/contest/1979/submission/264517605
// Source: open-r1/codeforces-submissions (train-00035-of-00036.parquet), CC BY 4.0.
// https://huggingface.co/datasets/open-r1/codeforces-submissions
// https://creativecommons.org/licenses/by/4.0/
#include <bits/stdc++.h>

#define MOD 1000000007LL

using namespace std;
using ll = long long;
using pii = pair<int, int>;
using pll = pair<ll, ll>;
using vll = vector<ll>;

void solve()
{
    ll n;
    cin >> n;

    auto interact = [&](ll deg)
    {
        cout << "? " << deg << endl;
        pll ans;
        cin >> ans.first >> ans.second;
        return ans;
    };

    vector<vll> v(2);
    vll top(2);
    auto get_v = [&](pll ans)
    {
        if (top[0] == ans.first) return 1;
        if (top[1] == ans.first) return 0;
        if (top[0] == ans.second) return 0;
        return 1;
    };

    set<ll> rem;
    for (int i = 1; i <= n; ++i) rem.insert(i);
    ll prev = 0;
    for (; n > 2; --n)
    {
        assert(top[0] != top[1] || !top[0]);
        auto ans = interact(prev ? 0LL : n - 2);
        assert(ans.first);
        rem.erase(ans.first);
        if (!ans.second && !prev)
        {
            prev = ans.first;
            continue;
        }

        ll ind = get_v(ans);
        v[ind].push_back(ans.first);
        top[ind] = ans.second;
        if (prev) v[ind].push_back(prev), top[ind] = 0;
        prev = 0;
    }

    if (prev)
    {
        if (top[0] == prev) v[1].push_back(prev), top[1] = 0;
        else v[0].push_back(prev), top[0] = 0;
    }

    // do last 2
    assert(rem.size() == 2);
    ll a = *rem.begin();
    ll b = *next(rem.begin());
    if (a == top[0] || b == top[1]) swap(a, b);
    v[0].push_back(a), v[1].push_back(b);

    // print :D:D::DD:D:D:D:D:D:::DDDDD
    cout << "! ";
    for (int i = 0; i < v[0].size(); ++i) cout << v[0][i] << " ";
    for (int i = (ll) v[1].size() - 1; i >= 0; --i) cout << v[1][i] << " ";
    cout << endl;
}

int main()
{
    cin.tie(0)->sync_with_stdio(0);
    cin.exceptions(cin.failbit);

    solve();
}

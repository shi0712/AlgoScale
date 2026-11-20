#include <bits/stdc++.h>
using namespace std;

// The oracle scores an n-bit guess and may terminate the interaction on a hit.
template<class Oracle> string solve(int n, Oracle ask) {
    struct Column { int mask, bit, position; };
    vector<Column> columns;
    int rows = 1;
    while (int(columns.size()) + 1 < n) {
        int mask = rows++;
        for (int j = 0; j < __builtin_popcount(unsigned(mask)) &&
             int(columns.size()) + 1 < n; ++j)
            columns.push_back({mask, j, int(columns.size())});
    }
    vector<vector<int>> odd(rows, vector<int>(rows, INT_MAX));
    for (int mask=1; mask<rows; ++mask) {
        int rank=0;
        for (int t=0; t<=mask; ++t)
            if ((t & mask)==t && __builtin_parity(unsigned(t))) odd[mask][t]=rank++;
    }
    vector<vector<int>> active(rows);
    for (int x=1; x<rows; ++x)
        for (auto c:columns)
            if (odd[c.mask][x & c.mask] < (1<<c.bit)) active[x].push_back(c.position);
    vector<int> response(rows);
    int weight=0;
    for (int x=0; x<rows; ++x) {
        string g(n,'F');
        for (int i:active[x]) g[i]='T';
        int score=ask(g);
        if (score==n) return g;
        if (!x) weight=n-score;
        response[x]=(score-(n-weight)+int(active[x].size()))/2;
    }
    string answer(n,'F');
    for (int s=rows-1; s>0; --s) {
        int coefficient=0;
        for (int t=s;; t=(t-1)&s) {
            coefficient+=(__builtin_parity(unsigned(s^t))?-1:1)*response[t];
            if (t==0) break;
        }
        coefficient=abs(coefficient);
        for (auto c:columns) if (c.mask==s && (coefficient & (1<<c.bit))) {
            answer[c.position]='T'; --weight;
            for (int x=0; x<rows; ++x)
                if (odd[s][x&s] < (1<<c.bit)) --response[x];
        }
    }
    if (weight) answer.back()='T';
    if (ask(answer)!=n) throw runtime_error("Inconsistent grading");
    return answer;
}
#ifdef VERIFY
int main() {
    mt19937 rng(847101);
    int count=0, maximum=0;
    for (int n=1;n<=1000;++n) {
        int repetitions=n<=12?(1<<n):2;
        for (int seed=0;seed<repetitions;++seed) {
            string s(n,'F');
            for (int i=0;i<n;++i) if (n<=12?((seed>>i)&1):(rng()&1)) s[i]='T';
            int queries=0;
            auto answer=solve(n,[&](const string& g) {
                ++queries;
                return int(inner_product(s.begin(),s.end(),g.begin(),0,plus<int>(),equal_to<char>()));
            });
            if (answer!=s || queries>254) return 1;
            ++count; maximum=max(maximum,queries);
        }
    }
    cout<<count<<" verified answer keys, maximum "<<maximum<<" calls\n";
}
#else
int main() {
    int n;if (!(cin>>n)) return 0;
    solve(n,[&](const string& s) {
        cout<<s<<endl;int score;if (!(cin>>score) || score<0) exit(0);return score;
    });
}
#endif

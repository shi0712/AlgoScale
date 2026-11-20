#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int ask(char operation, int value) {
    cout << operation << ' ' << value << endl;
    int response;
    if (!(cin >> response) || response < 0) exit(0);
    return response;
}

int main() {
    int n;
    if (!(cin >> n)) return 0;
    vector<bool> prime(n+1, true), live(n+1, true);
    vector<int> primes;
    for (int p=2;p<=n;++p) {
        if (!prime[p]) continue;
        primes.push_back(p);
        for (long long j=1LL*p*p;j<=n;j+=p) prime[j]=false;
    }
    vector<int> batches={1414};
    for (int k=12;k>=1;--k) batches.push_back((1<<k)-1);
    int start=0, nominal=n;
    for (int limit : batches) {
        int end=min(int(primes.size()),start+limit);
        if (start==end) break;
        for (int j=start;j<end;++j) {
            int p=primes[j];
            ask('B',p);
            for (int v=p;v<=n;v+=p)
                if (live[v]) { live[v]=false; --nominal; }
        }
        if (ask('A',1)==nominal) { start=end; continue; }
        int lo=start;
        if (start==0) {
            while (lo<end && primes[lo]<=313) {
                int factor=primes[lo++];
                if (!ask('A',factor)) continue;
                int answer=factor;
                for (int p : primes) {
                    if (p>n/factor) break;
                    for (long long multiple=1LL*factor*p;multiple<=n;multiple*=p) {
                        if (!ask('A',int(multiple))) break;
                        answer*=p;
                    }
                }
                cout << "C " << answer << endl;
                return 0;
            }
        }
        // All small prime divisors have been excluded, so the secret is prime.
        for (int j=lo;j<end;++j) {
            if (j+1==end || ask('A',primes[j])) {
                cout << "C " << primes[j] << endl;
                return 0;
            }
        }
    }
    cout << "C 1" << endl;
}

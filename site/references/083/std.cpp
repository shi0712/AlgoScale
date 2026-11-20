#include <cstdlib>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
using namespace std;

int ask(int i, int j) {
    cout << "? " << i << ' ' << j << endl;
    int reply;
    if (!(cin >> reply) || reply < 0) exit(0);
    return reply;
}

pair<int,int> locate(int n) {
    if (n == 3) {
        int first = ask(1,2), second = ask(1,3);
        if (first > second) return {1,2};
        if (first < second) return {1,3};
        return {2,3};
    }
    int current = 1;
    int a = -1, b = -1, divisor = 1;
    for (; current+2 <= n; current += 3) {
        int i = current, j = i+1, k = i+2;
        int ij = ask(i,j);
        if (ij > 1) {
            a=i; b=j; divisor=ij;
            current=k;
            break;
        }
        int ik = ask(i,k);
        if (ik > 1) {
            // j=zero would imply p[i]=1, contradicting ik>1.
            a=i; b=k; divisor=ik;
            current=k+1;
            break;
        }
        int jk = ask(j,k);
        if (jk > 1) {
            // i cannot be zero since ij=ik=1 and p[j],p[k] differ.
            a=j; b=k; divisor=jk;
            current=k+1;
            break;
        }
        // Three ones exclude zero from all three positions.
    }
    if (a == -1) return {current,min(current+1,n)};

    const int firstDivisor = divisor;
    const int totalMultiples = 1 + (n-1)/firstDivisor;
    int seenMultiples = 2;  // the two anchors are divisible by firstDivisor
    for (int i=current; i<=n; ++i) {
        if (2*divisor>n-1 || seenMultiples==totalMultiples) break;
        int value=ask(a,i);
        if (value%firstDivisor==0) ++seenMultiples;
        if (value%divisor) continue;
        if (value>divisor) {
            b=i; divisor=value;
            continue;
        }
        // Equality rules out a; the new pair is b,i.
        a=b; b=i;
        if (i<n && seenMultiples<totalMultiples) divisor=ask(a,b);
    }
    return {a,b};
}

int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    {
        int n;if (!(cin>>n) || n<2)return 0;
        auto [i,j]=locate(n);
        cout<<"! "<<i<<' '<<j<<endl;
        int verdict;if (!(cin>>verdict) || verdict<0)return 0;
    }
}



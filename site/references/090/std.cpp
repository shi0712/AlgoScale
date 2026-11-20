#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;

long long query(const vector<int>& vertices) {
    cout << "? 1 " << vertices.size();
    for (int i : vertices) cout << ' ' << i;
    cout << endl;
    long long sum;
    if (!(cin >> sum)) exit(0);
    return sum;
}

int main() {
    {
        int n; cin >> n;
        for (int i=1,a,b;i<n;++i) cin >> a >> b;
        long long center_sum=query({1});
        vector<int> diff(n+1);
        int digits=1;
        while ((1<<(digits+1))-1<=n) ++digits;
        for(int first=2;first<=n;first+=digits){
            int len=min(digits,n-first+1);
            vector<int> terms;
            for(int j=0;j<len;++j)terms.insert(terms.end(),1<<j,first+j);
            int ones=(query(terms)-center_sum*(int)terms.size()+(int)terms.size())/2;
            for(int j=0;j<len;++j)diff[first+j]=(ones>>j&1)?1:-1;
        }
        vector<int> answer=diff;
        if (center_sum%2) {
            answer[1]=int(center_sum);
        } else {
            cout << "? 2 1" << endl;
            int low=2,high=n;
            while (low<high) {
                int amount=high-low+1, width=1;
                auto weight=[&](int w) {
                    int full=amount/w,last=amount%w;
                    return 1LL*w*full*(full-1)/2+1LL*full*last;
                };
                while(weight(width)>n)++width;
                vector<int> terms;
                long long before=0;
                for(int i=0;i<amount;++i){
                    int w=i/width;
                    terms.insert(terms.end(),w,low+i);
                    before+=w*(center_sum+diff[low+i]);
                }
                int lost=terms.size()-llabs(query(terms)-before)/2;
                int base=low+lost*width;
                high=min(high,base+width-1);low=base;
            }
            answer[low]=int(center_sum)+diff[low];
            answer[1]=diff[low]; // The center has been toggled.
        }
        cout << '!';
        for (int i=1;i<=n;++i) cout << ' ' << answer[i];
        cout << endl;
    }
}

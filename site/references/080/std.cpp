#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <set>
using namespace std;
using I = long long;

template<class Query>
array<I,3> last_bucket(I lo,I hi,I total,bool have_single,I single,Query query){
    vector<array<I,3>> choices;
    for(I a=lo;a<=hi;++a)for(I b=a+1;b<=hi;++b){
        if(have_single){if((a^b)==total)choices.push_back({a,b,single});}
        else for(I c=b+1;c<=hi;++c)if((a^b^c)==total)choices.push_back({a,b,c});
    }
    if(choices.size()==1)return choices.front();
    int t=have_single?2:3;
    for(I a=lo;a<=hi;++a)for(I b=a;b<=hi;++b){
        map<I,int> answers;
        for(int i=0;i<int(choices.size());++i){
            I x=0;for(int j=0;j<t;++j)if(a<=choices[i][j] && choices[i][j]<=b)x^=choices[i][j];
            if(!answers.emplace(x,i).second)break;
        }
        if(answers.size()==choices.size()){
            I x=query(a,b);if(!answers.count(x))abort();return choices[answers[x]];
        }
    }
    abort();
}
template<class Query>
array<I,3> recover(I n, Query query) {
    I top=1;
    while(top<=n) top*=2;
    I lo=0, hi=top-1, total=query(1,n);
    bool have_single=false;
    I single=0;
    while(lo<hi) {
        I low_value=max<I>(1,lo), high_value=min(hi,n);
        if(hi-lo<8)return last_bucket(low_value,high_value,total,have_single,single,query);
        if(have_single && high_value==low_value+1) return {single,low_value,high_value};
        if(!have_single && high_value==low_value+2) return {low_value,low_value+1,high_value};
        I mid=(lo+hi)/2;
        I from=max<I>(1,lo), to=min(mid,n);
        I left=(from<=to ? query(from,to) : 0);
        I right=total^left;
        if(left && right){
            if(have_single) return {single,left,right};
            // All right indices have this bit set; odd right count means one.
            I split_bit=mid-lo+1;
            bool right_single=(right&split_bit)!=0;
            single=right_single?right:left;
            have_single=true;
            if(right_single) {hi=mid;total=left;}
            else {lo=mid+1;total=right;}
        } else if(right) {
            lo=mid+1;total=right;
        } else {
            // If all three xor to zero they cannot all be in the right half.
            hi=mid;total=left;
        }
    }
    abort();
}
#ifdef VERIFY
int main() {
    int checked=0, peak=0;
    for (int n=3;n<=64;++n) for(int a=1;a<n;++a) for(int b=a+1;b<n;++b)
        for(int c=b+1;c<=n;++c) {
            int calls=0;
            auto ans=recover(n,[&](I l,I r){
                if(l<1 || r>n || l>r) abort();
                ++calls;I t=0;for(int v:{a,b,c}) if(l<=v && v<=r)t^=v;return t;
            });
            sort(ans.begin(),ans.end());
            if(ans!=array<I,3>{a,b,c} || calls>max(2,63-__builtin_clzll(n)))abort();
            ++checked;peak=max(calls,peak);
        }
    for(I base:{0LL,8LL})for(int cut=1;cut<=8;++cut)for(int t=2;t<=3;++t){
        I lo=max<I>(1,base),hi=base+cut-1;
        for(I a=lo;a<=hi;++a)for(I b=a+1;b<=hi;++b)
            for(I c=(t==3?b+1:0);c<=(t==3?hi:0);++c){
                auto ans=last_bucket(lo,hi,a^b^c,t==2,0,[&](I l,I r){I x=0;for(I v:{a,b,c})if(l<=v&&v<=r)x^=v;return x;});
                sort(ans.begin(),ans.end());array<I,3>want{a,b,c};sort(want.begin(),want.end());if(ans!=want)abort();
            }
    }
    cout<<checked<<" triples and every terminal XOR class, max "<<peak<<'\n';
}
#else
int main() {
    {
        I n;cin>>n;
        auto ans=recover(n,[](I l,I r){
            cout<<"xor "<<l<<" "<<r<<endl;
            I x;if(!(cin>>x)||x<0)exit(0);return x;
        });
        cout<<"ans "<<ans[0]<<' '<<ans[1]<<' '<<ans[2]<<endl;
    }
}
#endif

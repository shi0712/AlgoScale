#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <tuple>
using namespace std;
using I=long long;
constexpr I limit=100000000000000LL;
bool ask(I x) {
    cout<<"? "<<x<<endl;
    string s;if(!(cin>>s)||(s!="Lucky!"&&s!="Fraudster!"))exit(0);
    return s=="Lucky!";
}
// Units of capital. A successful comparison gains at least gain units;
// an unsuccessful comparison costs at most loss units.
I capacity(int steps,int money,int gain,int loss) {
    if(steps==0)return 1;
    if(money>=steps*loss)return min(limit,1LL<<min(steps,47));
    static map<tuple<int,int,int,int>,I> memo;
    auto key=make_tuple(steps,money,gain,loss);
    auto found=memo.find(key);if(found!=memo.end())return found->second;
    I yes=capacity(steps-1,money+gain,gain,loss);
    I no=money<loss?0:capacity(steps-1,money-loss,gain,loss);
    return memo[key]=min(limit,yes+no);
}
I solve() {
    I base=0,cash=1;
    for(I x=1;x<=limit;x*=2) {
        if(!ask(x)) {cash-=x;break;}
        cash+=x;base=x;
    }
    if(!base)return 0;
    I lo=base,hi=min(limit,2*base-1);
    if(base<8) {
        for(I x=base+1;x<=hi;++x) {
            while(cash<x) {ask(lo);cash+=lo;}
            if(!ask(x))break;
            cash+=x;lo=x;
        }
        return lo;
    }
    I unit=base/8, pivot=11*unit;
    while(cash<pivot) {ask(base);cash+=base;}
    if(ask(pivot)) {lo=pivot;cash+=pivot;}
    else {hi=pivot-1;cash-=pivot;}
    int gain=int(lo/unit),loss=int((hi+unit-1)/unit),money=int(cash/unit);
    int steps=0;
    while(capacity(steps,money,gain,loss)<hi-lo+1)++steps;
    while(lo<hi) {
        --steps;
        if(money<loss) {
            ask(lo);money+=gain;
            continue;
        }
        I below=min(hi-lo,capacity(steps,money-loss,gain,loss));
        I x=lo+below;
        if(ask(x)) {lo=x;money+=gain;}
        else {hi=x-1;money-=loss;}
    }
    return lo;
}
int main(){
    {I m=solve();cout<<"! "<<m<<endl;}
}

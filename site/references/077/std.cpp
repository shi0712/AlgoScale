#include <bits/stdc++.h>
#include <vector>

int count_mushrooms(int n);

int use_machine(std::vector<int> x);



#include <algorithm>
#include <array>
#include <vector>
namespace mush_policy {
struct Option {int signed_rows,depth,calls,digits,major,minor,items;};
constexpr Option options[]={
{0,-1,1,0,1,0,2}, // identify one directly
{0,0,1,1,2,0,4},{0,1,2,2,2,0,8},{0,2,4,5,4,0,26},{0,3,8,13,8,0,104},
{0,4,16,33,20,0,440},{0,5,32,81,48,0,1984},{0,6,64,193,112,0,9248},
{1,1,2,3,3,2,15},{1,2,4,8,5,4,55},{1,3,8,20,9,8,207},
{1,4,16,48,17,16,799},{1,5,32,112,33,32,3135},{1,6,64,256,65,64,12415}};
constexpr int prefix_limit=650,largest_n=20000;
using Values=std::vector<std::array<int,prefix_limit+1>>;
struct Policy {
    Values choice,calls,items;
    Policy():choice(prefix_limit+1),calls(prefix_limit+1),items(prefix_limit+1) {}
};
inline Policy make_policy() {
    Policy s;
    for(int k=prefix_limit;k>=1;k--)for(int a=0;a<=k;a++) {
        int majority=std::max(a,k-a),minority=k-majority;
        int seen=k,turns=0;
        while(seen<largest_n) {
            seen+=std::max(majority,(k+turns+1)/2);turns++;
        }
        s.calls[k][a]=turns;s.items[k][a]=2*(largest_n-k);s.choice[k][a]=-1;
        for(int j=0;j<int(sizeof(options)/sizeof(options[0]));j++) {
            auto o=options[j];int gain=o.calls+o.digits;
            if(majority<o.major || minority<o.minor || k+gain>prefix_limit)continue;
            int cost=o.calls+*std::max_element(s.calls[k+gain].begin()+a,s.calls[k+gain].begin()+a+gain+1);
            int items=o.items+*std::max_element(s.items[k+gain].begin()+a,s.items[k+gain].begin()+a+gain+1);
            if(items<=100000 && (cost<s.calls[k][a] || (cost==s.calls[k][a] && items<s.items[k][a]))) {
                s.choice[k][a]=j;s.calls[k][a]=cost;s.items[k][a]=items;
            }
        }
    }
    return s;
}
}

#include <cassert>
#include <numeric>
using namespace std;
using Matrix=vector<vector<int>>;
using namespace mush_policy;

static Matrix matrix(bool signed_rows,int depth) {
    Matrix rows{{1}};
    for(int level=0;level<depth;level++) {
        int q=rows.size(),b=rows.front().size();
        Matrix next;
        if(!signed_rows) {
            vector<int> right(2*b+q-1);
            fill(right.begin()+b,right.begin()+2*b,1);next.push_back(right);
        }
        for(int i=0;i<q-int(!signed_rows);i++) {
            vector<int> sum(2*b+q-int(!signed_rows)),diff(sum);
            for(int j=0;j<b;j++) {
                sum[j]=sum[b+j]=diff[j]=rows[i][j];
                diff[b+j]=signed_rows?-rows[i][j]:1-rows[i][j];
            }
            diff[2*b+i]=1;next.push_back(move(sum));next.push_back(move(diff));
        }
        if(!signed_rows)next.emplace_back(2*b+q-1,1);
        rows=move(next);
    }
    if(!signed_rows && rows.size()>=2)
        for(size_t i=0;i<rows.back().size();i++)rows.back()[i]^=rows[rows.size()-2][i];
    return rows;
}
static vector<int> decode(bool signed_rows,vector<int> sums) {
    if(sums.size()==1)return sums;
    int q=sums.size()/2;
    vector<int> left(q),right(q),extra(q-int(!signed_rows));
    for(int i=0;i<int(extra.size());i++) {
        int j=2*i+int(!signed_rows);
        int both=sums[j]+sums[j+1]-(signed_rows?0:sums[0]);
        extra[i]=both&1;left[i]=(both-extra[i])/2;right[i]=sums[j]-left[i];
    }
    if(!signed_rows) {
        right.back()=sums[0];
        left.back()=sums.back()-right.back()-accumulate(extra.begin(),extra.end(),0);
    }
    auto a=decode(signed_rows,left),b=decode(signed_rows,right);
    a.insert(a.end(),b.begin(),b.end());a.insert(a.end(),extra.begin(),extra.end());
    return a;
}
static int counted_tail(vector<int> known[2],int next,int n) {
    int answer=known[0].size();
    while(next<n) {
        int base=known[1].size()>known[0].size();
        int m=min(n-next,int(known[base].size()));
        vector<int> query;
        for(int j=0;j<m;j++) {query.push_back(known[base][j]);query.push_back(next+j);}
        int r=use_machine(query);
        answer+=base?(r+1)/2:m-(r+1)/2;
        known[base^(r&1)].push_back(next+m-1);next+=m;
    }
    return answer;
}
int count_mushrooms(int n) {
    vector<int> known[2];known[0]={0};
    if(n==3)return 3-use_machine({1,0,2});
    if(n<=prefix_limit)return counted_tail(known,1,n);
    static const Policy policy=make_policy();
    static const vector<Matrix> matrices=[] {
        vector<Matrix> all;
        for(auto o:options)all.push_back(o.depth<0?Matrix{}:matrix(o.signed_rows,o.depth));
        return all;
    }();
    int next=1;
    while(true) {
        int j=policy.choice[next][known[0].size()];
        if(j<0)break;
        const Option &o=options[j];
        if(o.depth<0) {int r=use_machine({0,next});known[r].push_back(next++);continue;}
        const Matrix &rows=matrices[j];
        int start=next+o.calls;
        vector<int> sums;
        for(int i=0;i<o.calls;i++) {
            const auto &row=rows[i];
            int pos=count(row.begin(),row.end(),1),neg=count(row.begin(),row.end(),-1);
            int majority=known[1].size()>known[0].size();
            int positive_species=majority^(neg>pos),last=0;
            vector<int> query;
            for(int sign:{1,-1}) {
                if((sign==1?pos:neg)==0)continue;
                last=positive_species^(sign==-1);
                int used=0;query.push_back(known[last][used++]);
                for(int v=0;v<int(row.size());v++)if(row[v]==sign) {
                    query.push_back(start+v);query.push_back(known[last][used++]);
                }
            }
            query.push_back(next+i);
            int r=use_machine(query)-int(pos>0 && neg>0);
            known[last^(r&1)].push_back(next+i);
            sums.push_back(positive_species?pos-r/2:r/2-neg);
        }
        if(!o.signed_rows && o.calls>1)sums.back()+=sums[o.calls-2];
        auto bits=decode(o.signed_rows,sums);
        for(int i=0;i<o.digits;i++) {assert(bits[i]==0 || bits[i]==1);known[bits[i]].push_back(start+i);}
        next=start+o.digits;
    }
    return counted_tail(known,next,n);
}

int use_machine(std::vector<int> ids){
    std::cout<<"? "<<ids.size();
    for(int v:ids)std::cout<<' '<<v;
    std::cout<<std::endl;int result;
    if(!(std::cin>>result)||result<0)std::exit(0);
    return result;
}
int main(){
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    int n;if(!(std::cin>>n))return 0;
    int result=count_mushrooms(n);std::cout<<"! "<<result<<std::endl;
}

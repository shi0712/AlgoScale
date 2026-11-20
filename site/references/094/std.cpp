#include <array>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

int queries;
int ask(int version, int edge) {
    cout << "? " << version << ' ' << edge << endl;
    int reply;
    if (!(cin >> reply) || reply < 0) exit(0);
    ++queries;
    return reply;
}

// Rank 0..14 gets its own deletion batch, rank 15 is an extra edge of
// a large cycle, rank 16 is a bridge.
constexpr array<int,16> preorder{3,1,0,2,7,5,4,6,9,8,11,10,13,12,15,14};
int rankPivot[17][17];
void readRankTree(int lo, int hi, int& next) {
    if (lo == hi) return;
    int p = rankPivot[lo][hi] = preorder[next++];
    readRankTree(lo,p,next);
    readRankTree(p+1,hi,next);
}

struct LengthTree {
    array<int,15> values{};
    int size=0, pivot[15][15]{};
    LengthTree(const array<int,16>& frequencies, int minimum) {
        int cost[15][15]{}, prefix[16]{};
        for (int len=minimum;len<=15;++len) if(frequencies[len]>0) {
            values[size]=len;
            prefix[size+1]=prefix[size]+frequencies[len];
            ++size;
        }
        for (int width=2;width<=size;++width) {
            for (int l=0,r=width-1;r<size;++l,++r) {
                int best=10000000;
                for (int p=l;p<r;++p) {
                    int candidate=cost[l][p]+cost[p+1][r];
                    if(candidate<best) {best=candidate; pivot[l][r]=p;}
                }
                cost[l][r]=best+prefix[r+1]-prefix[l];
            }
        }
    }
};
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int next=0;
    readRankTree(0,16,next);
    {
        int m; cin >> m; queries=0;
        array<int,16> base{}; // base[15] permanently denotes the original graph.
        array<int,17> ranks{};
        vector<int> rank(m+1),answer(m+1);
        for (int e=1;e<=m;++e) {
            int lo=0,hi=16,success=0;
            while (lo<hi) {
                int p=rankPivot[lo][hi];
                if (ask(base[p],e)) {hi=p;success=queries;}
                else lo=p+1;
            }
            rank[e]=lo;++ranks[lo];
            if (lo<15) base[lo]=success;
        }
        array<int,16> count{};
        for(int len=1;len<15;++len) count[len]=ranks[len-1]-ranks[len];
        count[15]=ranks[14];
        // Each rank b of a cycle of length >=b+1 occurs exactly once.
        // Its histogram is the common suffix of count. The interval DP
        // therefore supplies the best ordered comparison tree for all rows.
        array<array<int,16>,14> remaining;
        for (auto& row:remaining) row=count;
        for(int e=1;e<=m;++e) {
            int b=rank[e];
            if (b==16) {answer[e]=-1;continue;}
            if (b>=14) {answer[e]=0;continue;}
            auto& frequencies=remaining[b];
            LengthTree tree(frequencies,b+1);
            int lo=0,hi=tree.size-1;
            while (lo<hi) {
                int p=tree.pivot[lo][hi];
                if (ask(base[tree.values[p]],e)) hi=p;
                else lo=p+1;
            }
            int length=tree.values[lo];
            --frequencies[length];
            answer[e]=length==15?0:length;
        }
        cout<<"!";
        for(int e=1;e<=m;++e) cout<<' '<<answer[e];
        cout<<endl;
        int verdict;
        if (!(cin>>verdict) || verdict<0) return 0;
    }
}

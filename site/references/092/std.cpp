#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

// Hidden values are internally 0..n-1. Subtracting 1 preserves the
// integer-average predicate, as does reflecting every value in n-1.
struct Group {
    vector<int> positions;
    int child[2] = {-1, -1};
    int zero_child = -1;
};
vector<Group> groups;
int n;


bool ask(const vector<int>& subset) {
    cout << "? " << subset.size();
    for (int i : subset) cout << ' ' << i;
    cout << endl;
    int reply;
    if (!(cin >> reply) || reply < 0) exit(0);
    return reply != 0;
}

// Every other residue class contributes its fixed first pair.
bool ask_pair(const vector<int>& level, int at, int a, int b) {
    vector<int> subset;
    for (int node : level) {
        if (node == at) {
            subset.push_back(a);
            subset.push_back(b);
        } else {
            subset.push_back(groups[node].positions[0]);
            subset.push_back(groups[node].positions[1]);
        }
    }
    return ask(subset);
}

void attach(int node, vector<int> left, vector<int> right) {
    if (left.size() != right.size()) groups[node].zero_child = left.size() < right.size();
    groups[node].child[0] = (int)groups.size();
    groups.push_back({move(left)});
    groups[node].child[1] = (int)groups.size();
    groups.push_back({move(right)});
}

#include <bit>
#include <cstdint>
#include <map>
struct BlockPlan {
    int width, size, num_states, words;
    vector<int> masks;
    vector<pair<int,int>> pairs;
    vector<vector<int>> choices;
    vector<vector<uint64_t>> ones;

    static vector<int> digits(int x, int base, int count) {
        vector<int> v(count);
        for (int i=count-1; i>=0; --i) { v[i]=x%base; x/=base; }
        return v;
    }

    BlockPlan(int width, int size) : width(width), size(size) {
        for (int m=0;m<(1<<width);++m) if (popcount((unsigned)m)==width/2) masks.push_back(m);
        for (int i=0;i<width;++i) for (int j=i+1;j<width;++j) pairs.push_back({i,j});
        int action_count=1;
        num_states=1;
        for (int i=0;i<size;++i) { num_states*=masks.size(); action_count*=pairs.size(); }
        words=(num_states+63)/64;
        vector<vector<int>> states;
        for (int i=0;i<num_states;++i) states.push_back(digits(i,masks.size(),size));
        for (int i=1;i<action_count;++i) {
            choices.push_back(digits(i,pairs.size(),size));
            vector<uint64_t> yes(words);
            for (int t=0;t<num_states;++t) {
                int parity=0;
                for (int j=0;j<size;++j) {
                    int m=masks[states[t][j]];
                    auto [a,b]=pairs[choices.back()[j]];
                    parity ^= (m^(m>>1)^(m>>a)^(m>>b))&1;
                }
                if (parity) yes[t/64]|=uint64_t(1)<<(t%64);
            }
            ones.push_back(move(yes));
        }
    }
};

const int width_list[]={3,5,7};
int block_size(int width) { return width==3?5:width==5?3:2; }

int block_depth(int width,int k) {
    static int depth3[]={0,2,4,5,7,8};
    static int depth5[]={0,4,7,10};
    static int depth7[]={0,6,11};
    return width==3?depth3[k]:width==5?depth5[k]:depth7[k];
}

int class_width(int group_size) { return group_size%2?group_size:group_size-1; }

void split_block(const vector<int>& level,const vector<int>& batch,int width,bool shared) {
    static map<pair<int,int>,BlockPlan> cache;
    int size=batch.size();
    auto [it,fresh]=cache.try_emplace(make_pair(width,size),width,size);
    const BlockPlan& plan=it->second;
    vector<uint64_t> alive(plan.words,~uint64_t(0));
    if (plan.num_states%64) alive.back()=(uint64_t(1)<<(plan.num_states%64))-1;
    int count=plan.num_states;
    while (count>1) {
        // Minimize the larger surviving branch. Keep the first lexicographic
        // choice on ties, as used in the exhaustive finite certificate.
        int score=count,best=-1,positive=0;
        for (int i=0;i<(int)plan.ones.size();++i) {
            int q=0;
            for (int j=0;j<plan.words;++j) q+=popcount(alive[j]&plan.ones[i][j]);
            int worst=max(q,count-q);
            if (worst<score) { score=worst;best=i;positive=q; }
        }
        if (best<0) exit(1);
        vector<int> selections(level.size());
        for (int i=0;i<size;++i) selections[batch[i]]=plan.choices[best][i];
        vector<int> subset;
        for (int i=0;i<(int)level.size();++i) {
            const auto& v=groups[level[i]].positions;
            if (selections[i]==0) { subset.push_back(v[0]);subset.push_back(v[1]); }
            else {
                auto [a,b]=plan.pairs[selections[i]];
                subset.push_back(v[a]);subset.push_back(v[b]);
            }
        }
        bool yes=ask(subset)!=shared;
        for (int j=0;j<plan.words;++j) alive[j]&=yes?plan.ones[best][j]:~plan.ones[best][j];
        count=yes?positive:count-positive;
    }
    int word=0;
    while (!alive[word]) ++word;
    int state=word*64+countr_zero(alive[word]);
    auto decoded=BlockPlan::digits(state,plan.masks.size(),size);
    for (int i=0;i<size;++i) {
        int node=level[batch[i]],mask=plan.masks[decoded[i]];
        const auto& v=groups[node].positions;
        vector<int> small,large;
        for (int j=0;j<width;++j) ((mask>>j)&1?small:large).push_back(v[j]);
        if ((int)v.size()>width) small.push_back(v[width]);
        attach(node,move(small),move(large));
    }
}

bool use_blocks(const vector<int>& level,bool shared) {
    int conventional=0,combined=1;
    map<int,vector<int>> by_width;
    for (int i=0;i<(int)level.size();++i) {
        int s=groups[level[i]].positions.size();
        if (s>8) return false;
        if (s>2) { by_width[class_width(s)].push_back(i);conventional+=s-2+s%2; }
    }
    for (int w:width_list) {
        int g=block_size(w), m=by_width[w].size();
        combined+=(m/g)*block_depth(w,g)+block_depth(w,m%g);
    }
    if (combined>=conventional) return false;
    for (int w:width_list) {
        auto& v=by_width[w];int g=block_size(w);
        for (int j=0;j<(int)v.size();j+=g) {
            vector<int> batch(v.begin()+j,v.begin()+min(j+g,(int)v.size()));
            split_block(level,batch,w,shared);
        }
    }
    for (int u:level) if (groups[u].positions.size()==2) {
        // Copy the two positions before attach may resize groups.
        int a=groups[u].positions[0],b=groups[u].positions[1];
        attach(u,{a},{b});
    }
    return true;
}

void build_tree() {
    vector<int> all(n);
    iota(all.begin(), all.end(), 1);
    groups.push_back({move(all)});
    vector<int> level{0};
    while (true) {
        bool needs_queries = any_of(level.begin(), level.end(), [](int node) {
            return groups[node].positions.size() > 2;
        });
        bool shared = false;
        if (needs_queries) {
            const auto& reference = groups[level[0]].positions;
            shared = ask_pair(level, level[0], reference[0], reference[1]);
        }
        if (needs_queries && use_blocks(level,shared)) {
            vector<int> next;
            for (int node:level) {
                next.push_back(groups[node].child[0]);
                next.push_back(groups[node].child[1]);
            }
            level=move(next);
            continue;
        }
        int last_large = -1;
        if (needs_queries) for (int node : level)
            if (groups[node].positions.size() > 2) last_large = node;
        int parity_xor = ((int(level.size())-1)&1) ^ !shared;
        // Two-element groups certainly have opposite next bits, including any
        // such groups occurring after last_large in the iteration order.
        if (needs_queries) for (int node : level)
            if (groups[node].positions.size()==2) parity_xor ^= 1;
        vector<int> next;
        for (int node : level) {
            // attach() may reallocate groups, so keep the position list local.
            vector<int> v = groups[node].positions;
            if (v.size() == 1) continue;
            if (v.size() == 2) {
                attach(node, {v[0]}, {v[1]});
            } else if (node==last_large) {
                vector<int> parts[2]; // relative to v[0]
                parts[0].push_back(v[0]);
                parts[parity_xor].push_back(v[1]);
                for (int j=2; j<(int)v.size(); ++j) {
                    int part;
                    if (v.size()%2==0 && 2*parts[0].size()==v.size()) part=1;
                    else if (v.size()%2==0 && 2*parts[1].size()==v.size()) part=0;
                    else {
                        bool reply=ask_pair(level,node,v[0],v[j]);
                        part=parity_xor ^ (reply!=shared);
                    }
                    parts[part].push_back(v[j]);
                }
                attach(node,move(parts[0]),move(parts[1]));
            } else {
                vector<int> parts[2];
                int pivot = v[0];
                for (int j = 1; j < (int)v.size(); ++j) {
                    bool reply = j == 1 ? shared : ask_pair(level, node, pivot, v[j]);
                    parts[reply].push_back(v[j]);
                }
                int pivot_part;
                if (parts[0].size() != parts[1].size())
                    pivot_part = parts[0].size() > parts[1].size();
                else
                    // Mixed-pair answer equals the answer for pivot paired
                    // with a member of the parity class opposite to pivot.
                    pivot_part = !ask_pair(level, node, parts[0][0], parts[1][0]);
                parity_xor ^= (pivot_part != (int)shared);
                parts[pivot_part].push_back(pivot);
                attach(node, move(parts[0]), move(parts[1]));
            }
            next.push_back(groups[node].child[0]);
            next.push_back(groups[node].child[1]);
        }
        if (!needs_queries) break;
        level = move(next);
    }
    groups[0].zero_child = 0; // choose between a permutation and its reflection
}

vector<int> reconstruct() {
    vector<int> assigned(n+1), by_residue{0};
    for (int d=1; !by_residue.empty(); d*=2) {
        vector<int> next(2*d,-1);
        bool split=false;
        for (int r=0;r<d;++r) {
            int node=by_residue[r];
            if (node<0) continue;
            auto& g=groups[node];
            int size=g.positions.size();
            if (size==1) { assigned[g.positions[0]]=r; continue; }
            if (g.zero_child<0) {
                int partner=-1;
                for (int u=0; u<d; ++u) {
                    if (by_residue[u]>=0 && u%3!=r%3 &&
                        groups[by_residue[u]].positions.size()==g.positions.size()) {
                        partner=u;break;
                    }
                }
                if (partner<0) exit(1);
                int h=size/2;
                vector<int> subset=groups[g.child[0]].positions;
                const auto& pad=groups[by_residue[partner]].positions;
                subset.insert(subset.end(),pad.begin(),pad.end());
                bool reply=ask(subset);
                int sum_if_zero=h*(r+d*(h-1))+2*h*partner+d*h*(2*h-1);
                g.zero_child=reply!=(sum_if_zero%(3*h)==0);
            }
            split=true;
            next[r]=g.child[g.zero_child];
            next[r+d]=g.child[g.zero_child^1];
        }
        if (!split) break;
        by_residue=move(next);
    }
    return assigned;
}

int main() {
    if (!(cin >> n)) return 0;
    build_tree();
    vector<int> result=reconstruct();
    if (result[1]>=n/2) for (int i=1; i<=n; ++i) result[i]=n-1-result[i];
    cout << '!';
    for (int i=1; i<=n; ++i) cout << ' ' << result[i]+1;
    cout << endl;
}

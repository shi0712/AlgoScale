#include <bits/stdc++.h>
using namespace std;

namespace {
constexpr int N = 2000, W = (N + 63) / 64;
using Word = uint64_t;
int n, m;                         // m positions have been ordered
int pos[N], rk[N];                 // mutually inverse prefix orderings
signed char inv[N][N];
vector<int> asked[N];
Word sig[N+1][W], alive[W];
int words, queryCount, costSoFar[N+1];
unsigned state = 127829;
unsigned randomWord() {
    state ^= state << 13;
    state ^= state >> 17;
    return state ^= state << 5;
}

int parity(int l, int r) {
    if (l >= r) return 0;
    if (inv[l][r] == -1) {
        printf("? %d %d\n",l+1,r+1);
        fflush(stdout);
        int bit;
        if (scanf("%d",&bit)!=1 || (bit != 0 && bit != 1)) exit(0);
        inv[l][r] = bit;
        ++queryCount;
        asked[r].push_back(l);
    }
    return inv[l][r];
}

// A mixed finite difference of four interval parities gives one comparison.
bool greaterAt(int l, int r) {
    return parity(l,r) ^ parity(l+1,r) ^ parity(l,r-1) ^ parity(l+1,r-1);
}

constexpr int K = 14, D = 20, B = 2*D+1;
constexpr int lags[] = {0,0,1,2,3,4,8,16};
int lastLag[2] = {1,1};

int anchor(int k) {
    if (k==0) return 0;
    if (k==1) return m;
    if (k<8) return lags[k]<=m ? rk[m-lags[k]] : -N;
    if (k==8 || k==9) {
        if (m<2) return -N;
        int adjacent = (rk[m-1]+m+(k==8 ? -1 : 1))%m;
        // Progress the value-neighbor's position by one time step.
        return rk[pos[adjacent]+1];
    }
    if (k==10) return rk[m-bit_floor((unsigned)m)];
    if (k==11 || k==12) return rk[m-lastLag[k-11]];
    return -N;
}
struct Model {
    double bin[2][B]{}, total[2]{};
    double score = -2, base = 1, norm = 1;
    int center;
} model[K];
double extra[N], flat;
vector<int> heavy;

// These distributions only affect query choice. All gaps remain feasible
// until the parity of an actual interval rules them out.
void forecast() {
    double mix[K], total = 0;
    for (int k=0; k<K; ++k) {
        mix[k] = exp(model[k].score);
        total += mix[k];
    }
    fill(extra, extra+m+1, 0.0);
    int phase = m & 1;
    for (int k=0; k<K; ++k) {
        Model &a = model[k];
        a.center = anchor(k);
        a.norm = a.total[phase] + 0.3*a.total[phase^1] + 0.8;
        double mass = mix[k]/total, used = 0;
        for (int d=max(-D,-a.center); d<=min(D,m-a.center); ++d) {
            double h = (a.bin[phase][d+D]+0.3*a.bin[phase^1][d+D])/a.norm;
            extra[a.center+d] += mass*h;
            used += h;
        }
        a.base = (1-used)/(m+1);
    }
    heavy.clear();
    for (int g=0; g<=m; ++g)
        if (extra[g] > 0.0002) heavy.push_back(g);
    constexpr int keep = 28;
    if ((int)heavy.size() > keep) {
        nth_element(heavy.begin(),heavy.begin()+keep,heavy.end(),
                    [](int a,int b){ return extra[a]>extra[b]; });
        heavy.resize(keep);
    }
    double retained = 0;
    for (int g:heavy) retained += extra[g];
    flat = (1-retained)/(m+1);
}

void place(int start,int m,int g,int *rk,int *pos) {
    int cross = 0;
    for (int i=m-1; i>=0; --i) {
        if (rk[i]>=g) {
            cross ^= 1;
            ++rk[i];
        }
        inv[start+i][start+m] = inv[start+i][start+m-1]^cross;
    }
    move_backward(pos+g,pos+m,pos+m+1);
    pos[g] = m;
    rk[m] = g;
}

int streak = 0, direction = 0;
void append(int g) {
    int phase = m&1;
    double best = -1e30;
    for (int k=0; k<K; ++k) {
        Model &a = model[k];
        int d = g-a.center;
        double p = a.base;
        if (abs(d)<=D)
            p += (a.bin[phase][d+D]+0.3*a.bin[phase^1][d+D])/a.norm;
        a.score += 0.85*log(max(1e-12,p*(m+1)));
        best = max(best,a.score);
        if (a.center<0) continue;
        for (double &v:a.bin[phase]) v *= 0.94;
        if (abs(d)<=D) a.bin[phase][d+D] += 1;
        a.total[phase] = 0.94*a.total[phase]+1;
    }
    for (int k=0; k<K; ++k)
        model[k].score = max(-6.5,0.96*(model[k].score-best)+(k==K-1 ? 0.0 : -0.08));

    int dir = g==rk[m-1] ? -1 : g==rk[m-1]+1 ? 1 : 0;
    streak = dir && dir==direction ? streak+1 : (dir!=0);
    direction = dir;

    if (g>0) lastLag[0]=m-pos[g-1];
    if (g<m) lastLag[1]=m-pos[g];
    place(0,m,g,rk,pos);
    costSoFar[m+1]=queryCount;
    ++m;
}

// Bit g in row l is the parity contributed by a new item in gap g
// to suffix [l,m]. Extending the suffix flips gaps 0 through rk[l].
void makeSignatures(int m,const int *rk) {
    words = (m+64)/64;
    fill_n(sig[m],words,0);
    for (int l=m-1; l>=0; --l) {
        int k = rk[l], w = k/64;
        for (int j=0; j<w; ++j) sig[l][j] = ~sig[l+1][j];
        sig[l][w] = sig[l+1][w] ^ (~Word(0) >> (63-(k%64)));
        copy(sig[l+1]+w+1, sig[l+1]+words, sig[l]+w+1);
    }
    fill_n(alive,words,~Word(0));
    alive[words-1] >>= 63-(m%64);
}
bool bit(const Word *s,int g) { return (s[g/64]>>(g%64))&1; }

int filter(int l,int v) {
    int cnt = 0;
    for (int w=0; w<words; ++w) {
        alive[w] &= v ? sig[l][w] : ~sig[l][w];
        cnt += __builtin_popcountll(alive[w]);
    }
    return cnt;
}

int solveGap(int start,int m,const int *rk,const int *pos,vector<int> heavy,
             double flat,int lo,int hi) {
    if (lo==hi) return lo;
    makeSignatures(m,rk);
    fill(alive,alive+lo/64,0);
    alive[lo/64] &= ~Word(0)<<(lo%64);
    fill(alive+hi/64+1,alive+words,0);
    alive[hi/64] &= ~Word(0)>>(63-hi%64);
    int cnt = hi-lo+1;
    auto use=[&](int l,int reply) {
        return filter(l,reply ^ inv[start+l][start+m-1]);
    };
    for (int l:asked[start+m]) if (l>=start) cnt=use(l-start,inv[l][start+m]);
    int initial=cnt, rounds=0;
    bool uniformOnly=false;
    while (cnt>1) {
        // Switch to cardinality splits if the predictor spends too many
        // questions without appreciably reducing the set of possible ranks.
        if (rounds>5+(int)bit_width((unsigned)(initial/cnt))) uniformOnly=true;
        ++rounds;
        double weight = cnt*flat, topWeight = 0;
        int top = -1;
        for (int g:heavy) if (bit(alive,g)) {
            weight += extra[g];
            if (extra[g]>topWeight) { topWeight = extra[g]; top = g; }
        }
        // Ignore small fluctuations when data favor the uniform model.
        bool weighted = !uniformOnly && weight-cnt*flat > 0.025*weight;
        if (uniformOnly) top=-1;
        double bestDiff = 1e100;
        int best = -1, topSide = cnt;
        auto evaluate = [&](int l) {
            if (l<0 || l>=m) return;
            int yes = 0;
            for (int w=0; w<words; ++w)
                yes += __builtin_popcountll(alive[w]&sig[l][w]);
            if (yes==0 || yes==cnt) return;
            double diff;
            if (weighted) {
                double mass = yes*flat;
                for (int g:heavy) if (bit(sig[l],g)) mass += extra[g];
                diff = abs(weight-2*mass);
            } else diff = abs(cnt-2*yes);
            int side = top<0 || bit(sig[l],top) ? yes : cnt-yes;
            if (diff<bestDiff || (diff==bestDiff && side<topSide)) {
                bestDiff=diff; best=l; topSide=side;
            }
        };
        // Compact the sparse probability distribution to the current support.
        heavy.erase(remove_if(heavy.begin(),heavy.end(),
                              [](int g){return !bit(alive,g);}),heavy.end());
        auto perfect = [&] { return !weighted && bestDiff<=cnt%2; };
        evaluate(0);
        if (!perfect()) evaluate(m-1);
        int bounds[4], nb = 0;
        if (top>=0) {
            for (int k=max(0,top-1); k<=min(m-1,top); ++k) {
                int p = pos[k];
                bounds[nb++]=p;
                if (p+1<m) bounds[nb++]=p+1;
            }
            for (int i=0; i<nb && !perfect(); ++i) evaluate(bounds[i]);
        }
        for (int i=2; i<=7 && i<=m && !perfect(); ++i) evaluate(m-i);
        if (!perfect()) {
            // Neighboring suffix rows at the median pivot XOR to a threshold
            // comparison. At least one separates a quarter of the candidates.
            int i=(cnt-1)/2, w=0;
            while (__builtin_popcountll(alive[w])<=i) {
                i-=__builtin_popcountll(alive[w]);
                ++w;
            }
            Word mask=alive[w];
            while (i--) mask &= mask-1;
            int p=pos[64*w+__builtin_ctzll(mask)];
            evaluate(p);
            evaluate(p+1);
        }
        int trials = min(m, weighted ? 48 : 90);
        for (int i=0; i<trials && !perfect(); ++i)
            evaluate(trials==m ? i : (int)(randomWord()%m));
        if (best==-1)
            for (int l=0; l<m && best==-1; ++l) evaluate(l);

        // For a likely gap its two neighboring values provide a certificate
        // of at most four interval replies. Use a shortest sufficient subset.
        if (top>=0 && (flat+topWeight)>0.55*weight && topSide>2) {
            int shortest = 5, possible = 0;
            for (int s=1; s<(1<<nb); ++s) {
                int len=__builtin_popcount((unsigned)s);
                if (len>shortest) continue;
                bool unique=true;
                for (int w=0; w<words && unique; ++w) {
                    Word v=alive[w];
                    if (w==top/64) v &= ~(Word(1)<<(top%64));
                    for (int j=0; j<nb && v; ++j) if ((s>>j)&1)
                        v &= bit(sig[bounds[j]],top) ? sig[bounds[j]][w] : ~sig[bounds[j]][w];
                    if (v) unique=false;
                }
                if (unique) {
                    if (len<shortest) possible=0;
                    shortest=len; possible|=s;
                }
            }
            if (possible && (shortest<=2 || topSide>(1<<(shortest-1)))) {
                bestDiff=1e100;
                for (int j=0; j<nb; ++j) if ((possible>>j)&1) evaluate(bounds[j]);
            }
        }
        cnt=use(best,parity(start+best,start+m));
    }
    for (int w=0; w<words; ++w)
        if (alive[w]) return w*64+__builtin_ctzll(alive[w]);
    exit(0);
}

// In two alternating increasing sequences, four successive items can have
// order a < c < b < d. Given a < b and c < d, odd parity of (a,b,c)
// forces a < c < b, and odd parity of (b,c,d) then forces b < d.
bool extendAlternating() {
    if (m<8 || m+3>=n) return false;
    bool rev=rk[m-2]<rk[m-4];
    auto lessRank=[&](int a,int b) { return rev ? rk[a]>rk[b] : rk[a]<rk[b]; };
    int firstUpper=(m&1)^1;
    if (!lessRank(m-2,firstUpper) || !lessRank(m-2,m-1)) return false;
    for (int i=m-1;i>=2;--i)
        if (!lessRank(i-2,i)) return false;

    int end=m;
    while (end+1<n && parity(end,end+1)==rev &&
           parity(end-2,end)!=rev && parity(end-1,end+1)!=rev)
        end+=2;
    if (end==m) return false;
    auto stillSeparated=[&](int i) { return greaterAt(firstUpper,i)!=rev; };
    // Compare the last lower-lane entry to the first upper-lane entry.
    int separated=end;
    if (!stillSeparated(end-2)) {
        int lo=-1,hi=(end-m)/2-1;
        while (hi-lo>1) {
            int mid=(lo+hi)/2;
            if (stillSeparated(m+2*mid)) lo=mid;
            else hi=mid;
        }
        separated=m+2*hi;
    }
    while (m<end) {
        forecast();
        int g;
        if (m<separated) g=rk[m-2]+!rev;
        else {
            int a=rk[m-2],b=rk[m-1];
            g=solveGap(0,m,rk,pos,heavy,flat,min(a,b)+1,max(a,b));
        }
        append(g);
        forecast();
        append(rev ? 0 : m);
    }
    return true;
}

// When consecutive position blocks have occupied successive disjoint value
// ranges, order a new block locally, then verify the separating comparison.
constexpr int maxBlock=256;
double blockCost[maxBlock+1];
struct BlockGuess { int len=0, dir=0; };

BlockGuess detectBlock() {
    if (m<6) return {};
    int limit=min({maxBlock,m/2,n-m});
    if (limit<3) return {};
    bool upper[2*maxBlock+1]{}, lower[2*maxBlock+1]{};
    int small=m,large=-1;
    for (int z=1; z<=2*limit; ++z) {
        small=min(small,rk[m-z]);
        large=max(large,rk[m-z]);
        upper[z]=(small==m-z);
        lower[z]=(large==z-1);
    }
    int monotone=1;
    int step=rk[m-1]>rk[m-2] ? 1 : -1;
    while (monotone<limit && (rk[m-monotone]>rk[m-monotone-1])==(step>0))
        ++monotone;
    for (int k=3; k<=limit; ++k) {
        int dir=upper[k] && upper[2*k] ? 1 : lower[k] && lower[2*k] ? -1 : 0;
        if (!dir) continue;
        // Local ordering saves little on a run already extending the prefix.
        if (k<=monotone && step==dir) continue;
        double estimate=k<=monotone ? k-1 : blockCost[k];
        if (costSoFar[m]-costSoFar[m-k]>estimate+4)
            return {k,dir};
    }
    return {};
}

bool processBlock(BlockGuess block) {
    int start=m, len=min(block.len,n-m);
    int ranks[maxBlock]{}, positions[maxBlock]{}, gaps[maxBlock]{};
    int monotone=0;
    for (int t=1; t<len; ++t) {
        int g=-1;
        if (t==1 || monotone) {
            int sign=parity(start+t-1,start+t) ? -1 : 1;
            if (t==1 || sign==monotone) g=sign>0 ? t : 0;
            monotone=g<0 ? 0 : sign;
        }
        if (g<0) g=solveGap(start,t,ranks,positions,{},1.0,0,t);
        gaps[t]=g;
        place(start,t,g,ranks,positions);
    }
    bool above=block.dir>0;
    int boundary=pos[above ? m-1 : 0];
    int nearest=start+positions[above ? 0 : len-1];
    bool separated=greaterAt(boundary,nearest)!=above;
    for (int t=0; t<len; ++t) {
        forecast();
        int g=gaps[t]+(above ? start : 0);
        if (!separated) {
            int lo=0,hi=m;
            for (int j=0; j<t; ++j) {
                if (ranks[j]<ranks[t]) lo=max(lo,rk[start+j]+1);
                else hi=min(hi,rk[start+j]);
            }
            g=solveGap(0,m,rk,pos,heavy,flat,lo,hi);
        }
        append(g);
    }
    return separated;
}
}

int main() {
    static char inputBuffer[64];
    setvbuf(stdin,inputBuffer,_IOFBF,sizeof(inputBuffer));
    if (scanf("%d",&n)!=1 || n<1 || n>N) return 0;
    memset(inv,-1,sizeof(inv));
    for (int i=0;i<n;++i) inv[i][i]=0;
    m=1;
    model[K-1].score=0;
    int skipUntil=0, nextBlockAttempt=0;
    BlockGuess active;
    for (int i=1;i<=maxBlock;++i) blockCost[i]=blockCost[i-1]+log2(i)+0.08;
    while (m<n) {
        // A monotone prefix extends with just one adjacent comparison.
        if (m==1 || streak==m-1) {
            int b=parity(m-1,m);
            if (m==1 || direction==(b ? -1 : 1)) {
                forecast();
                append(b ? 0 : m);
                continue;
            }
        }
        if (extendAlternating()) continue;
        if (m>=nextBlockAttempt) {
            if (!active.len) active=detectBlock();
            if (active.len) {
                if (!processBlock(active)) {
                    nextBlockAttempt=m+active.len;
                    active={};
                }
                continue;
            }
        }
        // A run repeatedly inserted in the same gap can be verified in bulk:
        // its near end is bounded by adjacency, its far end by one comparison.
        if (streak>=4 && m>=skipUntil) {
            int dir=direction;
            int end=m;
            while (end<n && parity(end-1,end)==(dir<0)) ++end;
            skipUntil=end+1;
            int k=rk[m-1]+dir;
            bool outside=k<0 || k>=m;
            int limit=m;
            if (outside) limit=end;
            else if (end-m>=4) {
                auto fits=[&](int z){return greaterAt(pos[k],z)==(dir>0);};
                if (fits(end-1)) limit=end;
                else if (end-m>=12 && fits(m)) {
                    int lo=m,hi=end-1;
                    while (hi-lo>1) {
                        int mid=(lo+hi)/2;
                        if (fits(mid)) lo=mid;
                        else hi=mid;
                    }
                    limit=lo+1;
                }
            }
            int first=m;
            while (m<limit) {
                forecast();
                append(rk[m-1]+(dir>0));
            }
            if (m>first) {
                if (m<end) skipUntil=m+1;
                continue;
            }
        }
        forecast();
        int g=solveGap(0,m,rk,pos,heavy,flat,0,m);
        append(g);
    }
    putchar('!');
    for (int i=0;i<n;++i) printf(" %d",rk[i]+1);
    putchar('\n');
    fflush(stdout);
}

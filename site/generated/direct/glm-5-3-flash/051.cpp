#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const int P = 1000000007;
static const ll MOD = 1000000007LL;
#pragma GCC optimize("O3","unroll-loops")

static char ibuf[1 << 23], obuf[1 << 23];

int main() {
    size_t len = fread(ibuf, 1, sizeof(ibuf) - 1, stdin);
    ibuf[len] = '\0';
    char* p = ibuf;
    auto readInt = [&p]() -> ll {
        while (*p && (*p < '0' || *p > '9')) ++p;
        ll v = 0;
        while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); ++p; }
        return v;
    };
    int n = (int)readInt(), Q = (int)readInt();
    while (*p==' '||*p=='\n'||*p=='\r'||*p=='\t') ++p;
    const char* s = p;
    int m = 0;
    for (int i = 0; i < n; ++i) if (s[i] == '?') ++m;
    const int W = m + 1, sz = W * W;
    vector<int> Fa(sz,0), Fb(sz,0), Fc(sz,0), nFa(sz,0), nFb(sz,0), nFc(sz,0);
    int q = 0;
    {   char c0 = s[0];
        if (c0=='?') { q=1; Fa[W]=1; Fb[1]=1; Fc[0]=1; }
        else if (c0=='a') Fa[0]=1; else if (c0=='b') Fb[0]=1; else Fc[0]=1; }
    for (int t = 1; t < n; ++t) {
        const char ch = s[t];
        if (ch == '?') {
            ++q;
            for (int i = 0; i <= q; ++i) {
                const int len = q - i, off = i * W;
                int* __restrict A = &nFa[off]; int* __restrict B = &nFb[off]; int* __restrict C = &nFc[off];
                const int* fa = &Fa[off]; const int* fb = &Fb[off]; const int* fc = &Fc[off];
                if (i == 0) memset(A, 0, (size_t)(len+1)*sizeof(int));
                else {
                    const int* fbm = fb - W; const int* fcm = fc - W;
                    for (int j = 0; j <= len; ++j) { int v = fbm[j]+fcm[j]; A[j] = (v>=P)?(v-P):v; }
                }
                B[0] = 0;
                for (int j = 1; j <= len; ++j) { int v = fa[j-1]+fc[j-1]; B[j] = (v>=P)?(v-P):v; }
                for (int j = 0; j <= len; ++j) { int v = fa[j]+fb[j]; C[j] = (v>=P)?(v-P):v; }
            }
        } else if (ch == 'a') {
            for (int i = 0; i <= q; ++i) {
                const int len = q-i, off = i*W;
                int* __restrict A=&nFa[off]; int* __restrict B=&nFb[off]; int* __restrict C=&nFc[off];
                const int* fb=&Fb[off]; const int* fc=&Fc[off];
                for (int j=0;j<=len;++j){ int v=fb[j]+fc[j]; A[j]=(v>=P)?(v-P):v; }
                memset(B,0,(size_t)(len+1)*sizeof(int)); memset(C,0,(size_t)(len+1)*sizeof(int));
            }
        } else if (ch == 'b') {
            for (int i = 0; i <= q; ++i) {
                const int len = q-i, off = i*W;
                int* __restrict A=&nFa[off]; int* __restrict B=&nFb[off]; int* __restrict C=&nFc[off];
                const int* fa=&Fa[off]; const int* fc=&Fc[off];
                memset(A,0,(size_t)(len+1)*sizeof(int));
                for (int j=0;j<=len;++j){ int v=fa[j]+fc[j]; B[j]=(v>=P)?(v-P):v; }
                memset(C,0,(size_t)(len+1)*sizeof(int));
            }
        } else {
            for (int i = 0; i <= q; ++i) {
                const int len = q-i, off = i*W;
                int* __restrict A=&nFa[off]; int* __restrict B=&nFb[off]; int* __restrict C=&nFc[off];
                const int* fa=&Fa[off]; const int* fb=&Fb[off];
                memset(A,0,(size_t)(len+1)*sizeof(int)); memset(B,0,(size_t)(len+1)*sizeof(int));
                for (int j=0;j<=len;++j){ int v=fa[j]+fb[j]; C[j]=(v>=P)?(v-P):v; }
            }
        }
        swap(Fa,nFa); swap(Fb,nFb); swap(Fc,nFc);
    }
    vector<int> W2(sz,0);
    vector<ll> rowsum(m+1,0), colsum(m+1,0), diag(m+1,0);
    ll T = 0;
    for (int i = 0; i <= m; ++i) {
        const int len = m-i, off = i*W;
        for (int j = 0; j <= len; ++j) {
            ll v = ((ll)Fa[off+j] + Fb[off+j] + Fc[off+j]) % MOD;
            W2[off+j] = (int)v;
            rowsum[i]+=v; colsum[j]+=v; diag[i+j]+=v; T+=v;
        }
    }
    for (int i=0;i<=m;++i){rowsum[i]%=MOD;colsum[i]%=MOD;diag[i]%=MOD;}
    T %= MOD;
    vector<ll> SA(m+2,0), SB(m+2,0), SC(m+2,0);
    for (int x=m-1;x>=0;--x) SA[x]=(SA[x+1]+rowsum[x+1])%MOD;
    for (int y=m-1;y>=0;--y) SB[y]=(SB[y+1]+colsum[y+1])%MOD;
    for (int z=m-1;z>=0;--z) SC[z]=(SC[z+1]+diag[m-1-z])%MOD;
    vector<int> G(sz,0), H(sz,0), K(sz,0);
    for (int x=m-1;x>=0;--x) for (int y=m-1;y>=0;--y) {
        ll v = (ll)G[(x+1)*W+y] + G[x*W+(y+1)] - G[(x+1)*W+(y+1)];
        if (x+y+2<=m) v += W2[(x+1)*W+(y+1)];
        v%=MOD; if(v<0)v+=MOD; G[x*W+y]=(int)v;
    }
    for (int t=0;t<=m;++t) for (int x=m-1;x>=0;--x) {
        ll v = (ll)H[(x+1)*W+t];
        if (t>0) v += H[x*W+(t-1)] - H[(x+1)*W+(t-1)];
        if (t-x-1>=0) v += W2[(x+1)*W+(t-x-1)];
        v%=MOD; if(v<0)v+=MOD; H[x*W+t]=(int)v;
    }
    for (int t=0;t<=m;++t) for (int y=m-1;y>=0;--y) {
        ll v = (ll)K[(y+1)*W+t];
        if (t>0) v += K[y*W+(t-1)] - K[(y+1)*W+(t-1)];
        if (t-y-1>=0) v += W2[(t-y-1)*W+(y+1)];
        v%=MOD; if(v<0)v+=MOD; K[y*W+t]=(int)v;
    }
    size_t opos = 0;
    for (int qq = 0; qq < Q; ++qq) {
        const ll x = readInt(), y = readInt(), z = readInt();
        const int xx=(int)min(x,(ll)m), yy=(int)min(y,(ll)m), zz=(int)min(z,(ll)m);
        int tt = (int)(m-1-z); if (tt<0) tt=0;
        ll ans = T - SA[xx] - SB[yy] - SC[zz] + (ll)G[xx*W+yy] + (ll)H[xx*W+tt] + (ll)K[yy*W+tt];
        ans %= MOD; if (ans<0) ans += MOD;
        char tmp[12]; int L=0;
        if (ans==0) tmp[L++]='0';
        while (ans>0){ tmp[L++]=(char)('0'+(int)(ans%10)); ans/=10; }
        while (L>0) obuf[opos++]=tmp[--L];
        obuf[opos++]='\n';
    }
    fwrite(obuf, 1, opos, stdout);
    return 0;
}

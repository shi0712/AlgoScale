#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64; typedef unsigned __int128 u128;
typedef __int128 i128; typedef long long ll;

static inline u64 isqrt128(u128 x){
    if(!x) return 0;
    u64 r=(u64)sqrtl((long double)x);
    while(r && (u128)r*r>x) --r;
    while((u128)(r+1)*(r+1)<=x) ++r;
    return r;
}
template<int dir>
static void scanA(u64 h,u64 w,u64 s,u128 P,u64 ac,u64 &best,bool fH,bool fW){
    u64 a=ac,H=(h+a-1)/a,W=s/H,b=(w+W-1)/W;
    i128 V=(i128)s*a*a+(i128)P-(i128)best*s*a;
    while(V<0){
        u64 cur=a+b;
        if(cur<best){ best=cur; V=(i128)s*a*a+(i128)P-(i128)best*s*a; if(V>=0) break; }
        u64 na;
        if constexpr(dir==0){ if(a>=h) break; na=a+1; } else { if(a<=1) break; na=a-1; }
        ll diff=(ll)(na+a)-(ll)best; i128 add=(i128)s*(i128)diff;
        if constexpr(dir==0) V+=add; else V-=add;
        a=na;
        if(fH){ if constexpr(dir==0){ while(H>1 && (u128)(H-1)*a>=h) --H; } else { while((u128)H*a<h) ++H; } }
        else H=(h+a-1)/a;
        if(fW){ if constexpr(dir==0){ while((u128)(W+1)*H<=s) ++W; } else { while(W>1 && (u128)W*H>s) --W; } }
        else W=s/H;
        if constexpr(dir==0){ while(b>1 && (u128)(b-1)*W>=w) --b; } else { while((u128)b*W<w) ++b; }
    }
}
template<int dir>
static void scanW(u64 h,u64 w,u64 s,u64 wmax,u64 wc,u64 &best,bool fH,bool fa){
    u64 W=wc,H=s/W,a=(h+H-1)/H,b=(w+W-1)/W;
    i128 V=(i128)s*w+(i128)h*W*W-(i128)best*s*W;
    while(V<0){
        u64 cur=a+b;
        if(cur<best){ best=cur; V=(i128)s*w+(i128)h*W*W-(i128)best*s*W; if(V>=0) break; }
        u64 nw;
        if constexpr(dir==0){ if(W>=wmax) break; nw=W+1; } else { if(W<=1) break; nw=W-1; }
        i128 term=(i128)h*(i128)(nw+W)-(i128)best*s;
        if constexpr(dir==0) V+=term; else V-=term;
        W=nw;
        if(fH){ if constexpr(dir==0){ while(H>1 && (u128)H*W>s) --H; } else { while((u128)(H+1)*W<=s) ++H; } }
        else H=s/W;
        if(fa){ if constexpr(dir==0){ while((u128)a*H<h) ++a; } else { while(a>1 && (u128)(a-1)*H>=h) --a; } }
        else a=(h+H-1)/H;
        if(fa){ if constexpr(dir==0){ while(b>1 && (u128)(b-1)*W>=w) --b; } else { while((u128)b*W<w) ++b; } }
        else b=(w+W-1)/W;
    }
}
int main(){
    int t; if(scanf("%d",&t)!=1) return 0;
    while(t--){
        u64 h,w,s; if(scanf("%llu %llu %llu",&h,&w,&s)!=3) return 0;
        if((u128)h*w<=(u128)s){ puts("0"); continue; }
        if(h<w) swap(h,w);
        u128 P=(u128)h*w;
        u64 sc=isqrt128(P/(u128)s);
        u64 best=ULLONG_MAX;
        for(int d=-2;d<=2;++d){
            ll v=(ll)sc+d; if(v<1) continue; u64 x=(u64)v;
            if(x<=h){ u64 H=(h+x-1)/x; if(H<=s){ u64 W=s/H; u64 c=x+(w+W-1)/W; if(c<best) best=c; } }
            if(x<=w){ u64 W=(w+x-1)/x; if(W<=s){ u64 H=s/W; u64 c=x+(h+H-1)/H; if(c<best) best=c; } }
        }
        { u64 c=h+(w+s-1)/s; if(c<best) best=c; }
        { u64 c=w+(h+s-1)/s; if(c<best) best=c; }
        if(s>=h){
            u64 ac=sc;
            if((u128)s*ac*(ac+1)<P) ++ac;
            if(ac>h) ac=h; if(ac<1) ac=1;
            bool fH=(s<=2*w), fW=(s<=2*h);
            scanA<0>(h,w,s,P,ac,best,fH,fW);
            scanA<1>(h,w,s,P,ac,best,fH,fW);
        } else {
            u64 wmax=min(w,s);
            u64 wc=isqrt128((u128)s*w/h);
            while((u128)h*wc*(wc+1)<(u128)s*w) ++wc;
            if(wc>wmax) wc=wmax; if(wc<1) wc=1;
            bool fH=(h<=2*w), fa=(h<=2*s);
            scanW<0>(h,w,s,wmax,wc,best,fH,fa);
            scanW<1>(h,w,s,wmax,wc,best,fH,fa);
        }
        printf("%llu\n",best-2);
    }
    return 0;
}

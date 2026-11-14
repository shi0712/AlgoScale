#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static constexpr ll INF = (1LL<<60);

/*
For a proposed finish time T, reverse the elevator's motion. Request i now
has deadline T-a[i], and the reversed route may start anywhere. We have raised
a[i] to at least |x[i]-start|, so after its last request the reversed route can
always return to start by T.

Let D(l,r) be the earliest time to cover [l,r], with the first visit to l last.
For fixed l its finite entries form a prefix. Store E[r] = D(l,r)+x[l].
When l moves left, existing finite E entries do not change: reaching the new
left endpoint from an already completed left endpoint is optimal whenever
that old entry is finite. The new deadline may truncate the prefix.

Beyond the old finite prefix [l+1,k], a route must finish on the right before
returning left. Its best last departure from x[l+1] comes from the first j
satisfying
  E[j] + max_{j<p<=k+1}(a[p]+x[p]) <= T+2*x[l+1].
It then reaches every subsequent right endpoint at x[r]+c, so the new E
entries are one segment 2*x[r]+c. Stop at the first missed right deadline or
the new left deadline. Thus a whole DP row needs only a truncation and one
linear range assignment, rather than an entry-by-entry update.

Persistent suffix-maximum profiles and a tree of linear E segments support
these operations. All pruning bounds in the simultaneous tree search are
lower bounds; ambiguous nodes are examined recursively.
*/
struct Solver {
    struct PNode { int l=0,r=0; ll tag=INF, mn1=INF,mn2=INF,mn=INF; };
    struct ANode { ll tag=0, mn1=0,mn2=0,mn=0,mx=0; int slope=1; };
    int n, sz;
    vector<ll> x,a,b;
    vector<PNode> p;
    vector<int> root;
    vector<ANode> tr;
    vector<ll> rmq;
    vector<int> ruledOut;
    Solver(vector<ll> xx,vector<ll> aa):n(xx.size()),x(move(xx)),a(move(aa)) {
        b.resize(n);for(int i=0;i<n;i++)b[i]=a[i]+x[i];
        sz=1;while(sz<n)sz*=2;
        rmq.assign(2*sz,-INF);
        for(int i=0;i<n;i++)rmq[sz+i]=b[i];
        for(int i=sz-1;i;i--)rmq[i]=max(rmq[2*i],rmq[2*i+1]);
        p.reserve(n*24+1);p.push_back(PNode());
        root.resize(n);
        vector<int> st;
        for(int r=1;r<n;r++) {
            while(!st.empty() && b[st.back()]<=b[r])st.pop_back();
            int l=st.empty()?0:st.back();
            root[r]=passign(root[r-1],0,n-1,l,r-1,b[r]);
            st.push_back(r);
        }
        tr.resize(n*4+4);
        ruledOut.resize(n+1);
    }
    int constant(int old,int l,int r,ll value) {
        int id=p.size();p.push_back(p[old]);
        p[id].tag=value;p[id].mn=value;
        p[id].mn1=value+x[l];p[id].mn2=value+2*x[l];
        return id;
    }
    int passign(int old,int l,int r,int ql,int qr,ll value) {
        if(ql<=l && r<=qr)return constant(old,l,r,value);
        int id=p.size();p.push_back(p[old]);
        int mid=(l+r)/2;
        if(p[id].tag!= -1) {
            // The update replaces complete constant runs of the suffix-maximum
            // profile. Only a previously unused (infinite) run can be split.
            assert(p[id].tag==INF);
            p[id].l=p[id].r=0;
            p[id].tag=-1;
        }
        if(ql<=mid) {int ch=passign(p[id].l,l,mid,ql,qr,value);p[id].l=ch;}
        if(qr>mid) {int ch=passign(p[id].r,mid+1,r,ql,qr,value);p[id].r=ch;}
        const auto &L=p[p[id].l];const auto &R=p[p[id].r];
        p[id].mn=min(L.mn,R.mn);p[id].mn1=min(L.mn1,R.mn1);p[id].mn2=min(L.mn2,R.mn2);
        return id;
    }
    void aset(int v,int l,int r,int slope,ll c) {
        tr[v].slope=slope;tr[v].tag=c;
        tr[v].mn=slope*x[l]+c;tr[v].mx=slope*x[r]+c;
        tr[v].mn1=(slope-1)*x[l]+c;
        tr[v].mn2=(slope-2)*x[(slope==1)?r:l]+c;
    }
    void push(int v,int l,int r) {
        if(tr[v].slope) {
            int mid=(l+r)/2;
            aset(2*v,l,mid,tr[v].slope,tr[v].tag);
            aset(2*v+1,mid+1,r,tr[v].slope,tr[v].tag);
            tr[v].slope=0;
        }
    }
    void assign(int v,int l,int r,int ql,int qr,ll c) {
        if(ql<=l && r<=qr){aset(v,l,r,2,c);return;}
        push(v,l,r);int mid=(l+r)/2;
        if(ql<=mid)assign(2*v,l,mid,ql,qr,c);
        if(qr>mid)assign(2*v+1,mid+1,r,ql,qr,c);
        auto &z=tr[v];auto &L=tr[2*v];auto &R=tr[2*v+1];
        z.mn=min(L.mn,R.mn);z.mx=max(L.mx,R.mx);
        z.mn1=min(L.mn1,R.mn1);z.mn2=min(L.mn2,R.mn2);
        if(L.slope && L.slope==R.slope && L.tag==R.tag){z.slope=L.slope;z.tag=L.tag;}
    }
    int firstGreater(int v,int l,int r,int ql,int qr,ll cap) {
        if(r<ql || l>qr || tr[v].mx<=cap)return -1;
        if(l==r)return l;
        if(tr[v].slope) {
            int lo=max(l,ql),hi=min(r,qr);
            if(tr[v].slope*x[lo]+tr[v].tag>cap)return lo;
            ll lim=(cap-tr[v].tag)/tr[v].slope;
            auto it=upper_bound(x.begin()+lo,x.begin()+hi+1,lim);
            return it==x.begin()+hi+1?-1:int(it-x.begin());
        }
        int mid=(l+r)/2;
        int ans=firstGreater(2*v,l,mid,ql,qr,cap);
        if(ans<0)ans=firstGreater(2*v+1,mid+1,r,ql,qr,cap);
        return ans;
    }
    ll valueAt(int v,int l,int r,int pos) {
        if(tr[v].slope)return tr[v].slope*x[pos]+tr[v].tag;
        int mid=(l+r)/2;
        if(pos<=mid)return valueAt(2*v,l,mid,pos);
        return valueAt(2*v+1,mid+1,r,pos);
    }
    ll rangeMax(int l,int r) {
        ll ans=-INF;
        for(l+=sz,r+=sz+1;l<r;l/=2,r/=2){if(l&1)ans=max(ans,rmq[l++]);if(r&1)ans=max(ans,rmq[--r]);}
        return ans;
    }
    int bad(int v,int l,int r,int ql,ll cap) {
        if(r<ql || rmq[v]<=cap)return n;
        if(l==r)return l;
        int mid=(l+r)/2;
        int z=bad(2*v,l,mid,ql,cap);
        if(z==n)z=bad(2*v+1,mid+1,r,ql,cap);
        return z;
    }
    // Find the first j for which E[j] + max_{j<p<=r}(a[p]+x[p]) <= bound.
    int search(int av,int pv,int l,int r,int ql,int qr,ll bound,int slope=0,ll c=0) {
        if(r<ql || l>qr)return -1;
        if(!slope && tr[av].slope){slope=tr[av].slope;c=tr[av].tag;}
        if(ql<=l && r<=qr) {
            ll lower;
            if(slope)lower=(slope==1?p[pv].mn1:p[pv].mn2)+c;
            else lower=max({p[pv].mn1+tr[av].mn1,p[pv].mn2+tr[av].mn2,p[pv].mn+tr[av].mn});
            if(lower>bound)return -1;
            if(p[pv].tag!=-1) {
                ll val=slope?slope*x[l]+c:tr[av].mn;
                return val+p[pv].tag<=bound?l:-1;
            }
            if(l==r)return l;
        }
        if(l==r)return -1;
        int mid=(l+r)/2;
        // A uniform persistent node has no materialized descendants with its value.
        if(p[pv].tag!=-1) {
            int pos=max(l,ql);
            if(pos>min(r,qr))return -1;
            ll val=slope?slope*x[pos]+c:valueAt(av,l,r,pos);
            return val+p[pv].tag<=bound?pos:-1;
        }
        int ans=search(2*av,p[pv].l,l,mid,ql,qr,bound,slope,c);
        if(ans<0)ans=search(2*av+1,p[pv].r,mid+1,r,ql,qr,bound,slope,c);
        return ans;
    }
    // Finite E entries cannot decrease as the left endpoint moves outward.
    // A prefix ruled out for r stays ruled out for every larger right endpoint.
    int prefixBound(int r) {
        int ans=0;
        for(int i=r+1;i;i-=i&-i)ans=max(ans,ruledOut[i]);
        return ans;
    }
    void remember(int r,int j) {
        for(int i=r+1;i<=n;i+=i&-i)ruledOut[i]=max(ruledOut[i],j);
    }
    bool check(ll T) {
        fill(ruledOut.begin(),ruledOut.end(),0);
        aset(1,0,n-1,1,0);
        int k=n-1;
        for(int l=n-2;l>=0;l--) {
            ll cap=T-a[l]+x[l];
            int cut=firstGreater(1,0,n-1,l+1,k,cap);
            if(cut>=0)k=cut-1;
            else if(k<n-1) {
                int r=k+1, u=l+1, ql=max(u,prefixBound(r)),j=-1;
                if(ql==u && rangeMax(u+1,r)<=T+x[u])j=u;
                else if(ql<=k)j=search(1,root[r],0,n-1,ql,k,T+2*x[u]);
                if(j!=u)remember(r,j<0?r:j);
                if(j>=0) {
                    ll c=valueAt(1,0,n-1,j)-2*x[l+1];
                    int end=r-1;
                    if(2*x[r]+c<=cap)
                        end=upper_bound(x.begin()+r,x.end(),(cap-c)/2)-x.begin()-1;
                    end=min(end,bad(1,0,sz-1,r,T-c)-1);
                    if(end>=r){assign(1,0,n-1,r,end,c);k=end;}
                }
            }
        }
        if(k==n-1)return true;
        int ql=prefixBound(n-1);
        return ql<=k && search(1,root[n-1],0,n-1,ql,k,T+2*x[0])>=0;
    }
};
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int floors,start,m;if(!(cin>>floors>>start>>m))return 0;
    vector<ll> last(floors,-1);
    for(int i=0;i<m;i++){int t,f;cin>>t>>f;last[f]=max(last[f],(ll)t);}
    // A request between two requests released no earlier is redundant:
    // traveling between those two floors serves the middle request as well.
    vector<ll> suffix(floors+1,-1);
    for(int f=floors-1;f>=0;f--)suffix[f]=max(suffix[f+1],last[f]);
    vector<ll>x,a;
    ll pref=-1;
    for(int f=0;f<floors;f++)if(last[f]>=0){
        if(last[f]>pref || last[f]>suffix[f+1]){
            x.push_back(f);
            a.push_back(max(last[f],(ll)abs(f-start)));
        }
        pref=max(pref,last[f]);
    }
    // The start-distance lower bounds may create additional redundant requests.
    int cnt=x.size();suffix.assign(cnt+1,-1);
    for(int i=cnt-1;i>=0;i--)suffix[i]=max(suffix[i+1],a[i]);
    int used=0;pref=-1;
    for(int i=0;i<cnt;i++){
        ll v=a[i];
        if(v>pref || v>suffix[i+1]){x[used]=x[i];a[used]=v;used++;}
        pref=max(pref,v);
    }
    x.resize(used);a.resize(used);
    ll low=*max_element(a.begin(),a.end());
    if(x.size()==1){cout<<low<<'\n';return 0;}
    ll span=x.back()-x.front();
    ll right=abs((ll)start-x.front())+span;
    ll left=abs((ll)start-x.back())+span;
    for(int i=0;i<used;i++){
        right=max(right,a[i]+x.back()-x[i]);
        left=max(left,a[i]+x[i]-x.front());
    }
    ll high=min(left,right);
    Solver sol(move(x),move(a));
    if(!sol.check(low)){
        ++low;
        while(low<high){ll mid=(low+high)/2;if(sol.check(mid))high=mid;else low=mid+1;}
    }
    cout<<low<<'\n';
    return 0;
}

// Contestant submission 190471345, https://codeforces.com/contest/1764/submission/190471345
// Source: open-r1/codeforces-submissions (train-00035-of-00036.parquet), CC BY 4.0.
// https://huggingface.co/datasets/open-r1/codeforces-submissions
// https://creativecommons.org/licenses/by/4.0/
#include <bits/stdc++.h>

using namespace std;



#define int long long

#define fi first

#define se second

using ll=long long;

using ull=unsigned long long;

using lll=__int128;

using db=double;

using ld=long double;

using pii=pair<int,int>;

using vi=vector<int>;

using vii=vector<pii>;

mt19937 rnd(random_device{}());



const int N=1055;

int n;

map<pair<pii,int>,int> mp;

int qry(int l,int r,int k)

{

	if (l>r) return 0;

	if (l==r) return 1;

	if (l==1&&r==n) return n/k+1;

	int &x=mp[{pii{l,r},k}];

	if (x) return x;

	printf("? %lld %lld %lld\n",l,r,k);

	fflush(stdout);

	scanf("%lld",&x);

	return x;

}

int fln;

void solve(int l,int r)

{

	if (l==r) 

	{

		printf("! %lld\n",l);

		fflush(stdout);

		exit(0);

	}

	if (l+1==r)

	{

		if (n%2==0&&!fln)

		{

			if (l==1)

			{

				if (qry(r,n,n)>1) solve(l,l);

				else solve(r,r);

			}

			else

			{

				if (qry(1,l,n)>1) solve(r,r);

				else solve(l,l);

			}

		}

		else

		{

			if (l>1&&qry(1,l-1,2)+1==qry(1,r,2))

			{

				if (qry(1,l-1,2)+1==qry(1,l,2)) solve(l,l);

				else solve(r,r);

			}

			else

			{

				if (qry(r+1,n,2)+1==qry(r,n,2)) solve(r,r);

				else solve(l,l);

			}

		}

		return;

	}

	int mid=l+r>>1;

	int cl=qry(1,mid,2)*2-mid;

	int cr=qry(mid+1,n,2)*2-(n-mid);

	if (n%2==0)

	{

		if (!fln)

		{

			if (cl==cr)

			{

				if (qry(mid+1,n,n)>1) fln=2,cr--;

				else fln=1,cl--;

			}

		}

		else fln==1?cl--:cr--;

	}

	if (cl>cr) solve(l,mid);

	else solve(mid+1,r);

}

signed main()

{

	scanf("%lld",&n);

	solve(1,n);

}

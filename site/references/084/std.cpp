// Contestant submission 303356923, https://codeforces.com/contest/1847/submission/303356923
// Source: open-r1/codeforces-submissions (train-00035-of-00036.parquet), CC BY 4.0.
// https://huggingface.co/datasets/open-r1/codeforces-submissions
// https://creativecommons.org/licenses/by/4.0/
#include<algorithm>
#include<cstdio>
#include<vector>
#include<cstdlib>
#define N 5010
#define M 10
using namespace std;
int ans[N] = {0, 1, 1, 1, 2, 2, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int n, ss = 0;
int hh[N];
int a[N], f[M][M][M], g[M][M][M];
vector<int>zh;
void slove(vector<int>p);
int inp(int a, int b, int c)
{
	int indices[] = {a, b, c};
	sort(indices, indices + 3);
	printf("? %d %d %d\n", indices[0], indices[1], indices[2]);
	fflush(stdout);
	int x;
	if (scanf("%d", &x) != 1 || x < 0) exit(0);
	return x;
//	return f[ans[a]][ans[b]][ans[c]];
}
void dfs(int now)
{
	if(now == zh.size())
	{
		int ff = 1;
		for(int i = 0; i < zh.size(); i++)
		for(int j = i + 1; j < zh.size(); j++)
		for(int k = j + 1; k < zh.size(); k++)
			if(g[i][j][k] != f[hh[zh[i]]][hh[zh[j]]][hh[zh[k]]])
				ff = 0;
		if(ff)
		{
			if(ss)
			{
				printf("! -1\n");
				exit(0);
			}
			ss++;
			for(int i = 0; i < zh.size(); i++)a[zh[i]] = hh[zh[i]];
			return;
		}
		return;
	}
	for(int i = 1; i <= 4; i++)
	{
		hh[zh[now]] = i;
		dfs(now + 1);
	}
}
void gg(vector<int>p, int w, int k1, int k2)
{
	a[k1] = w;
	a[k2] = w;
	if(w != 1)
	{
		for(int to : p)
		if(to != k1 && to != k2)
		{
			int x = inp(k1, k2, to);
			if(w == 2)
			{
				if(x == 15)a[to] = 1;
				if(x == 48)a[to] = 2;
				if(x == 63)a[to] = 3;
				if(x == 0)a[to] = 4;
			}
			if(w == 3)
			{
				if(x == 35)a[to] = 1;
				if(x == 128)a[to] = 2;
				if(x == 243)a[to] = 3;
				if(x == 320)a[to] = 4;
			}
			if(w == 4)
			{
				if(x == 63)a[to] = 1;
				if(x == 240)a[to] = 2;
				if(x == 495)a[to] = 3;
				if(x == 768)a[to] = 4;
			}
		}
		return;
	}
	int k = 0;
	vector<int>h;
	for(int to : p)
	if(to != k1 && to != k2)
	{
		if(k)h.push_back(to);
		else
		{
			int x = inp(k1, k2, to);
			if(x == 3) a[to] = 1;
			else
			{
				h.push_back(to);
				if(h.size() == 4)k = 1;
			}
		}
	}
	int r = 0;
	for(int i = 0; i < 4 && i < h.size(); i++)
		for(int j = i + 1; j < 4 && j < h.size(); j++)
		{
			int x = inp(k1, h[i], h[j]);
			if(x == 15)
			{
				r = 1, gg(h, 2, h[i], h[j]);
				return;
			}
			if(x == 35)
			{
				r = 1, gg(h, 3, h[i], h[j]);
				return;
			}
			if(x == 63)
			{
				r = 1, gg(h, 4, h[i], h[j]);
				return;
			}
		}
	if(!r)slove(h);
}
void slove(vector<int>p)
{
	if(p.size() <= 9)
	{
		for(int i = 0; i < p.size(); i++)
		for(int j = i + 1; j < p.size(); j++)
		for(int k = j + 1; k < p.size(); k++)
		{
			int x = inp(p[i], p[j], p[k]);
			g[i][j][k] = x;
		}
		zh = p;
		dfs(0);
		return;
	}
	int w = 0, k1, k2;
	for(int i = 0; i < 9; i++)
	for(int j = i + 1; j < 9; j++)
	for(int k = j + 1; k < 9; k++)
	if(!w)
	{
		int x = inp(p[i], p[j], p[k]);
		k1 = p[i];
		k2 = p[j];
		if(x == 3) w = 1;
		else if(x == 48)w = 2;
		else if(x == 243)w = 3;
		else if(x == 768)w = 4;
	}
	gg(p, w, k1, k2);
}
int main()
{
	for(int a = 1; a <= 4; a++)
	for(int b = 1; b <= 4; b++)
	for(int c = 1; c <= 4; c++)
	if(a + b > c && a + c > b && b + c > a)
	f[a][b][c] = (a + b + c) * (a + b - c) * (a + c - b) * (b + c - a);
	scanf("%d", &n);
	vector<int>p;
	for(int i = 1; i <= n; i++)p.push_back(i);
	slove(p);
	printf("! ");
	for(int i = 1; i <= n; i++)printf("%d ", a[i]);
	printf("\n");
	return 0;
}

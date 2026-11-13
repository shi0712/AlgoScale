# Sum and Product 题解

## 1. 把加法条件化为乘积同余

需要统计所有 $0\le k\le m$、$0\le a,b<n$，满足

$$
k(a+b)\equiv ab\pmod n
$$

的有序三元组。直接枚举 $k$ 或 $(a,b)$ 都无法适应 $n\le10^{12}$。

原式可以写成

$$
(a-k)(b-k)\equiv k^2\pmod n.
$$

对于固定的 $k$，模 $n$ 下平移是双射。因此令 $x=a-k,y=b-k$，所求的 $f(n,k)$ 就等于

$$
xy\equiv k^2\pmod n
$$

在模 $n$ 剩余类中的有序解数。

## 2. 按 $\gcd(x,n)$ 分类

固定 $d=\gcd(x,n)$。这样的 $x$ 共有 $\varphi(n/d)$ 个。

- 若 $d\nmid k^2$，则 $xy\equiv k^2\pmod n$ 无解。
- 若 $d\mid k^2$，约去 $d$ 后，$x/d$ 与 $n/d$ 互质，因此 $y$ 在模 $n/d$ 下唯一确定。提升到模 $n$ 后，恰好有 $d$ 个不同的 $y$。

由此得到

$$
f(n,k)=\sum_{\substack{d\mid n\\d\mid k^2}}d\varphi(n/d).
$$

这里也包含 $x=0$ 的情形：此时 $d=n$，且 $\varphi(1)=1$。

设 $d=\prod_p p^{i_p}$，定义

$$
q(d)=\prod_p p^{\lceil i_p/2\rceil}.
$$

逐个比较质因子的指数可知，$d\mid k^2$ 等价于 $q(d)\mid k$。在区间 $[0,m]$ 中，$q(d)$ 的倍数有 $\lfloor m/q(d)\rfloor+1$ 个，其中多出的 $1$ 对应 $k=0$。

交换求和顺序，答案变为

$$
\sum_{d\mid n}d\varphi(n/d)
\left(\left\lfloor\frac{m}{q(d)}\right\rfloor+1\right).
$$

问题已经从枚举 $m$ 个整数，缩小为枚举 $n$ 的因子。

## 3. 合并具有相同 $q(d)$ 的因子

对一个质数幂 $p^e\parallel n$，记

$$
P=p^e,\qquad A=\varphi(p^e),\qquad h=\left\lceil\frac e2\right\rceil.
$$

若 $d$ 中 $p$ 的指数为 $i$，对应权值的局部因子为

$$
p^i\varphi(p^{e-i})=
\begin{cases}
A,&0\le i<e,\\
P,&i=e.
\end{cases}
$$

但 $q(d)$ 中只记录 $j=\lceil i/2\rceil$，所以可以把产生同一 $j$ 的项加起来：

$$
w_p(j)=
\begin{cases}
A,&j=0,\\
2A,&0<j<h,\\
P,&j=h\text{ 且 }e\text{ 为奇数},\\
A+P,&j=h\text{ 且 }e\text{ 为偶数}.
\end{cases}
$$

原因是，中间的 $j$ 对应两个指数 $i=2j-1,2j$；最后一组在 $e$ 为奇数时只有 $i=e$，在 $e$ 为偶数时有 $i=e-1,e$。

令

$$
R=\prod_{p^e\parallel n}p^{\lceil e/2\rceil},\qquad
W(q)=\prod_p w_p(v_p(q)).
$$

答案中所有 $k=0$ 的贡献可直接相乘得到：

$$
f(n,0)=\prod_{p^e\parallel n}\bigl(p^e+e\varphi(p^e)\bigr).
$$

其余贡献为

$$
\boxed{\sum_{\substack{q\mid R\\q\le m}}W(q)\left\lfloor\frac mq\right\rfloor.}
$$

DFS 枚举各质因子的指数 $0\le j\le h$，同时维护 $q$ 和 $W(q)$，即可计算这个式子。如果继续乘入一个质数后会使 $q>m$，后续更大的指数也不必枚举。

## 4. 适应新范围的质因数分解

一共有至多 $10^4$ 组测试，$n$ 可达 $10^{12}$。逐个试除到 $\sqrt n$ 会在大量大质数或大半素数输入上耗时较多。参考实现使用 Miller–Rabin 判素与 Pollard–Rho 分解。

Miller–Rabin 先判断当前数是否为质数；若不是，Pollard–Rho 尝试找到一个非平凡因子，再递归分解两部分。采用适用于 64 位整数的固定底数组，可以使判素结论在本题范围内确定。Pollard–Rho 的运行时间采用通常的随机启发式分析，不应把它写成确定的最坏时间保证。

所有计数都对 $998244353$ 取模，但质因数分解和 $q\le m$ 的判断必须使用原整数。模乘中间结果用 `__uint128_t`，防止 64 位乘法溢出。

## 5. 正确性与复杂度

平移把原条件双射为乘积同余；按 $\gcd(x,n)$ 分类得到互不重叠的全部解；因子条件 $d\mid k^2$ 与 $q(d)\mid k$ 等价，所以交换求和得到的因子公式准确。最后，局部权值只是合并相同 $q(d)$ 的贡献，没有删除或重复任何项。DFS 剪掉的分支均满足 $\lfloor m/q\rfloor=0$。因此算法的结果就是题目要求的总和。

若 $n=\prod_i p_i^{e_i}$，记

$$
D=\prod_i\left(\left\lceil\frac{e_i}{2}\right\rceil+1\right).
$$

得到质因数分解后，DFS 的最坏时间为 $O(D)$，剪枝只会减少工作量；额外空间为 $O(\log n)$。总时间为**质因数分解的代价加 $O(D)$**。Pollard–Rho 找因子的常见启发式估计约为 $O(n^{1/4})$ 次迭代，本实现每次迭代还含一次最大公约数计算。多组数据的代价逐组相加。

边界 $m=0$ 时只需输出 $f(n,0)$。当 $n=1$ 时没有质因子，空乘积取 $1$，且题目保证 $m=0$，结果也是 $1$。

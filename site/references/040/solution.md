# 《平移乘积》题解

## 1. 等价变形

原同余式为

$$
k(a+b)\equiv ab\pmod n.
$$

移项并配方：

$$
(a-k)(b-k)\equiv k^2\pmod n.
$$

令 $x=a-k,y=b-k$。在模 $n$ 的剩余类中，这个平移是一一对应，所以 $f(n,k)$ 等于同余式

$$
xy\equiv k^2\pmod n
$$

的有序解数。

## 2. 固定 $\gcd(x,n)$

固定 $d=\gcd(x,n)$。满足这一条件的 $x$ 有 $\varphi(n/d)$ 个。

若 $d\nmid k^2$，显然无解。若 $d\mid k^2$，写成 $x=dx'$ 后约去 $d$，因为 $x'$ 在模 $n/d$ 下可逆，所以 $y$ 在模 $n/d$ 下唯一；提升回模 $n$ 后共有 $d$ 个 $y$。

因此

$$
f(n,k)=\sum_{\substack{d\mid n\\d\mid k^2}}d\varphi(n/d).
$$

设

$$
d=\prod_p p^{i_p},\qquad q(d)=\prod_p p^{\lceil i_p/2\rceil}.
$$

则 $d\mid k^2$ 当且仅当 $q(d)\mid k$。交换求和顺序得到

$$
\sum_{k=0}^{m}f(n,k)
=\sum_{d\mid n}d\varphi(n/d)
\left(\left\lfloor\frac{m}{q(d)}\right\rfloor+1\right).
$$

这已经给出了一个正确的因数枚举算法，但还可以合并具有相同 $q(d)$ 的项。

## 3. 合并相同的 $q(d)$

考虑 $p^e\parallel n$，记

$$
P=p^e,\qquad A=\varphi(p^e),\qquad h=\left\lceil\frac e2\right\rceil.
$$

当 $d$ 中 $p$ 的指数为 $i$ 时，$d\varphi(n/d)$ 的局部贡献为

$$
p^i\varphi(p^{e-i})=
\begin{cases}
A,&i<e,\\
P,&i=e.
\end{cases}
$$

而 $q(d)$ 中 $p$ 的指数是 $j=\lceil i/2\rceil$。合并产生同一 $j$ 的指数 $i$，得到局部权值

$$
w_p(j)=
\begin{cases}
A,&j=0,\\
2A,&0<j<h,\\
P,&j=h\text{ 且 }e\text{ 为奇数},\\
A+P,&j=h\text{ 且 }e\text{ 为偶数}.
\end{cases}
$$

令

$$
R=\prod_{p^e\parallel n}p^{\lceil e/2\rceil},
\qquad W(q)=\prod_p w_p(v_p(q)).
$$

于是原答案可以按 $q\mid R$ 枚举。

代码将 $k=0$ 的部分单独求出：

$$
f(n,0)=\prod_{p^e\parallel n}\left(p^e+e\varphi(p^e)\right).
$$

剩余部分为

$$
\sum_{\substack{q\mid R\\q\le m}}W(q)\left\lfloor\frac mq\right\rfloor.
$$

DFS 只扩展 $q\le m$ 的分支；一旦 $q>m$，该分支以及所有后代的商都为零。

## 4. 质因数分解

本版本取消了“大数用例至多 $10$ 个”的限制，所以筛出 $10^6$ 内质数后逐个试除会在大量接近 $10^{12}$ 的质数或大质数平方上消耗大量时间。

标准程序使用：

- 固定底数 Miller-Rabin，对无符号 64 位范围确定判素；
- Pollard-Rho 分解合数；
- `__uint128_t` 完成模乘，避免乘法溢出。

Pollard-Rho 使用固定随机种子，答案和本地复测路径可复现；其运行时间仍属于随机分解算法的通常期望分析，而非确定的最坏界。

## 5. 正确性证明

**引理 1**：$f(n,k)$ 等于 $xy\equiv k^2\pmod n$ 的有序剩余类解数。

证明：映射 $(a,b)\mapsto(a-k,b-k)$ 是模 $n$ 剩余类有序对上的双射，且原同余式恰好等价于变形后的同余式。证毕。

**引理 2**：

$$
f(n,k)=\sum_{d\mid n,d\mid k^2}d\varphi(n/d).
$$

证明：按 $d=\gcd(x,n)$ 分类。每类有 $\varphi(n/d)$ 个 $x$。当且仅当 $d\mid k^2$ 时有解；有解时，约去 $d$ 后 $x/d$ 可逆，$y$ 模 $n/d$ 唯一，因而模 $n$ 有 $d$ 个提升。各类互不相交，求和即得。证毕。

**引理 3**：$d\mid k^2$ 当且仅当 $q(d)\mid k$。

证明：逐个质数比较指数。$p^{i_p}\mid k^2$ 等价于 $v_p(k)\ge\lceil i_p/2\rceil$，正是 $p^{\lceil i_p/2\rceil}\mid k$。证毕。

**引理 4**：局部权值 $w_p(j)$ 等于所有满足 $\lceil i/2\rceil=j$ 的指数 $i$ 所对应的 $p^i\varphi(p^{e-i})$ 之和。

证明：$i<e$ 时每项均为 $A$，$i=e$ 时为 $P$。$j=0$ 仅对应 $i=0$；中间 $j$ 对应 $2j-1,2j$；末端根据 $e$ 的奇偶分别只有 $e$，或有 $e-1,e$。代入即可得到四种权值。证毕。

**定理**：标准程序输出题目要求的答案。

证明：由引理 1、2、3，交换 $k$ 与 $d$ 的求和顺序可得直接因数公式。由引理 4，标准程序对相同 $q(d)$ 的项进行的乘法式合并保持总权值不变。单独提出的 $f(n,0)$ 正好对应公式中的所有“$+1$”，DFS 则枚举其余 $q\le m$ 的项；被剪枝的项均满足 $\lfloor m/q\rfloor=0$。故程序计算的和与题目定义完全相同。证毕。

## 6. 复杂度与计算模型

设

$$
n=\prod p_i^{e_i},\qquad
D=\prod_i\left(\left\lceil\frac{e_i}{2}\right\rceil+1\right).
$$

在固定字长整数运算为 $O(1)$ 的 RAM 模型下：

- 合并后的 DFS 最坏枚举 $D$ 个状态，时间 $O(D)$；剪枝只会减少状态数；
- Miller-Rabin 每次判素需要 $O(\log n)$ 次模乘，底数个数固定；
- Pollard-Rho 分解采用通常的启发式期望分析，寻找因子的工作量约为 $O(n^{1/4})$ 次固定字长模运算；这不是确定性最坏时间保证；
- 对每组数据，可概括为 Pollard-Rho 分解的期望代价加 $O(D)$；全部 $T$ 组的代价为逐组求和；
- 辅助空间为 $O(\log n)$，来自递归分解、质因子表和 DFS 深度。

所有计数均及时对 $998244353$ 取模；`__uint128_t` 仅用于模乘中间值。$n\le10^{12}$ 保证质数幂、DFS 中的 $q$ 及相关乘法边界可安全存入无符号 64 位整数。

# Old Problems 题解

## 题意

求

$$F(A,B,C)=\sum_{i=1}^A\sum_{j=1}^B\sum_{k=1}^C\tau(ijk)\pmod{10^9+7},$$

其中 $\tau(x)$ 是正约数个数。三个上界均可达到 $10^6$，不能枚举三元组，也不能筛到 $ABC$。我们先把乘积的约数个数拆成修正因子的和，再利用整除分块合并大量相同贡献。

## 1. 从素数指数得到四变量公式

固定一个素数，假设它在 $i,j,k$ 中的指数分别是 $\alpha,\beta,\gamma$。它对 $\tau(ijk)$ 的贡献是 $\alpha+\beta+\gamma+1$，并且

$$\begin{aligned}
\alpha+\beta+\gamma+1={}&(\alpha+1)(\beta+1)(\gamma+1)\\
&-\alpha\beta(\gamma+1)-\alpha\gamma(\beta+1)-\beta\gamma(\alpha+1)\\
&+2\alpha\beta\gamma.
\end{aligned}$$

第一项对应分别计算三个数的约数个数；中间三项分别修正“两边共同含有该素数”；最后一项修正三边都含有的情况。

展开所有素数的乘积。将选中三项两两修正的素数乘积分别记作 $r,s,t$，选中三边修正的素数乘积记作 $d$。每个素数只能选择一次，所以 $d,r,s,t$ 都无平方因子，且两两互质。这恰好等价于 $\mu^2(drst)=1$。

记

$$D(x)=\sum_{v=1}^x\tau(v),\qquad D(0)=0,$$

其中 $\mu$ 是 Möbius 函数，$\omega$ 是不同质因子个数。对剩余因子分别求和，得到

$$\boxed{F(A,B,C)=\sum_{\mu^2(drst)=1}2^{\omega(d)}\mu(r)\mu(s)\mu(t)
D\!\left(\left\lfloor\frac A{drs}\right\rfloor\right)
D\!\left(\left\lfloor\frac B{drt}\right\rfloor\right)
D\!\left(\left\lfloor\frac C{dst}\right\rfloor\right).}$$

$D,\mu,\omega$ 及最小质因子均可在线性筛中预处理到最大上界，而不是到 $ABC$。

## 2. 利用对称性缩小枚举

输入三个上界的地位对称，先排成 $A\le B\le C$。同时将修正变量按 $r\le s\le t$ 枚举，再补上它们的不同排列。

分母依次为 $drs\le drt\le dst$。若存在一个排列能产生非零贡献，必然满足

$$drs\le A,\qquad drt\le B,\qquad dst\le C.$$

这是把三个分母从小到大匹配三个容量的必要且充分条件。

由于修正变量两两互质，相等的变量只能都是 $1$。于是分成两类：

1. $r=s=t=1$：直接累加 $2^{\omega(d)}D(A/d)D(B/d)D(C/d)$。
2. $r\le s<t$：固定 $d,r,s$，枚举 $t\in[s+1,U]$，其中 $U=\min(B/(dr),C/(ds))$。

$r=s=1$ 时共有三种不同排列；否则共有六种。枚举每个不同排列的分母与 $A,B,C$ 的对应关系即可。某个排列不满足实际容量时，相应 $D(0)=0$，自动没有贡献。

## 3. 不逐个枚举 $t$：六个商一起分块

固定 $d,r,s$，与 $t$ 有关的整除商只有

$$\left\lfloor\frac{\lfloor X/(dr)\rfloor}{t}\right\rfloor,
\quad\left\lfloor\frac{\lfloor X/(ds)\rfloor}{t}\right\rfloor,
\qquad X\in\{A,B,C\}.$$

从左端点 $L$ 出发，对一个常数 $q$，若 $v=\lfloor q/L\rfloor>0$，则该商保持不变直到 $\lfloor q/v\rfloor$；若商为零，之后也一直为零。将六个右端点与 $U$ 取最小值，得到一个共同区间 $[L,R]$。

区间内所有 $D$ 的乘积都相同，唯一需要求和的是

$$\sum_{t=L}^R\mu(t)[\gcd(t,drs)=1].$$

因此，只要能快速计算这个带互质限制的 Möbius 区间和，就能整块累加贡献。

## 4. 互质 Möbius 和

设 $\mathcal M(x)=\sum_{t\le x}\mu(t)$，$\operatorname{rad}(m)$ 表示 $m$ 的不同质因子之积。定义

$$\mathcal M_m(x)=\sum_{t\le x}\mu(t)[\gcd(t,m)=1].$$

有恒等式

$$\boxed{\mathcal M_m(x)=\sum_{q\le x,\ \operatorname{rad}(q)\mid m}\mathcal M(\lfloor x/q\rfloor).}$$

可以按素数检验：对于 $p\mid m$，Möbius 函数在 $1,p,p^2,\ldots$ 上的局部序列是 $1,-1,0,\ldots$，与 $1,1,1,\ldots$ 卷积后为 $1,0,0,\ldots$，正好删去所有含 $p$ 的项；对于不整除 $m$ 的素数则保持原来的 Möbius 因子。

因此，分解 $m=drs$ 后，按质因子递归枚举 $q$ 的各个指数，就能求出区间和

$$\sum_{\operatorname{rad}(q)\mid m}
\left(\mathcal M(\lfloor R/q\rfloor)-\mathcal M(\lfloor(L-1)/q\rfloor)\right).$$

$q$ 可以含任意次幂。例如 $m=6$ 时要枚举 $2^a3^b$，不能只枚举 $1,2,3,6$。若递归中两端整除商已经相等，该分支的贡献为零，可以直接停止。

## 5. 小范围区间的位集优化

在本题时限下，大量较小的互质区间查询值得单独加速。参考实现固定 $K=2048$，对每个无平方因子的 $m$ 保存 $K$ 位的位集 $C_m$，第 $t-1$ 位表示 $\gcd(t,m)=1$。

若 $p$ 是 $m$ 的最小质因子，则

$$C_m=C_{m/p}\ \&\ \overline{\text{“}p\text{ 的倍数”位集}}.$$

另建 $\mu(t)=1$ 与 $\mu(t)=-1$ 两个位集。查询 $R\le K$ 的区间时，分别求交并统计置位数，两者相减即答案。每个位集只有 $32$ 个 64 位字，一次查询最多扫描 $32$ 个字。

很短的区间也可直接逐项检查；较大区间仍使用上一节的递归。位集只改变计算同一数学量的方法，不改变计数公式。

## 6. 正确性与复杂度

素数指数恒等式给出原和式的精确展开；排序枚举加不同排列覆盖每个修正四元组一次；整除分块把相同乘积系数合并；互质 Möbius 恒等式精确计算块内权重。因此最终累加值等于原来的三重求和。

令 $N=\max(A,B,C)$。固定 $d,r,s$ 时，$t>s$ 上的每个商最多变化 $O(N/(drs))$ 次。总分块数不超过

$$O\left(\sum_{d\le N}\sum_{s\le\sqrt{N/d}}\sum_{r\le s}\frac N{drs}\right)=O(N\log^3(N+1)).$$

令 $\Psi(N)$ 是 $m,x\le N$ 时，仅使用 $m$ 的质因子且不超过 $x$ 的整数个数的最大值。递归枚举一次区间和的代价为 $O(\Psi(N))$。对任意固定 $\varepsilon>0$，

$$\Psi_m(x)\le x^\varepsilon\prod_{p\mid m}(1-p^{-\varepsilon})^{-1}
\le C_\varepsilon x^\varepsilon\operatorname{rad}(m)^\varepsilon,$$

所以 $\Psi(N)=N^{o(1)}$。单组复杂度可写为

$$\boxed{O(N\log^3(N+1)\Psi(N))=N^{1+o(1)}\text{ 时间},\quad O(N)\text{ 空间}.}$$

空间界把固定宽度位集的 $32$ 字视为常数。所有测试共用一次到最大 $N$ 的筛；题目还保证 $\sum N\le2\times10^6$。这个渐近界含有次多项式因子，不应简写为未经证明的固定多对数界。

实现中，乘积界限要先用 64 位整数检查，再访问筛表；不能访问可能超过 $N$ 的下标 $drst$。负的 Möbius 贡献最终需规范到 $[0,10^9+6]$。枚举质数幂时先判断是否超过商界，避免乘法溢出。

# 三维约数和：题解与实现说明

改编自 [QOJ 2586](https://qoj.ac/problem/2586)，使用[讨论 2610](https://qoj.ac/problem/2586/discussion/2610) 的局部修正展开、整除分块与互质 Möbius 和方法。标准程序 `src/std.cpp` 对每个输入现场筛表、求和，没有预存答案。

## 1. 从一个素数的指数开始

若某素数在 $i,j,k$ 中的指数为 $\alpha,\beta,\gamma$，则它对 $\tau(ijk)$ 的贡献是 $\alpha+\beta+\gamma+1$。恒等式

$$\alpha+\beta+\gamma+1=(\alpha+1)(\beta+1)(\gamma+1)-\alpha\beta(\gamma+1)-\alpha\gamma(\beta+1)-\beta\gamma(\alpha+1)+2\alpha\beta\gamma$$

将这个贡献展开成五种选择：不修正、三个两两修正之一、三者共同修正。把两两修正素数的乘积记为 $r,s,t$，共同修正的乘积记为 $d$。

每个素数只能选一种修正，故 $d,r,s,t$ 各自无平方因子且两两互质，等价于 $\mu^2(drst)=1$。将各素数的恒等式相乘，展开后分别对剩余因子求和。记 $D(x)=\sum_{v=1}^x\tau(v)$，$D(0)=0$，可得

$$F(A,B,C)=\sum_{\mu^2(drst)=1}2^{\omega(d)}\mu(r)\mu(s)\mu(t)
D\left(\left\lfloor\frac A{drs}\right\rfloor\right)
D\left(\left\lfloor\frac B{drt}\right\rfloor\right)
D\left(\left\lfloor\frac C{dst}\right\rfloor\right).$$

$\mu$ 是 Möbius 函数，$\omega$ 是不同质因子数。所有除法中的分母均为普通整数，先整除再访问 $D$，没有模意义下的除法。

## 2. 利用修正变量的对称性

先排序输入上界为 $A\le B\le C$，不改变答案。枚举 $r\le s\le t$，将不同排列对三个上界的贡献全部加上。最小的乘积分母是 $drs$，最大的为 $dst$；若 $drs>A$ 或 $dst>C$，任何排列都没有贡献。一个足够覆盖所有可能项的上界为

$$drs\le A,\qquad drt\le B,\qquad dst\le C.$$

由于互质，两个相等的修正变量只能同时为 $1$。程序单独计入 $r=s=t=1$；其余情况枚举 $r\le s<t$，即 $t\ge s+1$。当 $r=s=1$ 时补三种不同排列，否则补六种，避免把相同变量的交换重复计数。

固定 $d,r,s$ 后，$t$ 的上界可取
$U=\min(\lfloor B/(dr)\rfloor,\lfloor C/(ds)\rfloor)$。
补排列时超出某个真实上界的项自然含 $D(0)=0$。

## 3. 六个整除商共同分块

与 $t$ 有关的商来自

$$\left\lfloor\frac {\lfloor X/(dr)\rfloor}t\right\rfloor,\qquad
\left\lfloor\frac {\lfloor X/(ds)\rfloor}t\right\rfloor,
\quad X\in\{A,B,C\}.$$

从左端点 $L$ 出发，对每个非零商 $v=\lfloor q/L\rfloor$，将块右端点限制到 $\lfloor q/v\rfloor$；商为零时不再变化。六者右端点取最小值，再与 $U$ 取小，便得到所有乘积系数都不变的区间。

块内只剩

$$\sum_{L\le t\le R}\mu(t)[\gcd(t,drs)=1].$$

短块长度不超过 $32$ 时直接枚举是常数阈值优化；大块使用下面的前缀和，算法仍然现场计算。

## 4. 互质 Möbius 前缀和

令 $\mathcal M(x)=\sum_{t\le x}\mu(t)$，$\mathcal M(0)=0$。对 $m=drs$，有

$$\mathcal M_m(x)=\sum_{t\le x}\mu(t)[\gcd(t,m)=1]
=\sum_{q\le x,\ \operatorname{rad}(q)\mid m}\mathcal M(\lfloor x/q\rfloor).$$

证明可直接检查狄利克雷卷积的素数幂因子：对于 $p\mid m$，$\mu$ 的局部序列 $1,-1,0,\ldots$ 与全 $1$ 序列卷积为 $1,0,0,\ldots$，恰好屏蔽所有含 $p$ 的正幂；对 $p\nmid m$ 保留 $\mu$。

枚举的 $q$ 可以含任意次幂。例如 $m=6$ 时须枚举 $2^a3^b$，不能只枚举 $m$ 的平方自由约数。程序分解 $m$，按质因子依次 DFS 枚举幂，直接计算两端差
$\mathcal M(\lfloor R/q\rfloor)-\mathcal M(\lfloor(L-1)/q\rfloor)$，并在两商相等或下一质因子已过大时停止。每一合法修正四元组恰贡献一次，其权重来自第 1 节恒等式，故分块后的答案仍严格等于原三重和。

## 5. 标程中的固定宽度加速

直接实现上述公式，在本范围下仍会反复求很多较小的互质 Möbius 区间和。本包最终标程保留相同的整除分块与 DFS，同时使用固定常数 $K=2048$ 的辅助位集。

对每个 $m\le N$ 的无平方因子整数，现场构造位集 $C_m$，第 $t-1$ 位为 $[\gcd(t,m)=1]$，只记录 $1\le t\le K$。令 $p$ 为 $m$ 的最小质因子，则

$$C_m=C_{m/p}\mathbin{\&}\overline{\operatorname{multiples}(p)},\qquad C_1=\text{全 1 位集}.$$

另从本次线性筛结果构造 $\mu(t)=1$ 和 $\mu(t)=-1$ 的两个位集。对 $R\le K$，将 $C_m$ 与区间掩码及这两个符号位集分别求交、数置位位数，两者相减便精确得到所需区间和。只为无平方因子的 $m$ 分配位集，并用索引数组映射，降低空间常数。大端点仍使用原 DFS；递归中剩余的质因子乘积也是无平方因子整数，端点足够小时同样可以调用此加速。

每个位集固定 $K/64=32$ 个 64 位字，预处理、空间均为 $O(32N)=O(N)$；一次位集区间查询至多扫描 32 个字，所以不改变后文上界。这里将 $K=2048$ 视为实现中的固定常数；若把 $K$ 作为随输入增长的参数，就必须显式计入 $O(NK/64)$ 的代价。

此外，对当前上界现场生成 $D(\lfloor A/q\rfloor)$、$D(\lfloor B/q\rfloor)$、$D(\lfloor C/q\rfloor)$ 的数组以减少重复除法；总长度为 $O(N)$。短区间直接枚举、零 Möbius 和跳过权重计算、需要时才分解质因子，均为常数方面的改进。

这些表只记录本次输入对应的辅助算术量与互质关系，**不是题目答案表**。当输入上界达到 $10^6$ 时，算法仍然执行修正因子枚举、整除分块以及大端点的 Möbius DFS，不能通过查询 $2048$ 范围的答案解决本题。

## 6. 真实复杂度

令单用例 $N=\max(A,B,C)$。固定 $d,r,s$，从 $t>s$ 开始，每个整除商最多有 $O(N/(drs))$ 种取值。六个商的分段数也在这个界内。因此总块数不超过常数倍的

$$\sum_{d\le N}\sum_{s\le\sqrt{N/d}}\sum_{r\le s}\frac N{drs}
=O(N\log^3(N+1)).$$

定义

$$\Psi_m(x)=\#\{q\le x:\operatorname{rad}(q)\mid m\},\qquad
\Psi(N)=\max_{1\le m,x\le N}\Psi_m(x).$$

上述按递增质因子、枚举幂的 DFS 访问数为 $O(\Psi_m(x))$：非平凡展开节点可以对应“当前已选乘积乘以下一质因子”，该整数的最大质因子唯一确定这一节点；正幂分支也对应唯一的光滑整数，零次幂分支至多每个展开节点一个。前缀差剪枝只会减少访问。

任取固定 $\varepsilon>0$，

$$\Psi_m(x)\le x^\varepsilon\prod_{p\mid m}(1-p^{-\varepsilon})^{-1}
\le C_\varepsilon x^\varepsilon\operatorname{rad}(m)^\varepsilon
\le C_\varepsilon N^{2\varepsilon}.$$

因为充分大的素数满足 $(1-p^{-\varepsilon})^{-1}\le p^\varepsilon$，有限多个小素数可以并入常数，所以此估计对所有 $m,x\le N$ 一致成立。由此 $\Psi(N)=N^{o(1)}$。

采用字 RAM 模型，将本范围内 64 位整数运算视为常数操作，单用例时间为

$$O(N\log^3(N+1)\Psi(N))=N^{1+o(1)},$$

空间 $O(N)$。多个用例共用一次到 $N_{\max}$ 的线性筛，时间为
$O(N_{\max}+\sum_t N_t\log^3(N_t+1)\Psi(N_t))$，空间 $O(N_{\max})$。

**这里没有证明 $O(N\log^kN)$ 的固定多对数界，也不宣称全局最优。** 次多项式渐近界不能代替本次范围的实际测时。

## 7. 范围扩大后的安全条件与验证

线性筛保存最小质因子、$\mu,\omega,\tau,D,\mathcal M$ 到 $N_{\max}\le10^6$；输入总最大值受 $2\times10^6$ 约束。$drs$ 可能由多个数相乘，循环条件先转成 64 位，确认不超过 $N$ 才索引。短块只检查 $\mu(dst)$ 和 $\mu(rt)$；二者都不超过 $N$，不错误访问可能远大于 $N$ 的 $\mu(drst)$。

在此范围内 $\omega(m)\le7$，$2^{\omega(d)}\le128$，DFS 深度至多 $7$；枚举质数幂时先检查 `power > r / p` 再乘，避免溢出。$ijk$ 在数学上可达 $10^{18}$，标程不筛到或枚举该乘积。$\tau$ 的递推使用精确整数除法，没有模逆元前提。模乘与累加按阶段取模并使用 64 位，三项模乘结果之和小于 $3(10^9+7)^2<2^{63}-1$。位掩码对 64 位满字单独处理，不执行未定义的左移 64 位；位索引为 0 到 63。位集加速的真实峰值内存与 256 MB 限制检查结果见测时记录。

`reference.cpp` 独立普通筛并逐个枚举第四修正因子，不进行整除分块；`brute.cpp` 直接筛到 $ABC$ 再三重枚举。新标程实际验证包括 $A,B,C\le12$ 的全部 $1728$ 组有序参数、随机小输入、较大参数的参考对照、三个不同上界的六种排列以及单位边的独立 $D(N)=\sum_{d\le N}\lfloor N/d\rfloor$ 公式。还专门测试了 63/64/65、127/128/129、2047/2048/2049 等位字和截止点边界。具体执行记录和 sanitizer 检查见 `records/`。

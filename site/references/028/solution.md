# 单点与双点：题解与实现审计

本题改编自 [QOJ #17233「新年的神谕」](https://qoj.ac/problem/17233)。本包沿用用户提供的多项式递推算法，范围为 $n\le100$；标准程序现场计算所有系数，时间 $O(n^4)$，空间 $O(n^3)$。这是一份已实现、已本地验证的算法，不宣称是理论最优或全局最优。

## 1. 用多项式记录每个位置的答案

记

$$M=\frac{n(n+1)}2,\qquad m=n-1,\qquad d=M-1.$$

下标 $k\in[0,m]$ 对应单点卡片 $\{k+1\}$。用 $a_{k,q}$ 表示该卡片位于第 $q+1$ 位的方案数。

给每张卡片赋一个 $[0,1]$ 内的实数，按实数排序。相等的情形体积为零。固定目标单点的实数为 $t$。对于一个将其放在第 $q+1$ 位的排列，前面 $q$ 个实数有序地落在 $(0,t)$，后面 $d-q$ 个有序地落在 $(t,1)$，对应体积为

$$\frac{t^q(1-t)^{d-q}}{q!(d-q)!}.$$

因此合法区域的切片体积是

$$F_k(t)=\sum_{q=0}^{d}a_{k,q}\frac{t^q(1-t)^{d-q}}{q!(d-q)!}.$$

固定单点的全部实数后，每张双点卡片独立地位于两个端点之间。积分掉这些变量，得到

$$F_k(t)=\int_{\Omega_k(t)}W\,d\mathbf x,$$

其中

$$\Omega_k(t):0<x_1<\cdots<x_k<t<x_{k+1}<\cdots<x_m<1,$$

$$W=\prod_{a<b}(x_b-x_a)\prod_a|t-x_a|.$$

作分式变换

$$G_k(z)=(1+z)^dF_k\!\left(\frac z{1+z}\right).$$

现在基底变成普通幂：

$$G_k(z)=\sum_{q=0}^{d}\frac{a_{k,q}}{q!(d-q)!}z^q.$$

只要求出各个 $G_k$，逐项乘上 $q!(d-q)!$ 就得到答案。

## 2. 不同的最低次数与最高次数

定义

$$L_k=\frac{k(k+3)}2,\qquad R_k=d-L_{m-k}.$$

目标单点之前，必须放入前 $k$ 个单点和端点均不超过 $k+1$ 的 $\binom{k+1}{2}$ 张双点卡片。因此 $q\ge L_k$。

将所有数字 $x$ 改成 $n+1-x$，同时反转排列，合法性不变。这给出 $q\le R_k$ 以及

$$a_{k,q}=a_{m-k,d-q}.$$

于是 $G_k$ 的系数仅可能在 $[L_k,R_k]$ 内非零。这两组边界都严格递增：

$$L_0<\cdots<L_m,\qquad R_0<\cdots<R_m.$$

下面还会算出端点系数，证明它们非零。

## 3. 三项微分递推

对固定的区域 $\Omega_k(t)$，引入辅助多项式

$$J_s(t)=\binom ms^{-1}\int_{\Omega_k(t)}W\,e_s(t-x_1,\ldots,t-x_m)\,d\mathbf x,
\quad 0\le s\le m,$$

其中 $e_s$ 是 $s$ 次基本对称多项式，$J_0=F_k$。定义

$$D_s=\frac{s(m-s+4)}2,\qquad K_s=\frac{(m-s)(2m+4-s)}2.$$

这些辅助多项式满足

$$t(t-1)J_s'=(2t-1)L_{m-s}J_s+D_st(t-1)J_{s-1}-K_sJ_{s+1}.\tag{1}$$

越界的 $J_{-1},J_{m+1}$ 视为零。

式 (1) 可由分部积分得到：对所有变量，将

$$\frac{\partial}{\partial x_a}\left[x_a(1-x_a)W\,
e_s(t-x_1,\ldots,\widehat{t-x_a},\ldots,t-x_m)\right]$$

求和后积分。帽子表示删去这一项。端点 $0,1$ 上的 $x_a(1-x_a)$ 为零，两个变量碰撞时 Vandermonde 因子为零，变量到达 $t$ 时 $W$ 也为零，所以边界项消失。利用基本对称多项式的展开与求导恒等式，可将结果整理成 (1)。

也可直接核对 Forrester、Kumar 的 [Jacobi 系综积分微分递推，§2.2 式 (2.13)–(2.14)](https://arxiv.org/html/2402.16069v1)：取 $N=m$、$\lambda_1=\lambda_2=0$、$\alpha=\beta=1$，其系数分别为 $A_s=2L_{m-s}$、$B_s=-L_{m-s}$、$D_s$ 和 $(m-s)E_s=K_s$。固定有 $k$ 个变量小于 $t$ 的积分区域不会改变该恒等式。

令

$$Q_s(z)=(1+z)^{d+s}J_s\!\left(\frac z{1+z}\right),\qquad b_{s,q}=[z^q]Q_s(z).$$

代入 (1) 并比较系数得到

$$\begin{aligned}
(q-L_{m-s})b_{s,q}={}&(d+s-L_{m-s}-q+1)b_{s,q-1}\\
&+D_sb_{s-1,q-1}+K_sb_{s+1,q}.
\end{aligned}\tag{2}$$

按 $q$ 从小到大、$s$ 从大到小计算，右侧的系数均已得到。辅助次数只需要保留相邻两层。我们只需要 $Q_0=G_k$ 的 $0\ldots d$ 次系数，因此不需要继续计算辅助多项式的更高次部分。

## 4. 在所有解中构造一组基

式 (2) 在 $q=L_{m-s}$ 处不能直接除以左侧系数。这些位置恰好有 $n$ 个。

先在有理数域考虑截至次数 $d$ 的方程组。除这 $n$ 个位置外，所有未知数都由较早的系数唯一确定，因此解空间维数至多为 $n$。另一方面，真实的 $n$ 组积分给出 $n$ 个解；其 $Q_0=G_k$ 的最低次数不同，最低项非零，所以它们线性无关，解空间维数至少为 $n$。

因此解空间维数恰为 $n$。这也证明：在这 $n$ 个特殊位置指定任意自由量，其余位置按递推计算即可；特殊位置的右侧约束会自动满足，不能仅凭“分母为零”就未经证明地忽略它。

要构造第 $k$ 个基 $B_k$，仅将第 $k$ 个自由量设成适当的非零值，其他自由量都设为零。它此前的系数全为零，因此若将最低项归一化为与 $G_k$ 相同，就有

$$B_k=G_k+\sum_{r>k}c_{k,r}G_r.\tag{3}$$

## 5. 计算最低项系数和自由量

使用以下 Selberg 积分特例：

$$\begin{aligned}
I(s,a)&=\int_{0<u_1<\cdots<u_s<1}\prod_{i<j}(u_j-u_i)\prod_i u_i^a\,d\mathbf u\\
&=\prod_{i=1}^s\frac1{a+i}\prod_{1\le i<j\le s}\frac{j-i}{2a+i+j}.
\end{aligned}$$

规定 $I(0,a)=1$。这里只需要整数 $a\ge0$ 的有理数乘积式；程序不进行实数积分。

在 $F_k$ 中令左侧 $k$ 个变量 $x_i=tu_i$，取 $t$ 的最低项。左侧积分含有 $\prod_i(1-u_i)$，对变量反射并反转次序后，它等于 $I(k,1)$；右侧每个变量从与左侧的距离及其到 $t$ 的距离得到 $x^{k+1}$。所以

$$C_k=[z^{L_k}]G_k=I(k,1)I(m-k,k+1).\tag{4}$$

每个因子非零，$C_k\ne0$。反转对称性给出

$$[z^{R_k}]G_k=C_{m-k}.\tag{5}$$

在第 $k$ 个基首次出现非零系数的次数 $q=L_k$ 上，上一层系数全为零。令 $r=m-s$，式 (2) 从 $r=k+1$ 到 $m$ 变成

$$b_{m-r,L_k}=\frac{r(m+r+4)}{(k-r)(k+r+3)}b_{m-r+1,L_k}.$$

因此在自由位置设置

$$b_{m-k,L_k}=C_k\prod_{r=k+1}^{m}\frac{(k-r)(k+r+3)}{r(m+r+4)}\tag{6}$$

即可保证 $b_{0,L_k}=C_k$。程序将 (6) 记作 `seed[k]`，将所有 $n$ 个基一起递推。

## 6. 从最高项开始消元

按 $k=m,m-1,\ldots,0$ 处理。假定编号大于 $k$ 的 $G_r$ 已求出，由 (3)，只需消去它们的分量。

再按 $r=m,m-1,\ldots,k+1$ 处理：

$$B_k\leftarrow B_k-\frac{[z^{R_r}]B_k}{C_{m-r}}G_r.$$

由于 $R_r$ 严格递增，编号小于 $r$ 的目标多项式不可能贡献 $z^{R_r}$ 项，而编号大于 $r$ 的分量已经消去。由 (5)，这一步恰好消掉 $G_r$ 的分量。最终得到 $B_k=G_k$。

输出

$$a_{k,q}=[z^q]B_k\cdot q!(d-q)!\pmod p.$$

至此，体积计数、微分恒等式、自由量构造和消元依次证明标准程序的正确性。

## 7. 扩到 n=100 的实现前提

- $M\le5050$，$d\le5049$。代码按输入动态分配 $B[n][M]$ 和两个 $(n+2)\times n$ 的辅助层。
- 所有实际访问的小整数逆元下标至多为 $\max(M,2n+2)\le5050$。$p>10^8$ 且为质数，这些数在模 $p$ 下均可逆。分母 $q-L_{m-s}=0$ 的位置按自由量处理；负分母使用对应负逆元。
- $I$ 的分子、分母和式 (6) 中的非零因子都小于 $p$，所以 $C_k$ 不会变成零。$q!(d-q)!$ 也可逆，真实多项式的最低项在模 $p$ 后仍非零。前述基底与消元论证在此域中仍成立。辅助积分的有理系数可由多项式逐项积分得到，其小整数分母也远小于 $p$。
- 模数不是固定常量，不假定存在适合 NTT 的单位根；不使用 NTT、浮点数或高精度整数。实积分只用于证明。
- 乘法在 `long long` 中进行，单个模乘积小于 $10^{18}$。递推右侧三个乘积之和小于 $3\times10^{18}<2^{63}-1$。所有数组下标、次数和未经取模的小系数也在 `int` 范围内。
- `n=1` 时 $m=d=0$，空积为 $1$，逆元数组至少开到 $4$；输出恰为 `1`。标准程序无递归。

不能只扩大一个静态数组上限：逆元范围、消元主元非零性和三项乘积总和都需要上述检查。

## 8. 真实复杂度

预计算全部 $C_k$ 的乘积式为 $O(n^3)$ 次模运算。构造基有 $M=\Theta(n^2)$ 个次数、$n$ 个辅助状态和 $n$ 个基，共 $O(n^4)$ 次模运算；消元有 $O(n^2)$ 次，每次至多处理 $M$ 个系数，也是 $O(n^4)$。输出 $nM=\Theta(n^3)$ 个整数。

存储 $nM$ 个基系数，外加 $O(n^2)$ 个滚动状态，空间为 $O(n^3)$。

这里采用固定 64 位机器整数运算为常数时间的 RAM 模型，适用于题目给定的 $n$ 与 $p$ 上界。若将模数扩为任意大整数，必须额外计算乘除法的位复杂度。本包没有将此算法称为渐近最优；显式输出本身仅给出 $\Omega(n^3)$ 的下界。

## 9. 验证方法

`src/brute.cpp` 不使用上述微分递推：直接构建卡片的先后约束图，枚举所有能作为前缀的偏序理想。前向 DP 计算到达该前缀的方法数，后向 DP 计算余下部分的完成数；每条放入目标单点的转移，将二者乘积加到对应位置。它给出小规模完整矩阵的独立答案。

较大规模还检查形状、模数范围、支持区间、反转对称性，以及每一行的和等于总排列数。总数单独使用 Python 整数精确计算

$$T_n=M!\,I(n,0)=\frac{M!\prod_{i<j}(j-i)}{n!\prod_{i<j}(i+j)},$$

验证整除后再对 $p$ 取模。这种检查覆盖完整输出的若干必要性质，但不能替代大规模完整矩阵的独立对拍。

本次实际执行结果、计时记录及覆盖范围见 `validation_report.md` 和 `logs/`。用户原始材料中的历史测试声称仅作为原始记录保留，本包的“通过”均依据本次重新运行的日志。

## 10. C++17 标准程序

下列代码与 `src/std.cpp` 完全一致，沿用用户提供的现场递推实现。

```cpp
#include <bits/stdc++.h>
using namespace std;

using i64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, P;
    cin >> n >> P;

    const int m = n - 1;
    const int M = n * (n + 1) / 2;
    const int d = M - 1;
    const int lim = max(M, 2 * n + 2);

    // 所有需要的小整数逆元。
    vector<int> inv(lim + 1);
    inv[1] = 1;
    for (int i = 2; i <= lim; ++i) {
        inv[i] = P - (i64)(P / i) * inv[P % i] % P;
    }

    vector<int> L(n);
    for (int k = 0; k <= m; ++k) {
        L[k] = k * (k + 3) / 2;
    }

    // 同时返回 I(s,a) 和它的逆元。
    // 直接分别累乘分子、分母，不需要快速幂。
    auto integral = [&](int s, int a) -> pair<int, int> {
        i64 v = 1, iv = 1;

        for (int i = 1; i <= s; ++i) {
            v = v * inv[a + i] % P;
            iv = iv * (a + i) % P;

            for (int j = i + 1; j <= s; ++j) {
                int numerator = j - i;
                int denominator = 2 * a + i + j;

                v = v * numerator % P;
                v = v * inv[denominator] % P;

                iv = iv * denominator % P;
                iv = iv * inv[numerator] % P;
            }
        }

        return {(int)v, (int)iv};
    };

    // C[k]：G_k 的最低次项系数。
    // seed[k]：构造第 k 个基时，唯一非零的自由量。
    vector<int> C(n), invC(n), seed(n);

    for (int k = 0; k <= m; ++k) {
        auto [a, ia] = integral(k, 1);
        auto [b, ib] = integral(m - k, k + 1);

        C[k] = (i64)a * b % P;
        invC[k] = (i64)ia * ib % P;

        i64 v = C[k];

        for (int r = k + 1; r <= m; ++r) {
            v = v * (P + k - r) % P;
            v = v * (k + r + 3) % P;
            v = v * inv[r] % P;
            v = v * inv[m + r + 4] % P;
        }

        seed[k] = (int)v;
    }

    vector<int> D(n), K(n);

    for (int s = 0; s <= m; ++s) {
        D[s] = (i64)s * (m - s + 4) % P * inv[2] % P;

        K[s] = (i64)(m - s) * (2 * m + 4 - s)
             % P * inv[2] % P;
    }

    // B[k][q]：第 k 个基多项式的 z^q 系数。
    vector<vector<int>> B(n, vector<int>(M));

    // 辅助下标 s 存在 s+1 行。
    // 第 0 行和第 n+1 行作为全零哨兵。
    //
    // pre[s+1][k]：第 k 个基，上一层次数的辅助状态 s。
    // cur[s+1][k]：第 k 个基，当前层次数的辅助状态 s。
    vector<vector<int>> pre(n + 2, vector<int>(n));
    vector<vector<int>> cur(n + 2, vector<int>(n));

    for (int q = 0; q <= d; ++q) {
        for (auto &row : cur) {
            fill(row.begin(), row.end(), 0);
        }

        // 当前层依赖 s+1，因此 s 必须倒序。
        for (int s = m; s >= 0; --s) {
            int r = m - s;
            int u = s + 1;

            if (q == L[r]) {
                // 特殊位置：自由量。
                // 只有第 r 个基在这里取非零值。
                cur[u][r] = seed[r];
                continue;
            }

            int delta = q - L[r];
            int id = delta > 0
                   ? inv[delta]
                   : P - inv[-delta];

            int a = d + s - L[r] - q + 1;
            if (a < 0) a += P;

            for (int k = 0; k <= m; ++k) {
                // P < 1e9，三个乘积之和不会溢出 long long。
                i64 rhs = (i64)a * pre[u][k]
                        + (i64)D[s] * pre[u - 1][k]
                        + (i64)K[s] * cur[u + 1][k];

                cur[u][k] = (rhs % P) * id % P;
            }
        }

        // 只保存辅助状态 s=0，它就是所需基多项式。
        for (int k = 0; k <= m; ++k) {
            B[k][q] = cur[1][k];
        }

        pre.swap(cur);
    }

    // 倒序消元。
    // 处理完 k 后，B[k] 就是真正的目标多项式 G_k。
    for (int k = m; k >= 0; --k) {
        for (int r = m; r > k; --r) {
            int right = d - L[m - r];

            // G_r 的最高次项系数为 C[m-r]。
            int factor = (i64)B[k][right] * invC[m - r] % P;

            if (factor == 0) continue;

            // 已求出的 G_r 只在 [L[r], right] 内非零。
            for (int q = L[r]; q <= right; ++q) {
                int sub = (i64)factor * B[r][q] % P;

                B[k][q] -= sub;
                if (B[k][q] < 0) B[k][q] += P;
            }
        }
    }

    vector<int> fact(M + 1, 1);

    for (int i = 1; i <= M; ++i) {
        fact[i] = (i64)fact[i - 1] * i % P;
    }

    // 单点 {k+1} 位于第 q+1 位。
    for (int k = 0; k <= m; ++k) {
        for (int q = 0; q <= d; ++q) {
            int ans = (i64)B[k][q] * fact[q] % P
                    * fact[d - q] % P;

            if (q) cout << ' ';
            cout << ans;
        }
        cout << '\n';
    }

    return 0;
}

```

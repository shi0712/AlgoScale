# AlgoScale 001–074 题解重写说明

本次覆盖目录中全部 **74 道题**，逐题对照新的 `algoscale-statement.md`、原 `solution.md` 与当前 `std.cpp` 重写中文题解。来源快照为 [`shi0712/AlgoScale@dbdd36f`](https://github.com/shi0712/AlgoScale/tree/dbdd36f5d7312da6e0ba003ebdc7aae14b03c05d)。

正文保存在原来的 `site/references/NNN/solution.md` 路径中。每篇均说明问题转化、算法步骤、关键正确性依据、按当前实现计算的复杂度，以及新范围下的边界和数值类型；交互题另行解释操作次数上界。

## 后续代码修复状态

根据后续的代码修复要求，015 已增加精确线性空间判等，061 已修复负下标访问；048 的全范围构造仍未解决。实现、复杂度和验证记录见 [标准代码修复说明](STANDARD_CODE_FIXES.md)。以下保留最初发现的问题，并标注当前处理结果。

## 最初发现的三个问题

以下问题来自原有材料。015、061 已在后续提交中修改代码；048 仍保留明确的适用范围说明。

| 编号 | 问题 | 新题解的处理 |
| --- | --- | --- |
| 015 Spirited Away | 空间链维护是精确的，但最终用硬编码的 128 位指纹去重，不能保证任意输入下无碰撞。 | 已修复代码：桶键只加速分组，最终使用精确线性空间判等，真实碰撞回退到完整规范基去重。 |
| 048 Iroha and the Kingdom of Construction | “存在至多 180 栈的方案”不能保证当前连续分桶构造也只用 180 栈。存在可用 20 栈解决、但标程正反两次都需要 181 栈并退出的合法输入。 | 完整解释条件成立时的构造与界限，并给出反例结构。未把该程序描述成覆盖当前全部合法输入的标准解。 |
| 061 Matching | 计算零合并概率时，先访问 `ifac[m-r]`，后检查 `m-r<0`，存在负下标访问。 | 已修复代码：先处理 `r>m`，再访问合法的逆阶乘下标；82 种初始分组与独立前向 Markov 链一致。 |

### 048 的可复现反例

依次拼接 180 个互不共享点数的牌块。第 $b$ 块使用点数 $180b+1,\ldots,180b+180$（$0\le b<180$），先正序出现一次，再逆序出现一次。总计 $n=32400$ 种点数、64800 张牌。

每块可以独立使用 20 栈：连续分桶的容量为 $19,18,\ldots,1$，合计 190，足以覆盖 180 个递减的第二次出现位置。各块复用同一组栈，从最后一块开始清空，整体仍只需 20 栈。当前标程对整段输入的正反向构造都需要 181 栈，实测退出码为 2，且没有输出。

### 061 的可复现越界

使用题面样例：

```text
1
2 1
1 1
```

将原 `std.cpp` 以 `-D_GLIBCXX_ASSERTIONS` 编译运行，标准库的下标断言失败。初始状态为 `r=2m`，此时 `m-r` 为负；不能先访问数组，再把结果置零。后续修复代码已使用正确顺序，当前版本不再触发该断言。

## 复杂度和范围的主要修正

| 编号 | 修正 |
| --- | --- |
| 002 | 普通大 O 应保留多项式因子：$O(n^2 6^{n/2}+m\log m)$；省略因子时应使用 $O^*$ 记号。 |
| 021 | 保留实际求和式和 $N^{1+o(1)}$ 级别的限定，不将其无条件写成固定幂次的对数因子。 |
| 023 | 随机增量部分为期望 $O(n\log n)$，不是确定性最坏界。 |
| 040 | Pollard–Rho 使用启发式期望分析，并计入实现中的运算开销。 |
| 052 | 当前标程使用单调分治求凹序列卷积，具有对数因子，不能套用目录中无对数的复杂度。 |
| 058 | 新题面数值上限为 $10^6$。正文按当前倍数求和实现写成 $O(N+M\log M+Q)$。 |
| 059 | $O(n^2)$ 指大整数算术操作次数；正文另说明位数增长和大整数运算代价。 |
| 062 | 显式倍增与区间 DAG 的空间为 $O((n+m)\log n)$，不能写成线性。 |
| 066 | 保留 $W=0$ 时仍存在的状态，并计入平行边的存储。 |

此外，019 修正了原题解错误的常数项；030 按当前代码的 `j<n` 边界解释，没有沿用旧文关于历史版本负下标的描述；037 使用适用于所有合法方案的段数上界；044、068、072 补全了原文依赖缺失外部材料的论证或复现过程。

## 重点补全的证明

- 052：部分强化收益序列的凹性，从整数费用流给出证明。
- 053：预付返程费用的账本如何安排成真实行程，固定组数费用的 Monge 交换，以及完全背包最终周期和周期边界的补枚举。
- 054：区分“最左最右点同属一个强连通分量”和“全部选点强连通”，再通过唯一核心分解和区间权值消除多计。
- 055：先证明候选区间与真实字符串操作等价，再用 $5+3\to4+4$ 的交换证明凹性，最终推出只需枚举六个罚值。没有沿用“加一个预算行后区间矩阵仍然连续 1”的错误推理。
- 059、062、064、067：补全组合计数、交换判据、DP 优化及候选缩减的关键中间步骤。
- 068–074：对照交互协议说明状态含义、策略构造与最坏操作次数。

## 已完成的针对性验证

这些检查用于核对最容易出错的推导，不等同于全部标程通过官方隐藏评测。

| 范围 | 检查结果 |
| --- | --- |
| 全部 74 篇 | 标题与目录对应；原路径完整；Markdown 代码围栏和数学定界符配对；6978 处数学表达式通过 KaTeX 0.16.11 解析；`git diff --check` 通过。 |
| 018 | 28 组完整数组枚举，以及 27 组模边界闭式核对通过。 |
| 026 | 1544 组预算参数全部可行，上界最大为 27；特殊编码的 65536 个子集和互不相同。 |
| 044 | 检查 10660 个构造参数组合的尺寸上界。 |
| 045 | 枚举所有 1–13 点无标号树及各步直径选择，最短、最长折叠次数一致。 |
| 051 | 120 个小规模字符串与完整填法枚举一致。 |
| 052 | 150 个小规模实例与所有排列枚举一致。 |
| 053 | 160 个实例与完整火把状态最短路一致；500 组强制小容量分支与大容量分支一致；20000 组随机账本的逐人调度检查通过。 |
| 054 | 200 个实例与全部非空子集的强连通判定一致。 |
| 055 | 2716 个字符串、8386 个预算组合与真实合法操作的穷举一致。 |
| 059 | 45 组小规模全枚举一致；最大新范围下检查了结果的有限性、单调性和可闭式计算的最后一项。 |
| 060 | 2666 组小参数与完整候选枚举一致。 |
| 062 | 500 组随机图对与枚举全部生成树一致。 |
| 067 | 3000 个实例的稀疏候选 DP 与穷举选人一致。 |
| 068 | 233 个支持集、126118 个规范化状态的独立模拟通过，查询上界与正文一致。 |
| 072 | 完整层宽 DP 得到最大对齐右端点 1593674，小于 $2^{21}$。 |

原仓库的 `tests/test_reference_assets.py` 仍硬编码 85 个题目，与当前实际 74 题目录不符。本次以实际目录逐项检查完整性，没有把这一过期断言的失败当作题解缺失，也没有声称全部现有测试通过。

## 页面公式兼容

现有页面先执行 Markdown 解析、再执行 KaTeX，Markdown 会吞掉 TeX 的部分反斜杠，影响集合花括号、矩阵换行等表达式。本次在 `site/reference.js` 中加入数学 token 扩展，先保留公式原文，再沿用 HTML 清理和 KaTeX 渲染；并更新 `reference.html` 的脚本缓存版本。

使用网站实际版本 marked 15.0.12、KaTeX 0.16.11 检查全部 74 篇：源公式 6978 处，保护后的公式 token 6978 处，逐公式原文、HTML 转义往返和 KaTeX 解析均一致。另检查了代码块、行内代码、四种数学定界符、链接、强调、列表、引用、表格及 `<>&` 字符。运行环境没有可用的浏览器二进制，未将这些检查描述为浏览器截图验收。

## 74 题索引

以下链接均指向本次重写后的正文。015、061 标记为代码已修复，048 仍存在全范围算法缺口；“已重写”不表示完成了官方评测。

| 编号 | 题目及新版题解 | 状态 |
| --- | --- | --- |
| 001 | [Night Sky](site/references/001/solution.md) | 已重写 |
| 002 | [Years](site/references/002/solution.md) | 已重写 |
| 003 | [Seal](site/references/003/solution.md) | 已重写 |
| 004 | [Value of a Tree](site/references/004/solution.md) | 已重写 |
| 005 | [Industrial System](site/references/005/solution.md) | 已重写 |
| 006 | [Infinite Strife,](site/references/006/solution.md) | 已重写 |
| 007 | [Arc Path](site/references/007/solution.md) | 已重写 |
| 008 | [Mountain Climbing](site/references/008/solution.md) | 已重写 |
| 009 | [Heuristic Knapsack](site/references/009/solution.md) | 已重写 |
| 010 | [Play It by Ear](site/references/010/solution.md) | 已重写 |
| 011 | [Square Kingdom](site/references/011/solution.md) | 已重写 |
| 012 | [DFS Order 4](site/references/012/solution.md) | 已重写 |
| 013 | [Coloring](site/references/013/solution.md) | 已重写 |
| 014 | [Rikka with Lake](site/references/014/solution.md) | 已重写 |
| 015 | [Spirited Away](site/references/015/solution.md) | 已增加精确判等并修复代码 |
| 016 | [Balanced Rainbow Sequence](site/references/016/solution.md) | 已重写 |
| 017 | [J The Attacker Has](site/references/017/solution.md) | 已重写 |
| 018 | [EI Gave Me a Zero](site/references/018/solution.md) | 已重写 |
| 019 | [Euler? Euler!](site/references/019/solution.md) | 已重写 |
| 020 | [Increasing Tree Sequence](site/references/020/solution.md) | 已重写 |
| 021 | [Old Problems](site/references/021/solution.md) | 已重写 |
| 022 | [Quad Kingdoms Chess 2](site/references/022/solution.md) | 已重写 |
| 023 | [Military Maneuver](site/references/023/solution.md) | 已重写 |
| 024 | [Queue Sorting](site/references/024/solution.md) | 已重写 |
| 025 | [New Year Oracle](site/references/025/solution.md) | 已重写 |
| 026 | [Ball Machine](site/references/026/solution.md) | 已重写 |
| 027 | [Pico Park](site/references/027/solution.md) | 已重写 |
| 028 | [Product, Euler Totient, and Sum](site/references/028/solution.md) | 已重写 |
| 029 | [Median Replacement](site/references/029/solution.md) | 已重写 |
| 030 | [Rolling For Days](site/references/030/solution.md) | 已重写 |
| 031 | [Inverted](site/references/031/solution.md) | 已重写 |
| 032 | [Discrete Fourier Transform](site/references/032/solution.md) | 已重写 |
| 033 | [Panda Preserve](site/references/033/solution.md) | 已重写 |
| 034 | [Match](site/references/034/solution.md) | 已重写 |
| 035 | [Numbers on a Tree](site/references/035/solution.md) | 已重写 |
| 036 | [Road Repair](site/references/036/solution.md) | 已重写 |
| 037 | [Magic Tiles](site/references/037/solution.md) | 已重写 |
| 038 | [Paired Bracket Sequences](site/references/038/solution.md) | 已重写 |
| 039 | [Steadily Growing Steam](site/references/039/solution.md) | 已重写 |
| 040 | [Sum and Product](site/references/040/solution.md) | 已重写 |
| 041 | [Tree Transform](site/references/041/solution.md) | 已重写 |
| 042 | [Maximum](site/references/042/solution.md) | 已重写 |
| 043 | [Edge Coloring Problem](site/references/043/solution.md) | 已重写 |
| 044 | [World Map](site/references/044/solution.md) | 已重写 |
| 045 | [Folding Game of Ohto Ai](site/references/045/solution.md) | 已重写 |
| 046 | [Bitaro the Brave 3](site/references/046/solution.md) | 已重写 |
| 047 | [Ambulance](site/references/047/solution.md) | 已重写 |
| 048 | [Iroha and the Kingdom of Construction](site/references/048/solution.md) | 180 栈保证缺口已说明 |
| 049 | [Sort](site/references/049/solution.md) | 已重写 |
| 050 | [Las Vegas](site/references/050/solution.md) | 已重写 |
| 051 | [Athlete Welcome Ceremony](site/references/051/solution.md) | 已重写 |
| 052 | [No Bug No Game](site/references/052/solution.md) | 已重写 |
| 053 | [Bridging the Gap](site/references/053/solution.md) | 已重写 |
| 054 | [Under the Epilogue](site/references/054/solution.md) | 已重写 |
| 055 | [Magical String](site/references/055/solution.md) | 已重写 |
| 056 | [Angle Beats](site/references/056/solution.md) | 已重写 |
| 057 | [Apple Tree](site/references/057/solution.md) | 已重写 |
| 058 | [Cakes](site/references/058/solution.md) | 已重写 |
| 059 | [Double Sort](site/references/059/solution.md) | 已重写 |
| 060 | [Staple Stable](site/references/060/solution.md) | 已重写 |
| 061 | [Matching](site/references/061/solution.md) | 标程越界已修复 |
| 062 | [Two Spanning Trees](site/references/062/solution.md) | 已重写 |
| 063 | [Extending Distance](site/references/063/solution.md) | 已重写 |
| 064 | [Elevator Requests IV](site/references/064/solution.md) | 已重写 |
| 065 | [Affection](site/references/065/solution.md) | 已重写 |
| 066 | [Walk Variance](site/references/066/solution.md) | 已重写 |
| 067 | [Tennis Match](site/references/067/solution.md) | 已重写 |
| 068 | [The Last Singer](site/references/068/solution.md) | 已重写 |
| 069 | [Exchanging Kubic](site/references/069/solution.md) | 已重写 |
| 070 | [Slot Machine](site/references/070/solution.md) | 已重写 |
| 071 | [Inversion](site/references/071/solution.md) | 已重写 |
| 072 | [Menji, We Miss You!](site/references/072/solution.md) | 已重写 |
| 073 | [Where Am I Now?](site/references/073/solution.md) | 已重写 |
| 074 | [Radio Direction Finding](site/references/074/solution.md) | 已重写 |

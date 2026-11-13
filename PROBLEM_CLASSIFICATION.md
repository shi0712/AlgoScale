# AlgoScale 全部 74 题分类

分类版本：1.0。依据新题面、重写题解和当前参考程序，源版本为 [`2a9b05d`](https://github.com/shi0712/AlgoScale/tree/2a9b05d19cc1961d87350ebe3ad4205436c9820d)。

每题一个主领域用于互斥统计，关联领域用于交叉检索。核心标签描述决定解法的方法或必要模型，辅助标签描述实际使用的重要工具；证明中借用但未实际运行的算法不自动作为实现标签。

**048 的完整算法尚未覆盖新题面全部合法输入，其解法标签为候选。其余“已核对标注”表示分类与指定材料一致，不代表一次新的官方评测认证。**

数据文件：[完整 JSON 与统一词表](site/data/problem-classification.json)、[CSV 分类表](site/data/problem-classification.csv)。JSON 按 `id` 关联现有 `site/data/problems.json`，不把 `All` 存成标签。

## 主领域统计

| 主领域 | 英文名称 | 主领域题数 | 含关联领域的覆盖题数 |
| --- | --- | ---: | ---: |
| 算法策略 | Algorithmic Paradigms | 9 | 23 |
| 计算几何 | Computational Geometry | 4 | 7 |
| 数据结构 | Data Structures | 5 | 16 |
| 动态规划 | Dynamic Programming | 14 | 26 |
| 图论 | Graph Theory | 10 | 24 |
| 数学 | Mathematics | 28 | 39 |
| 搜索与枚举 | Search & Enumeration | 4 | 7 |
| 字符串算法 | String Algorithms | 0 | 0 |

合计 **74** 题，其中 **66** 道标准题、**8** 道交互题。主领域数量合计为 74；覆盖题数包含重复归属，不能相加。统一词表有 **161** 个细标签，包含必要的上位/下位专题；每题使用 2–5 个标签。

交互题：026、068、069、070、071、072、073、074。当前解法没有以 KMP、Z、AC 自动机、后缀结构或 Manacher 为核心的题目，不以题面出现字符串为由填充字符串算法类别。

## 全部题目

领域列先列主领域，随后列关联领域；标签列只显示核心项。辅助标签、逐标签依据、题目对象和源文件校验值保存在完整 JSON。048 的星号表示候选解法。

### 001–025

| 编号 | 题目 | 主领域及关联领域 | 核心标签 | 形式 / 任务 |
| --- | --- | --- | --- | --- |
| 001 | [Night Sky](site/references/001/solution.md) | 动态规划 + 图论 | 序列 DP、欧拉迹 | 标准 / 判定、构造 |
| 002 | [Years](site/references/002/solution.md) | 数学 + 图论 + 动态规划 | 最小生成树、容斥原理、子集卷积 | 标准 / 概率 |
| 003 | [Seal](site/references/003/solution.md) | 数据结构 + 动态规划 | 动态树形 DP、矩阵乘积、平衡表达式树 | 标准 / 计数 |
| 004 | [Value of a Tree](site/references/004/solution.md) | 动态规划 + 算法策略 | 树形 DP、长链剖分、根号分治与分块 | 标准 / 最优化 |
| 005 | [Industrial System](site/references/005/solution.md) | 数据结构 + 图论 | 树同构、换根、可持久化线段树 | 标准 / 计数 |
| 006 | [Infinite Strife,](site/references/006/solution.md) | 数学 + 算法策略 | Lucas 定理、模运算、根号分治与分块 | 标准 / 求值 |
| 007 | [Arc Path](site/references/007/solution.md) | 计算几何 + 图论 | 扫描线、圆几何、图建模、Dijkstra 最短路 | 标准 / 最优化 |
| 008 | [Mountain Climbing](site/references/008/solution.md) | 算法策略 + 数学 | 贪心、拉格朗日松弛、离散凸凹性 | 标准 / 最优化 |
| 009 | [Heuristic Knapsack](site/references/009/solution.md) | 算法策略 + 数据结构 | 贪心、构造算法、线段树 | 标准 / 判定、构造 |
| 010 | [Play It by Ear](site/references/010/solution.md) | 数学 + 算法策略 | 概率与期望、分治、多项式插值、NTT 数论变换 | 标准 / 期望、最优化 |
| 011 | [Square Kingdom](site/references/011/solution.md) | 算法策略 + 数学 | BFPRT 线性选择、剪枝与搜索 | 标准 / 顺序统计 |
| 012 | [DFS Order 4](site/references/012/solution.md) | 数学 | 生成函数、形式幂级数、NTT 数论变换 | 标准 / 计数 |
| 013 | [Coloring](site/references/013/solution.md) | 动态规划 + 数据结构 + 图论 | 树形 DP、功能图、平衡搜索树、线段树 | 标准 / 最优化 |
| 014 | [Rikka with Lake](site/references/014/solution.md) | 计算几何 | 多边形三角剖分、几何最短路、几何积分 | 标准 / 求值 |
| 015 | [Spirited Away](site/references/015/solution.md) | 数学 + 数据结构 | 异或线性基、线性化多项式、有限域 | 标准 / 计数 |
| 016 | [Balanced Rainbow Sequence](site/references/016/solution.md) | 算法策略 + 数学 | 贪心、离散凸凹性、二分搜索 | 标准 / 最优化、判定 |
| 017 | [J The Attacker Has](site/references/017/solution.md) | 数学 + 动态规划 + 图论 | 博弈论、二分图匹配、状压动态规划 | 标准 / 计数 |
| 018 | [EI Gave Me a Zero](site/references/018/solution.md) | 数学 | 容斥原理、生成函数、多点求值、NTT 数论变换 | 标准 / 计数 |
| 019 | [Euler? Euler!](site/references/019/solution.md) | 数学 | 生成函数、Stirling 数、多点求值、NTT 数论变换 | 标准 / 计数 |
| 020 | [Increasing Tree Sequence](site/references/020/solution.md) | 动态规划 + 数学 | 树形 DP、容斥原理、多项式卷积 | 标准 / 计数 |
| 021 | [Old Problems](site/references/021/solution.md) | 数学 | Möbius 反演、数论分块、筛法 | 标准 / 求值 |
| 022 | [Quad Kingdoms Chess 2](site/references/022/solution.md) | 数学 + 动态规划 | 概率与期望、生成函数、线性递推 | 标准 / 概率 |
| 023 | [Military Maneuver](site/references/023/solution.md) | 计算几何 + 数学 | Voronoi 图、Delaunay 三角剖分、半平面交、几何积分 | 标准 / 期望 |
| 024 | [Queue Sorting](site/references/024/solution.md) | 数学 | RSK 对应、生成函数、形式幂级数 | 标准 / 计数 |
| 025 | [New Year Oracle](site/references/025/solution.md) | 数学 | 生成函数、线性递推、线性代数 | 标准 / 计数 |

### 026–050

| 编号 | 题目 | 主领域及关联领域 | 核心标签 | 形式 / 任务 |
| --- | --- | --- | --- | --- |
| 026 | [Ball Machine](site/references/026/solution.md) | 算法策略 + 图论 + 数学 | 树重建、组合设计 | 交互 / 重建 |
| 027 | [Pico Park](site/references/027/solution.md) | 数学 + 算法策略 | 生成函数、NTT 数论变换、多点求值、分治 | 标准 / 计数 |
| 028 | [Product, Euler Totient, and Sum](site/references/028/solution.md) | 数学 | 欧拉函数、子集变换、位掩码 | 标准 / 计数 |
| 029 | [Median Replacement](site/references/029/solution.md) | 数学 + 动态规划 + 数据结构 | 有限状态动态规划、线段树、拉格朗日插值 | 标准 / 计数 |
| 030 | [Rolling For Days](site/references/030/solution.md) | 数学 + 动态规划 | 概率与期望、泊松化、计数 DP | 标准 / 期望 |
| 031 | [Inverted](site/references/031/solution.md) | 数据结构 + 动态规划 + 图论 | 动态树形 DP、重链剖分、矩阵乘积、平衡表达式树 | 标准 / 计数 |
| 032 | [Discrete Fourier Transform](site/references/032/solution.md) | 数学 + 计算几何 | FFT 快速傅里叶变换、Bluestein 变换、凸包、凸优化 | 标准 / 最优化 |
| 033 | [Panda Preserve](site/references/033/solution.md) | 计算几何 | Delaunay 三角剖分、Voronoi 图、扫描线、点定位 | 标准 / 最优化 |
| 034 | [Match](site/references/034/solution.md) | 数学 + 图论 + 算法策略 | 生成函数、容斥原理、分治、NTT 数论变换 | 标准 / 计数 |
| 035 | [Numbers on a Tree](site/references/035/solution.md) | 图论 + 数据结构 + 算法策略 | 贪心、并查集、重链剖分、线段树 | 标准 / 最优化 |
| 036 | [Road Repair](site/references/036/solution.md) | 图论 + 数学 | 最小生成树、Kruskal 重构树、子集变换 | 标准 / 最优化 |
| 037 | [Magic Tiles](site/references/037/solution.md) | 动态规划 + 数据结构 | 序列 DP、可持久化线段树、结构唯一化 | 标准 / 最优化 |
| 038 | [Paired Bracket Sequences](site/references/038/solution.md) | 数学 + 算法策略 | 生成函数、拉格朗日反演、CDQ 分治、NTT 数论变换 | 标准 / 计数 |
| 039 | [Steadily Growing Steam](site/references/039/solution.md) | 动态规划 | 背包动态规划、状态空间缩减 | 标准 / 最优化 |
| 040 | [Sum and Product](site/references/040/solution.md) | 数学 | 欧拉函数、因子枚举、Pollard–Rho 分解 | 标准 / 计数 |
| 041 | [Tree Transform](site/references/041/solution.md) | 算法策略 + 图论 | 构造算法、规范形 | 标准 / 构造、判定 |
| 042 | [Maximum](site/references/042/solution.md) | 数学 + 动态规划 | 概率与期望、生成函数、线性递推 | 标准 / 期望、计数 |
| 043 | [Edge Coloring Problem](site/references/043/solution.md) | 图论 + 算法策略 | 边染色、Vizing 扇、Kempe 交替链 | 标准 / 最优化、构造 |
| 044 | [World Map](site/references/044/solution.md) | 算法策略 + 图论 | 构造算法、深度优先搜索 | 标准 / 构造 |
| 045 | [Folding Game of Ohto Ai](site/references/045/solution.md) | 图论 + 数据结构 | 树直径、启发式合并 | 标准 / 最优化 |
| 046 | [Bitaro the Brave 3](site/references/046/solution.md) | 数据结构 + 计算几何 + 算法策略 | 凸包、可持久化数据结构、线段树 | 标准 / 最优化 |
| 047 | [Ambulance](site/references/047/solution.md) | 动态规划 + 算法策略 | 背包动态规划、根号分治与分块 | 标准 / 判定 |
| 048* | [Iroha and the Kingdom of Construction](site/references/048/solution.md) | 算法策略 | 贪心、最长递增子序列、耐心排序 | 标准 / 构造 |
| 049 | [Sort](site/references/049/solution.md) | 数学 + 动态规划 | 概率与期望、Markov 链、概率动态规划 | 标准 / 期望 |
| 050 | [Las Vegas](site/references/050/solution.md) | 图论 + 算法策略 + 数学 | 最小费用流、凸优化、二分搜索 | 标准 / 最优化、构造 |

### 051–074

| 编号 | 题目 | 主领域及关联领域 | 核心标签 | 形式 / 任务 |
| --- | --- | --- | --- | --- |
| 051 | [Athlete Welcome Ceremony](site/references/051/solution.md) | 数学 | 生成函数、NTT 数论变换、容斥原理 | 标准 / 计数 |
| 052 | [No Bug No Game](site/references/052/solution.md) | 动态规划 + 数学 | 分组背包、最大加卷积、分治优化 DP | 标准 / 最优化 |
| 053 | [Bridging the Gap](site/references/053/solution.md) | 动态规划 + 算法策略 | 完全背包、拉格朗日松弛、参数分治 | 标准 / 最优化 |
| 054 | [Under the Epilogue](site/references/054/solution.md) | 图论 + 动态规划 | 强连通性、计数 DP | 标准 / 计数 |
| 055 | [Magical String](site/references/055/solution.md) | 动态规划 + 数学 | 带权区间调度、拉格朗日松弛、离散凸凹性 | 标准 / 最优化 |
| 056 | [Angle Beats](site/references/056/solution.md) | 图论 | 一般图匹配、带花树算法、Micali–Vazirani 算法 | 标准 / 最优化、构造 |
| 057 | [Apple Tree](site/references/057/solution.md) | 动态规划 + 图论 | 换根 DP、组合计数 | 标准 / 计数 |
| 058 | [Cakes](site/references/058/solution.md) | 数学 | 倍数枚举、单调性 | 标准 / 判定 |
| 059 | [Double Sort](site/references/059/solution.md) | 数学 | 概率与期望、有限差分、多项式插值 | 标准 / 期望 |
| 060 | [Staple Stable](site/references/060/solution.md) | 数学 + 搜索与枚举 | 连续松弛、枚举 | 标准 / 最优化 |
| 061 | [Matching](site/references/061/solution.md) | 数学 + 动态规划 | 概率与期望、Markov 链、期望 DP | 标准 / 期望 |
| 062 | [Two Spanning Trees](site/references/062/solution.md) | 图论 + 数据结构 | 生成树、强连通分量、点双连通分量、倍增 | 标准 / 判定 |
| 063 | [Extending Distance](site/references/063/solution.md) | 图论 | 最小费用流、平面对偶、Dijkstra 最短路、势能重赋权 | 标准 / 最优化、构造 |
| 064 | [Elevator Requests IV](site/references/064/solution.md) | 动态规划 + 数据结构 | 区间 DP、线段树、单调性 | 标准 / 最优化 |
| 065 | [Affection](site/references/065/solution.md) | 动态规划 + 数据结构 | 序列 DP、单调队列 | 标准 / 最优化 |
| 066 | [Walk Variance](site/references/066/solution.md) | 图论 + 动态规划 | 图上 DP、图中的环 | 标准 / 最优化 |
| 067 | [Tennis Match](site/references/067/solution.md) | 动态规划 + 算法策略 | 贪心、候选缩减、稀疏状态 DP | 标准 / 最优化 |
| 068 | [The Last Singer](site/references/068/solution.md) | 算法策略 + 搜索与枚举 | 构造算法、决策树 | 交互 / 交互控制 |
| 069 | [Exchanging Kubic](site/references/069/solution.md) | 数据结构 + 算法策略 | 单调栈、贪心、最大子段和 | 交互 / 重建、构造 |
| 070 | [Slot Machine](site/references/070/solution.md) | 搜索与枚举 + 数学 | 候选消除、决策树、周期性 | 交互 / 交互控制 |
| 071 | [Inversion](site/references/071/solution.md) | 搜索与枚举 + 数据结构 | 二分搜索、决策树 | 交互 / 重建 |
| 072 | [Menji, We Miss You!](site/references/072/solution.md) | 搜索与枚举 + 图论 | 二分搜索、决策树、前缀编码 | 交互 / 重建 |
| 073 | [Where Am I Now?](site/references/073/solution.md) | 搜索与枚举 + 图论 + 计算几何 | 深度优先搜索、旋转对称、位集 | 交互 / 判定、重建 |
| 074 | [Radio Direction Finding](site/references/074/solution.md) | 数学 + 搜索与枚举 | 分段线性函数、二分搜索 | 交互 / 重建 |

## 分类依据与特殊说明

### 001 Night Sky

主要难点是环上分段的序列 DP；欧拉迹负责把可行分段变成真实操作，因此同时关联图论。

规模优化：将全部起点合并到同一 DP，按前缀异或类别维护阶段最大值，并以基数排序支持总量线性的查询准备。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/001/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/001/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/001/std.cpp)

### 002 Years

主要计算障碍是概率容斥与子集代数，归于数学；Kruskal 合并结构和根集 DP 是两个实质组成部分，关联图论和动态规划。

规模优化：把原始方向组合的枚举改写为根集分布与源分量容斥，再按完整根集和真子集分组，用快速子集卷积处理指数规模状态。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/002/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/002/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/002/std.cpp)

### 003 Seal

状态计数依赖树 DP，但适应新规模的核心是动态维护与加权平衡组织，因此以数据结构为主，关联动态规划。

规模优化：每个节点只有两次状态变化，按事件更新加权平衡表达式树，使每次变化只重算对数数量的复合节点。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/003/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/003/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/003/std.cpp)

### 004 Value of a Tree

核心模型是树形 DP；根号分治是实现新规模所必需的算法策略。这里的大层压缩不是位掩码状压 DP。

规模优化：将层数按根号阈值拆分，小层用长链合并，大层利用交换性质压缩区域，平衡两部分状态和转移总量。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/004/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/004/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/004/std.cpp)

### 005 Industrial System

主要障碍是大量路径排名与点对计数查询的维护，因此归于数据结构；树中心换根和树类型结构构成重要的图论建模。

规模优化：精确类型压缩递归多重集，换根公式把询问转成路径统计；可持久化线段树与分层窗口表共同控制单次查询代价。

程序用类型序列的精确编号，不依赖随机树哈希；基准点是树中心而不是重心。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/005/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/005/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/005/std.cpp)

### 006 Infinite Strife,

核心是组合数和模数结构的数学变换，根号拆分承担规模优化；网格路径只是初始序列的计数来源，不属于 BFS/DFS。

规模优化：用指数、数位差分及模周期重写重复前缀积，再平方根拆分周期的高低位，避免建立长度与操作次数线性相关的大表。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/006/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/006/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/006/std.cpp)

### 007 Arc Path

首要障碍是从非相交圆构造正确且稀疏的几何连接关系，因此以计算几何为主；最短路是不可省略的图论部分。

规模优化：扫描线把相切检查压到线性数量的候选，再仅连接相邻圆上标记，使最短路图的顶点和边数保持线性。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/007/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/007/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/007/std.cpp)

### 008 Mountain Climbing

主要方法是贪心与参数优化，数学上的离散凸性保证预算恢复；没有按两个操作预算建立背包 DP。

规模优化：先按高度频率预处理开方链和折半收益，再区分小代价与大代价组织候选，并通过参数二分求解预算限制。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/008/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/008/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/008/std.cpp)

### 009 Heuristic Knapsack

核心是贪心可行性和参数构造，线段树支撑高效维护；题名含 Knapsack，但并不求解背包 DP。

规模优化：利用单调性只构造一个极大合法前缀，扫描期间用线段树批量维护跳过约束，避免为每个前缀重新模拟全部物品。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/009/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/009/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/009/std.cpp)

### 010 Play It by Ear

主要计算难点是概率分布的代数求值，因此以数学为主，关联分治策略；牌序未知是题目模型，不表示这是交互题。

规模优化：把策略的期望计算化成后缀多项式求值，再以分治和卷积外推复用相邻区间的计算。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/010/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/010/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/010/std.cpp)

### 011 Square Kingdom

主要障碍是隐式有序集合上的确定性选择，归于算法策略；排名近似与等差计数提供关键数学界限。

规模优化：先证明只有平方根数量的等差行相关，再把候选收缩到同阶大小并用常数轮流式选择控制时间和内存。

不是对巨大答案值域做普通二分，也不需要显式生成全部两两差值。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/011/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/011/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/011/std.cpp)

### 012 DFS Order 4

实际核心是组合生成函数和多项式计算；DFS 是被计数的过程，不是本题标准解的搜索算法。

规模优化：通过生成函数唯一计数 DFS 序，再用倍增和批量卷积计算系数，避免枚举所有树或所有排列。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/012/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/012/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/012/std.cpp)

### 013 Coloring

阶段收益的树 DP 是核心模型，平衡树与线段树解决状态维护；功能图的环树分解又提供独立的图论结构，因此关联三个领域。

规模优化：将可能很长的阶段收益表压成线性总量的断点，挂树通过平衡树合并，环上只在断点事件发生时更新线段树。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/013/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/013/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/013/std.cpp)

### 014 Rikka with Lake

建模、最短路传播和面积计算都依赖多边形几何，主领域为计算几何；这里的最短路是漏斗法而非一般图上的 Dijkstra。

规模优化：将一个洞用两个剪开域覆盖，漏斗状态在剖分上复用；同时利用有限坐标范围组织扫描，并逐片合并面积。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/014/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/014/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/014/std.cpp)

### 015 Spirited Away

核心是异或线性空间与有限域代数，关联线性基的维护和精确去重数据结构；不是字符串哈希题。

规模优化：每个右端点只保留维数严格递增的后缀空间链，各维记录共享生成元快照，并按维依次执行精确去重以控制空间。

当前修复版的桶键只用于加速；答案正确性不依赖无碰撞假设，严格复杂度应以更新后的题解为准。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/015/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/015/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/015/std.cpp)

### 016 Balanced Rainbow Sequence

核心是贪心与离散凸参数优化，括号串是输入对象；不应因此标注 KMP、后缀结构或其他字符串专门算法。

规模优化：先把全部前缀约束压成固定参数下的常数个边界，再利用离散凸性把参数枚举降为对数次线性扫描。

费用流的整数性只用于证明凸性，实际程序不建图或运行费用流。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/016/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/016/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/016/std.cpp)

### 017 J The Attacker Has

博弈必胜条件的数学转化是首要难点，之后通过匹配结构和子集 DP 求解，因此关联图论和动态规划。

规模优化：不展开数量巨大的实体牌，以点数集合和各花色计数压缩对局；相邻子集仅增加最低点数，缺口可常数时间转移。

匹配利用点数顺序求最小缺口，不调用通用网络流，也不展开博弈树做 Minimax 搜索。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/017/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/017/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/017/std.cpp)

### 018 EI Gave Me a Zero

限制条件通过容斥与生成函数处理，超大值域通过多项式性质解决，主领域为数学；不属于数位 DP。

规模优化：先用生成函数批量得到小值域计数，再利用答案关于值域上限的多项式性质进行分批插值和多点求值。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/018/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/018/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/018/std.cpp)

### 019 Euler? Euler!

题目与 Eulerian 排列统计和多项式计算有关，主领域是数学；题名中的 Euler 不表示欧拉回路或图论。

规模优化：将二维排列统计转为生成函数变换、矩阵转置及逐行多点求值，以快速卷积批量计算整张分布。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/019/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/019/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/019/std.cpp)

### 020 Increasing Tree Sequence

子树序列的显式 DP 是主要模型，容斥和多项式卷积构成重要数学加速；LCA 是题意约束而不是主要查询算法。

规模优化：单独保留重儿子，将二维表限制在轻侧规模，并按小块优先合并，用卷积加速插空与终段长度转移。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/020/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/020/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/020/std.cpp)

### 021 Old Problems

核心是除数函数、Möbius 和及数论分块，属于数学；使用位集不等于使用状压 DP。

规模优化：利用对称性缩小参数枚举，再把多个整除商共同分块，配合共享筛表和小区间位集避免逐项处理原三重和。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/021/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/021/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/021/std.cpp)

### 022 Quad Kingdoms Chess 2

随机排列的组合概率建模占主导，系数递推是实质 DP 部分；双方没有选择策略，不应因为 Chess 故事标为博弈搜索。

规模优化：通过两次代数消元把多种分段状态压成一个多项式，再沿坐标轴与对角线递推，避免为同强度两方数量建立二维表。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/022/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/022/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/022/std.cpp)

### 023 Military Maneuver

核心是 Voronoi/Delaunay 几何分解，数学积分负责精确求期望，因此关联数学。

规模优化：利用积分线性性分离最近与最远距离项，再用 Delaunay 邻接把每个单元的全部点约束缩成线性总量的几何约束。

随机化用于几何结构构造，答案通过解析积分求得，不是 Monte Carlo 采样近似。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/023/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/023/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/023/std.cpp)

### 024 Queue Sorting

解法由组合对应和多项式计算驱动，属于数学；队列是题目模型，不是标准程序的核心数据结构。

规模优化：以 RSK 一一对应避免重复统计队列分配方案，把重数乘积取形式对数后按倍数求和，再用快速幂级数指数恢复系数。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/024/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/024/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/024/std.cpp)

### 025 New Year Oracle

核心是生成函数、递推和线性消元，主领域为数学。

规模优化：从组合积分推导固定规模的系数递推，用少量自由坐标生成基并三角消元，避免直接枚举所有合法排列。

积分仅用于推导精确模整数公式；代码不做浮点数值积分，也不使用 NTT。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/025/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/025/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/025/std.cpp)

### 026 Ball Machine

主要瓶颈是如何设计少量可解码的交互操作，因此主类为算法策略；树的链分解和组合编码分别对应图论与数学。

规模优化：通过颜色对并行识别、整数计数编码和补链观测融合，配合预算 DP 将收集次数与最大球值的和控制在 27 内。

交互形式由目录单独提供；九行特殊码使用完整子集和枚举解码，不应把所有解码路径都概括为同一种变换。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/026/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/026/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/026/std.cpp)

### 027 Pico Park

计数与生成函数是主要建模工具，快速多项式分治负责适配规模；输入虽为 L/R 串，但没有使用模式匹配或字符串索引算法。

规模优化：用生成函数把排列计数化为多项式运算，再以分治矩阵乘积、多点求值和 NTT 将总计算控制在 O(n log² n)。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/027/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/027/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/027/std.cpp)

### 028 Product, Euler Totient, and Sum

核心是欧拉函数与质因子状态求和，因此归数学；SOS 在此为乘法子集变换，不能仅凭常见别名把它描述成一般背包 DP。

规模优化：利用值域上限分离大小质数，使状态仅由小质数集合决定，并按值汇总重复元素、对大质数组使用缩小的 SOS 状态。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/028/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/028/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/028/std.cpp)

### 029 Median Replacement

主要障碍是对所有序列最优值求和，需要阈值计数与多项式求和；DP 和线段树是独立的重要组成部分，因此三类同时关联。

规模优化：将巨大值域上的求和拆成 O(n) 个多项式区间，线段树维护采样点矩阵积，插值直接求区间和。

扫描的是阈值端点事件，并非几何扫描线。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/029/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/029/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/029/std.cpp)

### 030 Rolling For Days

概率模型的等价变换主导算法，DP 用于计算变换后的组合权重，故以数学为主、动态规划关联。

规模优化：泊松化后只按首次响铃总数聚合各类贡献，用二状态组合 DP 和阶乘公式计算期望，避免按所有类别已收集数量建状态。

Beta 积分用于推导可直接计算的阶乘公式；程序没有进行数值积分或随机模拟。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/030/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/030/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/030/std.cpp)

### 031 Inverted

主瓶颈是持续维护树形计数表达式，因此以数据结构为主；森林计数模型和动态树 DP 分别构成图论、动态规划关联。

规模优化：用带权平衡表达式树统一组织轻儿子合并和重链矩阵积，将每次计数更新限制为对数长度的祖先重算。

不是普通重链各建线段树的 O(log² n) 更新实现；也没有实际计算 Kirchhoff 行列式。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/031/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/031/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/031/std.cpp)

### 032 Discrete Fourier Transform

傅里叶变换与凸优化构成数学主线，直线上包络是实质性的计算几何工具；不能把任意长度 DFT 等同于直接补零 FFT。

规模优化：Bluestein 与 FFT 快速求所有频率值，再用直线上包络定位凸函数最优点，只检查相邻整数。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/032/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/032/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/032/std.cpp)

### 033 Panda Preserve

问题的核心是最近点几何结构与精确扫描判定，归计算几何；平衡树负责支撑扫描，没有使用旋转卡壳。

规模优化：利用凸性每条 Voronoi 边只保留首末交点，并以扫描线批量完成交点查找和点定位，使候选与事件总量保持线性。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/033/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/033/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/033/std.cpp)

### 034 Match

匹配计数由生成函数、容斥与卷积驱动，数学为主；图的特殊 XOR 结构与按位分治是重要关联。

规模优化：按二进制位分解图结构，使同层子问题总规模受控，匹配计数合并和补图转换统一通过快速卷积完成。

这里统计二分图匹配，但没有运行最大匹配、匈牙利算法或网络流。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/034/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/034/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/034/std.cpp)

### 035 Numbers on a Tree

树上一次性边交换化成局部端口约束是关键图结构转化，贪心和动态路径数据结构共同完成优化。

规模优化：树剖与线段树快速寻找最小可行终点，而全部数字实际提交的路径总长度仅为 2(n−1)，保证逐点更新总量线性。

拓扑排序只用于证明局部约束能实现为某种交换顺序；程序无需执行或输出该拓扑序。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/035/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/035/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/035/std.cpp)

### 036 Road Repair

核心是将增广图连通代价分解并压缩成 MST 问题，因此图论为主；子集变换负责共享大量集合的计数贡献。

规模优化：重构树与子集和把大城市图的贡献预处理并压缩，只在至多 k 个城镇的小图上反复处理各启用子集。

道路语境不意味着最短路；当前实现的目标是全体原城市连通的最小总费用。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/036/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/036/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/036/std.cpp)

### 037 Magic Tiles

最优选择通过链式 DP 建模，数据结构负责比较巨大得分等价的长度字典序；网格只是对象，不是几何算法核心。

规模优化：先缩减为区间链上的两状态 DP，再用持久化频率树和结构唯一化替代整份长度序列的复制与比较。

结构唯一化在散列冲突时比较完整孩子对，因此不是依赖无碰撞假设的随机指纹判等。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/037/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/037/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/037/std.cpp)

### 038 Paired Bracket Sequences

核心是组合生成函数和快速多项式，分治负责加速；括号序列属于题目对象，不应据此归入 KMP、后缀数组等字符串算法。

规模优化：用反演把许多多项式幂归并为一次幂，并以微分方程产生的递推配合 CDQ 卷积计算复合逆相关系数。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/038/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/038/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/038/std.cpp)

### 039 Steadily Growing Steam

这是带差值的背包 DP；范围提升依赖同时保证两种状态截断不漏最优解，而非单纯滚动数组或启发式剪枝。

规模优化：利用小点数上界 T，安全地把加倍次数截至 6T−1、差值截至 9T²，使状态量主要由 T 而非 n 或总点数决定。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/039/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/039/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/039/std.cpp)

### 040 Sum and Product

主要方法全部来自数论；递归因子枚举是公式求和的一部分，无需据此将搜索作为主领域。

规模优化：把三元组同余计数交换为带权因子求和，并用 Miller–Rabin 与 Pollard–Rho 获取分解，避免枚举全部 k 或数对。

固定底判素在本题范围内确定；Pollard–Rho 的随机化影响运行时间，不能声称确定的最坏分解时间界。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/040/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/040/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/040/std.cpp)

### 041 Tree Transform

核心是具有严格次数界的可逆构造策略，树结构保证模板可执行；BFS 只提供处理顺序，不是最短路问题。

规模优化：规范形转化对每个顶点只需常数次局部操作，统一控制两棵树之间的完整操作序列在 400 次预算内。

题目不要求最少操作数；初始星形树的不可操作性需要单独判断。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/041/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/041/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/041/std.cpp)

### 042 Maximum

概率转带权计数与指数生成函数主导建模，因此数学为主；计算落在确定性的组合状态递推上。

规模优化：围绕最大容量构造只与盒子数有关的状态，并用统一缩放消去逐状态除法，以小状态表推进大量投球轮次。

微分方程用于推导离散系数递推，程序不使用数值微分方程求解器。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/042/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/042/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/042/std.cpp)

### 043 Edge Coloring Problem

最优边染色及特殊稠密图结构是核心图论内容，构造策略保证达到颜色下界；交替路不等同于二分图最大匹配。

规模优化：先直接染色大核心，只对 O(N) 条补边执行每次 O(N) 的扇形调整，使总工作量保持 O(N²)。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/043/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/043/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/043/std.cpp)

### 044 World Map

主要任务是满足尺寸预算的图到网格构造，因此算法策略为主、图论关联；国家允许不连通，不能按平面图嵌入或普通计算几何归类。

规模优化：根据 DFS 深浅选用互补构造，并借助重叠和补边压缩方格尺寸，保证边长不超过 floor(5N/4)。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/044/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/044/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/044/std.cpp)

### 045 Folding Game of Ohto Ai

利用树直径与折叠不变性选择固定模拟过程，数据结构负责线性维护；名称中的 Game 不表示存在对抗博弈。

规模优化：直径长度总和由删除顶点数控制，短桶合并与收缩又由初始桶长度势能支付，从而实现总线性模拟。

当前解法无需搜索折叠顺序；所有合法折叠过程的次数相同。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/045/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/045/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/045/std.cpp)

### 046 Bitaro the Brave 3

规模瓶颈在维护和汇总多个凸包版本，因此数据结构为主；凸几何表示及贪心调度分别是不可缺少的关联。

规模优化：共享各激活版本的大部分凸包边并集中传播权重，将众多前缀函数合并成有限折线事件，再批量处理单调询问。

题面没有几何图形，但解法确实使用凸包；不能只依据题面物件决定是否归计算几何。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/046/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/046/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/046/std.cpp)

### 047 Ambulance

核心是四组容量分配背包及其批量复用策略；网格运输已化为距离公式，不需要 BFS、Dijkstra 或逐格寻路。

规模优化：枢轴引理将双分界缩减到逐病人枚举，再以平方根分块复用绝大部分背包，使总处理降为 O(N√N·T)。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/047/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/047/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/047/std.cpp)

### 048 Iroha and the Kingdom of Construction

当前候选的核心是全序设计、贪心分桶和单调子序列划分，因此暂归算法策略；这些标签只描述已有候选，不代表完整标准解。

规模优化：当前候选使用二分末尾数组加速分桶与栈分配，但这只改善候选实现的计算开销，尚不能保证对所有合法输入满足 180 栈限制。

全范围算法尚未解决：存在可用 20 栈完成、原候选正反构造却需要 181 栈并退出的合法输入。分类仅题目形式与对象确定，解法标签为暂定。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/048/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/048/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/048/std.cpp)

### 049 Sort

核心是随机过程的期望分解与状态化简，动态规划负责计算路径权重；题名 Sort 不表示排序算法本身是主要考点。

规模优化：把复杂排列过程聚合成两端点排名状态，消除自环后共享二维路径权重，并用前缀和完成全部首次切分的求值。

程序是确定性模运算，没有随机模拟，也没有对一般 Markov 链执行高斯消元。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/049/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/049/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/049/std.cpp)

### 050 Las Vegas

最小费用流精确表达分配与胜场约束，二分和离散凸性负责减少网络求解次数；名称 Las Vegas 与随机算法类别无关。

规模优化：每个赌场压缩成至多三种胜者选择，并利用费用关于胜场数的离散凸性，将需要求解的流网络数限制为对数级。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/050/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/050/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/050/std.cpp)

### 051 Athlete Welcome Ceremony

主障碍是字符串填法的代数计数与快速合并，因此归入数学；字符串只是计数对象，未使用专门字符串匹配或后缀算法。

规模优化：段内系数以二次规模生成，段间采用快速卷积，全部计数预处理后每个上限询问只做常数次查表。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/051/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/051/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/051/std.cpp)

### 052 No Bug No Game

分组背包是实际模型，离散凹性决定可加速的转移结构，因此主动态规划、关联数学。

规模优化：基础强度只有少量取值，先构造组内收益序列，再用决策单调分治加速每组背包转移。

费用流仅用于证明部分强化收益的凹性，代码不运行费用流；当前分治卷积实现包含对数因子。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/052/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/052/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/052/std.cpp)

### 053 Bridging the Gap

实际计算由账本 DP、完全背包和参数化 DAG DP 组成；大小容量分支选择具有独立的算法策略作用。

规模优化：用已安置人数与返程欠账压缩行程，再按容量选择互补分支，并用背包周期减少需要精确查询的组数。

Monge 性质用于推出费用凸性；小容量带奖励 DP 仍直接枚举前驱，不应标成执行了 SMAWK 或分治决策优化。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/053/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/053/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/053/std.cpp)

### 054 Under the Epilogue

强连通诱导集合是主要建模障碍，加权 DP 是计算手段，因此主图论、关联动态规划。

规模优化：利用区间出边结构压缩可达性状态，并用间隙权值消去非核心插点，使集合计数成为多项式规模的加权 DP。

这里的强连通标签表示不可缺少的图模型；程序并没有对所有候选集合运行 Tarjan。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/054/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/054/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/054/std.cpp)

### 055 Magical String

核心是区间选择 DP 与离散凹性，不是字符串匹配；因此主动态规划、关联数学。

规模优化：只保留线性数量的短区间候选，并枚举六个罚值进行线性前缀 DP，从状态中消去大预算 K。

代码枚举罚值而非二分罚值，不标为 WQS 二分；也未使用 KMP、Z 或后缀结构。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/055/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/055/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/055/std.cpp)

### 056 Angle Beats

主要困难是将三格骨牌约束归约成一般图最大匹配，归入图论；网格是输入对象，不能因此误标计算几何或二分图匹配。

规模优化：每个格子只产生常数个顶点和边，利用稀疏辅助图上的分阶段一般图匹配求最优放置，再线性恢复输出。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/056/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/056/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/056/std.cpp)

### 057 Apple Tree

以距离分布换根 DP 枚举所有树上中心，树的结构条件控制总状态量，因此主动态规划、关联图论。

规模优化：严格递减的孩子数限制使全部子树距离数组总长度为线性，换根仅平移偏移并修改相应子树的距离层。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/057/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/057/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/057/std.cpp)

### 058 Cakes

关键是整除求和与单调可行性条件，属于数学；题目询问分配是否存在，但无需输出构造或执行二分答案。

规模优化：利用配料数量上限预处理所有美味度的整除和，把每个可能很大的蛋糕数量询问化为一次候选计算和查表。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/058/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/058/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/058/std.cpp)

### 059 Double Sort

随机组合对象的期望与插值是主线，属于数学；标题中的排序不是标程的核心复杂度来源。

规模优化：用组合数矩合并所有随机间隔状态，再通过插值消去对巨大数值上界的逐项求和，运算规模由多项式次数控制。

二次级别只指大整数算术操作次数，不能当作相同数量的机器字操作；本解不是随机模拟。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/059/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/059/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/059/std.cpp)

### 060 Staple Stable

主方法是整数优化的连续下界与候选缩减，关联搜索枚举；矩形切割是对象，不需要扫描线或旋转卡壳。

规模优化：按长宽比例选择枚举变量，连续上下界把最优候选限制在约四次方根规模的区间，并二分定位区间端点。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/060/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/060/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/060/std.cpp)

### 061 Matching

概率模型与组合对称性主导状态压缩，DP 负责求解，因此主数学、关联动态规划；标题 Matching 不代表最大匹配算法。

规模优化：把多种颜色人数的演化压缩成单一来源数，并解析消去自环，避免枚举完整集合分组状态。

完整前向 Markov 链及高斯消元是独立核验方法，不是标程运行的方法。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/061/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/061/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/061/std.cpp)

### 062 Two Spanning Trees

生成树交换与连通分解是主要图论模型，树状数组和倍增路径结构是适配规模的实质工具。

规模优化：先用离线树状数组检验路径包含，再以点双块和倍增区间压缩交换依赖，避免逐条展开长树路径。

题面只要求存在性，不输出生成树；模二矩阵是证明表示，代码不执行高斯消元。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/062/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/062/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/062/std.cpp)

### 063 Extending Distance

核心是最短路、平面对偶和费用流之间的图论归约；与只在证明中提到费用流的题不同，本题实际运行费用流并恢复构造。

规模优化：在原图和对偶图两种费用流模型间按参数选择，把增广次数限制到行数与目标增量的较小者，并用小距离桶加速增广。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/063/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/063/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/063/std.cpp)

### 064 Elevator Requests IV

区间 DP 给出完整优化模型，线段树直接决定大规模可行性，因此主动态规划、关联数据结构。

规模优化：用归一化和单调性把一整行区间 DP 转移化为常数次区间赋值及边界查找，再由线段树对数时间完成。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/064/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/064/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/064/std.cpp)

### 065 Affection

两状态序列 DP 是基础，单调双端队列把多个参数下的状态一起维护，因此主动态规划、关联数据结构。

规模优化：对固定目标差同时维护全部低目标候选，函数块只入队出队常数次，将每轮全部候选的处理压成线性。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/065/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/065/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/065/std.cpp)

### 066 Walk Variance

路径与环的结构归约解决无限游走的最优化，有限状态图上 DP 完成计算，因此主图论、关联动态规划。

规模优化：小边权允许把总权作为 DP 状态；只从起点运行到顶点数层，并从代表游走恢复环，避免为每个环起点单独求解。

方差是确定性目标，不代表随机算法；重赋权最短路只用于正确性证明，标程不执行该证明中的最短路算法。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/066/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/066/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/066/std.cpp)

### 067 Tennis Match

有序配对后的稀疏 DP 是求解主体，贪心代表筛选是关键规模优化；题目虽然要求配对，但不运行一般图匹配。

规模优化：先将大选手集合压缩成每场仅与比赛数相关的代表候选，再在稀疏位置上做 DP，避免每场遍历全部选手。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/067/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/067/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/067/std.cpp)

### 068 The Last Singer

大规模主体是确定构造策略，边界小规模真实执行决策树搜索，因此主算法策略、关联搜索与枚举；交互形式由目录 track 独立表示。

规模优化：大规模采用有动作总量证明的锚点构造，小规模另用归一化和记忆化的预算决策树，分别覆盖统一的平方动作上限。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/068/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/068/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/068/std.cpp)

### 069 Exchanging Kubic

核心维护对象是单调栈上的正块与间隔，因此主数据结构；贪心合并和信息保留规则负责重构。

规模优化：跳点采样减少单点询问，活动块只入栈出栈一次；按合并与停止事件计费，使本地处理和查询量均为线性。

输出的是所有区间最大子段和等价的数组，不要求恢复隐藏原数组，也不需要线段树。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/069/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/069/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/069/std.cpp)

### 070 Slot Machine

主体是反馈驱动的候选辨识与动作策略，循环周期理论保证策略可行，因此主搜索、关联数学。

规模优化：用符号和唯一性压缩待处理轮状态，并维持无周期集合保证探测可分；按剩余轮数分配动作预算。

周期对象是循环占用集合；代码枚举偏移逐项检查，不运行 KMP 或 Z 算法。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/070/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/070/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/070/std.cpp)

### 071 Inversion

核心是交互候选二分与预算搜索，位集是本地计算的实质加速手段，因此主搜索、关联数据结构。

规模优化：按元素逐个恢复插入排名，以预算决策树控制询问数，并用位集翻转加速大批候选的分割评价。

这不是常规离线逆序对计数；不能仅凭题名贴树状数组标签。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/071/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/071/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/071/std.cpp)

### 072 Menji, We Miss You!

主体是树上自适应候选搜索与决策预算，二叉树结构决定允许的分割，因此主搜索、关联图论。

规模优化：按层宽分配不同深度查询长度，再用子树候选容量递推完成层内定位，使两阶段查询总数满足统一上限。

用于核验 21 次总上界的平方规模层宽 DP 不在线执行，不能据此主标动态规划或把方法误称为重心分解。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/072/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/072/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/072/std.cpp)

### 073 Where Am I Now?

主问题是未知状态下的地图探索，图搜索负责探索和导航；旋转形状等价与几何固定点判据不可省略，因此关联图论与计算几何。

规模优化：位集批量维护观测相容状态，DFS 的方向次序与共同中心导航给出动作上界，避免对每个可能位置单独探索。

计算几何关联来自实际形状规范化与旋转中心判定，而不是仅因为输入为网格；判断位置唯一也不要求朝向唯一。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/073/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/073/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/073/std.cpp)

### 074 Radio Direction Finding

分段距离代数模型决定信息提取方式，搜索只处理剩余的一维候选，因此主数学、关联搜索；环图对象不代表执行一般最短路。

规模优化：两次固定询问把多数情况直接化为方程，剩余未知量限制在短区间，再二分得到对数查询上界。

来源：[新题面](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/074/algoscale-statement.md) · [题解](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/074/solution.md) · [参考程序](https://github.com/shi0712/AlgoScale/blob/2a9b05d19cc1961d87350ebe3ad4205436c9820d/site/references/074/std.cpp)

## 字段与维护规则

- `primaryDomain` 必须是 `domains` 的首项；`domains` 可有 1–3 个领域。
- `topics` 引用统一词表，`role` 为 `core` 或 `supporting`，每项带可核对的中文 `evidence`。
- `taskTags`、`objectTags`、`scaleTags` 分别记录任务性质、题面对象和效率来源，互不替代。
- `track` 沿用原目录的 `Standard / Interactive`；它与模型评测的 Direct / Agent 模式不同。
- `taxonomy.topics[].parents` 用于上位专题检索；`aliases` 统一旧标签 ID。`problemIds` 只统计直接标注，不含父标签展开，也不默认排除候选题；能力分析应结合 `classificationStatus`。
- 题解或参考程序更新后，应重新核对相应标签，并更新 `sourceRevision`、来源与 SHA256。
- 标签用于题库浏览和结果分析，评测输入仍按既定协议提供题面，不额外提供解法标签。

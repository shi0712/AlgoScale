#!/usr/bin/env python3
"""Import the maintained classification workbook into the public catalog."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import openpyxl


DOMAIN_NAMES = {
    "算法策略": "Algorithmic Paradigms",
    "计算几何": "Computational Geometry",
    "数据结构": "Data Structures",
    "动态规划": "Dynamic Programming",
    "图论": "Graph Theory",
    "数学": "Mathematics",
    "搜索与枚举": "Search & Enumeration",
}

CORE_TAG_NAMES = {
    "BFPRT 线性选择": "BFPRT Linear Selection",
    "Bluestein 变换": "Bluestein's Algorithm",
    "CDQ 分治": "CDQ Divide and Conquer",
    "Delaunay 三角剖分": "Delaunay Triangulation",
    "Dijkstra 最短路": "Dijkstra's Shortest Path",
    "FFT 快速傅里叶变换": "Fast Fourier Transform (FFT)",
    "Kempe 交替链": "Kempe Chains",
    "Kruskal 重构树": "Kruskal Reconstruction Tree",
    "Lucas 定理": "Lucas's Theorem",
    "Markov 链": "Markov Chains",
    "Micali–Vazirani 算法": "Micali–Vazirani Algorithm",
    "Möbius 反演": "Möbius Inversion",
    "NTT 数论变换": "Number-Theoretic Transform (NTT)",
    "Pollard–Rho 分解": "Pollard–Rho Factorization",
    "RSK 对应": "Robinson–Schensted–Knuth Correspondence",
    "Stirling 数": "Stirling Numbers",
    "Vizing 扇": "Vizing Fans",
    "Voronoi 图": "Voronoi Diagrams",
    "一般图匹配": "General Graph Matching",
    "二分图匹配": "Bipartite Matching",
    "二分搜索": "Binary Search",
    "位掩码": "Bitmasking",
    "位集": "Bitsets",
    "倍增": "Binary Lifting",
    "倍数枚举": "Multiple Enumeration",
    "候选消除": "Candidate Elimination",
    "候选缩减": "Candidate Reduction",
    "决策树": "Decision Trees",
    "几何最短路": "Geometric Shortest Paths",
    "几何积分": "Geometric Integration",
    "凸优化": "Convex Optimization",
    "凸包": "Convex Hull",
    "分段线性函数": "Piecewise-Linear Functions",
    "分治": "Divide and Conquer",
    "分治优化 DP": "Divide-and-Conquer DP Optimization",
    "分组背包": "Group Knapsack",
    "前缀编码": "Prefix Codes",
    "剪枝与搜索": "Pruned Search",
    "功能图": "Functional Graphs",
    "动态树形 DP": "Dynamic Tree DP",
    "势能重赋权": "Potential Reweighting",
    "区间 DP": "Interval DP",
    "半平面交": "Half-Plane Intersection",
    "单调性": "Monotonicity",
    "单调栈": "Monotonic Stack",
    "单调队列": "Monotonic Queue",
    "博弈论": "Game Theory",
    "参数分治": "Parametric Divide and Conquer",
    "可持久化数据结构": "Persistent Data Structures",
    "可持久化线段树": "Persistent Segment Tree",
    "启发式合并": "Small-to-Large Merging",
    "周期性": "Periodicity",
    "因子枚举": "Divisor Enumeration",
    "图上 DP": "Graph DP",
    "图中的环": "Graph Cycles",
    "图建模": "Graph Modeling",
    "圆几何": "Circle Geometry",
    "多点求值": "Multipoint Evaluation",
    "多边形三角剖分": "Polygon Triangulation",
    "多项式卷积": "Polynomial Convolution",
    "多项式插值": "Polynomial Interpolation",
    "子集卷积": "Subset Convolution",
    "子集变换": "Subset Transforms",
    "完全背包": "Unbounded Knapsack",
    "容斥原理": "Inclusion–Exclusion",
    "带权区间调度": "Weighted Interval Scheduling",
    "带花树算法": "Blossom Algorithm",
    "平衡搜索树": "Balanced Search Trees",
    "平衡表达式树": "Balanced Expression Trees",
    "平面对偶": "Planar Duality",
    "并查集": "Disjoint Set Union",
    "序列 DP": "Sequence DP",
    "异或线性基": "XOR Linear Basis",
    "强连通分量": "Strongly Connected Components",
    "强连通性": "Strong Connectivity",
    "形式幂级数": "Formal Power Series",
    "扫描线": "Sweep Line",
    "拉格朗日反演": "Lagrange Inversion",
    "拉格朗日插值": "Lagrange Interpolation",
    "拉格朗日松弛": "Lagrangian Relaxation",
    "换根": "Rerooting",
    "换根 DP": "Rerooting DP",
    "数论分块": "Quotient Grouping",
    "旋转对称": "Rotational Symmetry",
    "最大加卷积": "Max-Plus Convolution",
    "最大子段和": "Maximum Subarray",
    "最小生成树": "Minimum Spanning Tree",
    "最小费用流": "Min-Cost Flow",
    "最长递增子序列": "Longest Increasing Subsequence",
    "有限域": "Finite Fields",
    "有限差分": "Finite Differences",
    "有限状态动态规划": "Finite-State DP",
    "期望 DP": "Expectation DP",
    "构造算法": "Constructive Algorithms",
    "枚举": "Enumeration",
    "树同构": "Tree Isomorphism",
    "树形 DP": "Tree DP",
    "树直径": "Tree Diameter",
    "树重建": "Tree Reconstruction",
    "根号分治与分块": "Square-Root Decomposition",
    "概率与期望": "Probability and Expectation",
    "概率动态规划": "Probabilistic DP",
    "模运算": "Modular Arithmetic",
    "欧拉函数": "Euler's Totient Function",
    "欧拉迹": "Euler Trails",
    "泊松化": "Poissonization",
    "深度优先搜索": "Depth-First Search",
    "点双连通分量": "Vertex-Biconnected Components",
    "点定位": "Point Location",
    "状压动态规划": "Bitmask DP",
    "状态空间缩减": "State-Space Reduction",
    "生成函数": "Generating Functions",
    "生成树": "Spanning Trees",
    "矩阵乘积": "Matrix Multiplication",
    "离散凸凹性": "Discrete Convexity and Concavity",
    "稀疏状态 DP": "Sparse-State DP",
    "筛法": "Sieve Methods",
    "线性代数": "Linear Algebra",
    "线性化多项式": "Linearized Polynomials",
    "线性递推": "Linear Recurrences",
    "线段树": "Segment Tree",
    "组合计数": "Combinatorial Counting",
    "组合设计": "Combinatorial Designs",
    "结构唯一化": "Structural Canonicalization",
    "耐心排序": "Patience Sorting",
    "背包动态规划": "Knapsack DP",
    "规范形": "Canonical Forms",
    "计数 DP": "Counting DP",
    "贪心": "Greedy Algorithms",
    "边染色": "Edge Coloring",
    "连续松弛": "Continuous Relaxation",
    "重链剖分": "Heavy-Light Decomposition",
    "长链剖分": "Long-Chain Decomposition",
}


def parse_args() -> argparse.Namespace:
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser()
    parser.add_argument("workbook", type=Path)
    parser.add_argument("--catalog", type=Path, default=root / "site/data/problems.json")
    parser.add_argument("--check", action="store_true", help="validate without writing")
    return parser.parse_args()


def split_values(value: object) -> list[str]:
    return [part.strip() for part in str(value or "").split("、") if part.strip() and part.strip() != "—"]


def translated(values: list[str], mapping: dict[str, str], label: str) -> list[str]:
    unknown = [value for value in values if value not in mapping]
    if unknown:
        raise ValueError(f"unknown {label}: {unknown}")
    return [mapping[value] for value in values]


def read_taxonomy(path: Path) -> dict[int, dict[str, object]]:
    sheet = openpyxl.load_workbook(path, read_only=False, data_only=True)["分类总表"]
    headers = {sheet.cell(6, column).value: column for column in range(1, sheet.max_column + 1)}
    required = {"编号", "题目", "主领域", "关联领域", "核心标签"}
    if not required.issubset(headers):
        raise ValueError(f"classification sheet is missing columns: {sorted(required - headers.keys())}")

    result: dict[int, dict[str, object]] = {}
    for row in range(7, sheet.max_row + 1):
        raw_id = sheet.cell(row, headers["编号"]).value
        if raw_id in (None, ""):
            continue
        try:
            problem_id = int(raw_id)
        except (TypeError, ValueError):
            # The maintained sheet includes summary tables below the 74 records.
            continue
        primary = str(sheet.cell(row, headers["主领域"]).value or "").strip()
        related = split_values(sheet.cell(row, headers["关联领域"]).value)
        core_tags = split_values(sheet.cell(row, headers["核心标签"]).value)
        if problem_id in result:
            raise ValueError(f"duplicate problem ID in workbook: {problem_id}")
        if primary not in DOMAIN_NAMES:
            raise ValueError(f"unknown primary domain for #{problem_id}: {primary}")
        result[problem_id] = {
            "workbookTitle": str(sheet.cell(row, headers["题目"]).value or "").strip(),
            "primaryDomain": DOMAIN_NAMES[primary],
            "relatedDomains": translated(related, DOMAIN_NAMES, "related domain"),
            "coreTags": translated(core_tags, CORE_TAG_NAMES, "core tag"),
        }
    return result


def main() -> None:
    args = parse_args()
    catalog = json.loads(args.catalog.read_text(encoding="utf-8"))
    taxonomy = read_taxonomy(args.workbook)
    catalog_ids = {int(problem["id"]) for problem in catalog}
    if catalog_ids != set(taxonomy):
        raise ValueError(
            f"ID mismatch: catalog-only={sorted(catalog_ids - set(taxonomy))}, "
            f"workbook-only={sorted(set(taxonomy) - catalog_ids)}"
        )

    title_warnings = []
    for problem in catalog:
        values = taxonomy[int(problem["id"])]
        workbook_title = values.pop("workbookTitle")
        if workbook_title != problem["title"]:
            title_warnings.append(
                f"#{int(problem['id']):03d}: catalog={problem['title']!r}, workbook={workbook_title!r}"
            )
        problem["taxonomy"] = values

    counts = {domain: 0 for domain in DOMAIN_NAMES.values()}
    for problem in catalog:
        domains = {problem["taxonomy"]["primaryDomain"], *problem["taxonomy"]["relatedDomains"]}
        for domain in domains:
            counts[domain] += 1
    expected = {
        "Algorithmic Paradigms": 23,
        "Computational Geometry": 7,
        "Data Structures": 16,
        "Dynamic Programming": 26,
        "Graph Theory": 24,
        "Mathematics": 39,
        "Search & Enumeration": 7,
    }
    if counts != expected:
        raise ValueError(f"domain totals changed: {counts}")

    for warning in title_warnings:
        print(f"warning: {warning}")
    print("domain totals:", ", ".join(f"{domain}={counts[domain]}" for domain in expected))
    if args.check:
        print(f"validated {len(catalog)} problems; catalog not modified")
        return
    args.catalog.write_text(
        json.dumps(catalog, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(f"updated taxonomy for {len(catalog)} problems in {args.catalog}")


if __name__ == "__main__":
    main()

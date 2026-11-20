## Problem Description

This is an interactive problem.

Kostyanych has chosen a complete undirected graph$^{\dagger}$ with $n$ vertices, and then removed exactly $(n - 2)$ edges from it. You can ask queries of the following type:

* "? $d$" — Kostyanych tells you the number of vertex $v$ with a degree at least $d$. Among all possible such vertices, he selects the vertex with the minimum degree, and if there are several such vertices, he selects the one with the minimum number. He also tells you the number of another vertex in the graph, with which $v$ is not connected by an edge (if none is found, then $0$ is reported). Among all possible such vertices, he selects the one with the minimum number. Then he removes the vertex $v$ and all edges coming out of it. If the required vertex $v$ is not found, then "$0\ 0$" is reported.

Find a Hamiltonian path$^{\ddagger}$ in the original graph. It can be proven that under these constraints, a Hamiltonian path always exists.

$^{\dagger}$A complete undirected graph is a graph in which there is exactly one undirected edge between any pair of distinct vertices. Thus, a complete undirected graph with $n$ vertices contains $\frac{n(n-1)}{2}$ edges.

$^{\ddagger}$A Hamiltonian path in a graph is a path that passes through each vertex exactly once.

## Input Format

Read n.

## Output Format

```text
? d
! v1 ... vn
```

Read the two vertex labels returned by the rule above, in order. The reported path must visit every vertex exactly once and use edges of the original graph. The starting graph is fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `! 1 2` |

## Constraints

- 2 ≤ n ≤ 100000. A threshold is an integer in [0,n−1].
- Time limit: **10 seconds**. Memory limit: **256 MB**.

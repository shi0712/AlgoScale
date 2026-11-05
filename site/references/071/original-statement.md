## Problem Description

Two connected simple undirected graphs, $A$ and $B$, each have $n$ vertices and $m$ edges. Their edges are indexed $1$ through $m$; equal indices do not necessarily have equal endpoints in the two graphs.

Determine whether there is a set of edge indices that forms a spanning tree in $A$ but does not form a spanning tree in $B$.

## Input Format

Read from standard input.

The first line contains the number of test cases $t$.

Each case begins with $n,m$. Each of the next $m$ lines contains $u_A,v_A,u_B,v_B$. The $i$-th line gives the endpoints of edge $i$ in $A$ and edge $i$ in $B$ respectively.

## Output Format

Write to standard output.

For each case, print `YES` if such an index set exists, and `NO` otherwise. You do not need to construct or print the set.

## Examples

Input:

```text
1
2 1
1 2 2 1
```

Output:

```text
NO
```

## Explanation / Notes

Both graphs consist of the same undirected edge between vertices 1 and 2. Their only spanning tree uses edge 1, so it cannot be a spanning tree in $A$ and fail in $B$.

The condition is directional: it specifically requires a spanning tree of the first graph that fails in the second.

## Constraints

- $1\le t\le2500$.
- $2\le n\le5000$ per case.
- $n-1\le m\le\min\left(5000,\frac{n(n-1)}2\right)$.
- Every endpoint lies in $[1,n]$.
- $u_A\ne v_A$ and $u_B\ne v_B$ for every edge.
- Both graphs are undirected, connected, and simple: no self-loops or parallel edges.
- Across all cases, $\sum n\le5000$ and, independently, $\sum m\le5000$.
- A spanning tree includes every vertex and exactly $n-1$ of the indexed edges.
- Time limit: **1 second**. Memory limit: **1024 MiB**.

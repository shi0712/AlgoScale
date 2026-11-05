## Problem Description

A simple undirected graph has $N$ vertices, each of degree exactly $N-3$. Its adjacency matrix is supplied as binary strings.

Assign a positive integer color to every edge so that any two edges sharing a vertex have different colors. The edge chromatic number is the smallest possible maximum color in such an assignment.

Find this minimum number $C$ and produce an edge coloring that attains it.

## Input Format

Read from standard input.

Read $N$, followed by $N$ binary strings $S_1,\ldots,S_N$, each of length $N$. Character $j$ of $S_i$ is `1` precisely when edge $\{i,j\}$ exists.

## Output Format

Write to standard output.

Print $C$, followed by an $N\times N$ integer matrix $c$.

- If edge $\{i,j\}$ exists, $c_{i,j}=c_{j,i}$ is its assigned positive color, at most $C$.
- If the edge does not exist, print $c_{i,j}=-1$.
- Every diagonal entry must be $-1$.

Any optimal valid coloring is accepted.

## Examples

Input:

```text
5
01001
10100
01010
00101
10010
```

Output:

```text
3
-1 2 -1 -1 1
2 -1 3 -1 -1
-1 3 -1 1 -1
-1 -1 1 -1 3
1 -1 -1 3 -1
```

## Explanation / Notes

The example graph is a cycle of length 5. Its edges cannot alternate between only two colors around the odd cycle. The displayed coloring uses three colors and is optimal.

Colors belong to edges, not to vertices. Nonedges must be represented by $-1$, rather than by color zero.

## Constraints

- There is one input instance.
- $4\le N\le300$.
- Each adjacency row has exactly $N$ characters, each `0` or `1`.
- The graph is simple and undirected: the matrix is symmetric and its diagonal is zero.
- Every vertex has degree exactly $N-3$.
- Consequently, the number of edges is exactly $N(N-3)/2$.
- All actual edge colors must be positive integers, with distinct colors on edges incident to a common vertex.
- The reported $C$ must be minimal; a proper coloring using more than the optimal number is insufficient.
- Time limit: 3 seconds.
- Memory limit: 1024 MB.

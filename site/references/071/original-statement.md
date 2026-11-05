## Problem Description

An $n\times m$ grid of vertices has weighted undirected edges between every pair of orthogonally adjacent vertices. A journey may start at any vertex in column 1 and end at any vertex in column $m$. Its distance is the sum of the traversed edge weights, and the traveler chooses the shortest available journey over all such endpoints.

One operation increases one edge weight by one. Make the shortest possible journey longer by $k$, using the minimum number of operations. Output both that minimum and a resulting set of edge weights.

## Input Format

Read from standard input.

The first line contains the number of test cases $T$.

For each case:

- Read $n,m,k$.
- Read $n$ rows, each containing $m-1$ weights $r_{i,1},\ldots,r_{i,m-1}$. Weight $r_{i,j}$ belongs to the horizontal edge $(i,j)$–$(i,j+1)$.
- Read $n-1$ rows, each containing $m$ weights $c_{i,1},\ldots,c_{i,m}$. Weight $c_{i,j}$ belongs to the vertical edge $(i,j)$–$(i+1,j)$.

## Output Format

Write to standard output.

For each case, print the minimum operation count, then the modified weights in the same layout: $n$ rows of $m-1$ horizontal weights followed by $n-1$ rows of $m$ vertical weights.

Only increases are allowed. Each resulting weight must lie in $[1,2\times10^9]$. Any optimal valid result is accepted.

**Do not print trailing spaces at the ends of output lines.**

## Examples

Input:

```text
1
3 3 3
1 1
2 2
3 3
1 1 1
2 2 2
```

Output:

```text
4
4 1
3 2
3 3
1 1 1
2 2 2
```

## Explanation / Notes

The original minimum distance is 2 along the top row. The shown construction increases the first top-row edge by 3 and the first middle-row edge by 1, for four operations in total. The new minimum distance is 5, so it has increased by the requested 3.

The traveler may move in either direction along any edge, including leftward moves. Starting and ending rows need not agree. The operation count equals the sum of all weight increments.

## Constraints

- Each test case has integer $n,m,k$.
- $2\le n,m\le500$.
- $1\le nm\le500$.
- $1\le k\le100$.
- Every original horizontal and vertical weight is an integer in $[1,10^9]$.
- Across all cases, $\sum nm\le5\times10^3$.
- Each output weight is an integer in $[1,2\times10^9]$ and must be at least its original value.
- Horizontal weight arrays have dimensions $n\times(m-1)$; vertical arrays have dimensions $(n-1)\times m$.
- The modified minimum distance between the first and last columns must increase by $k$.
- Time limit: **1 second**. Memory limit: **1024 MB**.

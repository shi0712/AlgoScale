# 37. Inverted

## Problem Description

Initially, an undirected tree has vertices $1,\ldots,n$. Process a sequence of $n-1$ distinct original vertices.

When processing vertex $x$, first create a new vertex $x+n$. For every original vertex $i$ adjacent to $x$ in the original tree:

- If vertex $i+n$ does not yet exist, add edge $(x+n,i)$.
- Otherwise, add edge $(x+n,i+n)$ and delete edge $(x,i+n)$. The latter edge is guaranteed to exist at this stage.

After each operation, report the number of spanning trees of the current graph modulo $998244353$.

## Input Format

Read from standard input.

Read $n$. The next $n-1$ lines contain tree edges $u,v$. The final sequence contains $n-1$ distinct integers $b_1,\ldots,b_{n-1}$, the original vertices to process in order.

## Output Format

Write to standard output.

Print $n-1$ lines. Line $i$ contains the spanning-tree count after processing $b_i$, modulo $998244353$.

## Examples

Input:

```text
5
1 2
1 3
2 4
2 5
1 5 2 3
```

Output:

```text
4
4
6
1
```

## Explanation / Notes

An operation preserves the original edges $(x,i)$. The edge deletion in the second rule concerns $(x,i+n)$, which was created earlier; no other edge is removed.

A spanning tree includes every vertex currently present. After $j$ operations, the graph has $n+j$ vertices. When $n=1$, the operation sequence and required output are both empty.

## Constraints

- $1\le n\le5000$.
- Every input edge has endpoints in $[1,n]$.
- The $n-1$ input edges form a valid tree.
- $1\le b_i\le n$, and the $n-1$ sequence entries are pairwise distinct.
- Each original vertex is processed at most once; exactly one original vertex is never processed.
- New vertex labels are $n+1$ through $2n$, but only labels corresponding to processed vertices are created.
- Counts are taken modulo $998244353$.
- Time limit: 1 second.
- Memory limit: 256 MB.

## Problem Description

Consider a tree on vertices $1,\ldots,n$, rooted at vertex $1$. A depth-first search starts by visiting and recording the root. At each vertex, its children may be processed in any order; each child's entire subtree is explored before moving on to the next child. Record a vertex when it is first visited.

You are given $m$ permutations, each starting with $1$. Count the rooted trees for which **every** supplied permutation can be a DFS order. The ordering of children may differ between the DFS runs.

Trees are distinct when their edge sets differ. Return the count modulo $10^9+7$.

## Input Format

Read from standard input.

The first line contains $n,m$. Each of the next $m$ lines contains one permutation $p_{k,1},\ldots,p_{k,n}$ of $1,\ldots,n$.

## Output Format

Write to standard output.

Print the number of compatible rooted trees modulo $1000000007$.

## Examples

Input:

```text
4 1
1 2 3 4
```

Output:

```text
5
```

## Explanation / Notes

Writing $(p_2,p_3,p_4)$ for the parents of vertices 2, 3, 4, the five compatible trees are $(1, 1, 1)$, $(1, 1, 3)$, $(1, 2, 1)$, $(1, 2, 2)$, and $(1, 2, 3)$.

This is a first-visit DFS order, not a sequence that records returns to ancestors. Child order is a choice made during a traversal and is not part of the identity of a counted tree.

## Constraints

- $1\le n,m\le500$.
- Every supplied sequence contains each integer in $[1,n]$ exactly once.
- $p_{k,1}=1$ for every sequence $k$.
- Every candidate tree is undirected and connected, with exactly $n-1$ edges and root $1$.
- One common tree must support all $m$ DFS orders, though different child-processing orders may be used.
- Answer modulus: $10^9+7$.
- There is one instance per input.
- Time limit: **3 seconds**. Memory limit: **1024 MiB**.

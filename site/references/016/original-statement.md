# 16. DFS Order 4

## Problem Description

Consider all rooted trees on vertices $1,\ldots,n$ with root 1 such that the parent $f_i$ of vertex $i$ satisfies $1\le f_i<i$ for $i=2,\ldots,n$.

For each tree, run depth-first search from vertex 1. Append a vertex to the order when first visiting it, and visit its children in increasing vertex-label order. Thus each tree produces one deterministic DFS preorder.

There are $(n-1)!$ possible parent arrays. Count how many different DFS orders they produce, modulo the supplied prime $P$. Orders are different when some position contains different vertices.

## Input Format

Read from standard input.

Read two integers $n,P$.

## Output Format

Write to standard output.

Print the number of distinct DFS orders modulo $P$.

## Examples

### Example

Input:

```text
4 114514199
```

Output:

```text
2
```

## Explanation / Notes

For $n=4$, the two possible orders are $[1,2,3,4]$ and $[1,2,4,3]$. They can be obtained from parent arrays $(f_2,f_3,f_4)=(1,1,1)$ and $(1,1,2)$, respectively.

The number of distinct orders is requested, so different trees producing the same order count only once.

## Constraints

- One test case.
- $1\le n\le800$.
- $10^8\le P\le1.01\times10^9$.
- $P$ is guaranteed to be prime.
- Every considered tree has root 1 and satisfies $1\le f_i<i$.
- Child visitation order is increasing by label; it is not a free choice.
- Time limit: 2 seconds; memory limit: 1024 MB.

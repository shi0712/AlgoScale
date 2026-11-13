## Problem Description

A tree has vertices $1,\ldots,n$ and is rooted at vertex 1. Let $\operatorname{depth}(x)$ be the number of edges from $x$ to the root, and let $\operatorname{lca}(x,y)$ be the lowest common ancestor of $x$ and $y$.

Count permutations $p$ of all $n$ vertices for which the depths of consecutive-pair LCAs are nondecreasing. Formally, for every $1\le i<n-1$,

$$\operatorname{depth}(\operatorname{lca}(p_i,p_{i+1}))\le
\operatorname{depth}(\operatorname{lca}(p_{i+1},p_{i+2})).$$

Return the count modulo $1000000007$.

## Input Format

Read from standard input.

Read $n$, followed by $n-1$ lines of edge endpoints $x,y$.

## Output Format

Write to standard output.

Print the count modulo $1000000007$.

## Examples

### Example

Input:

```text
3
1 2
2 3
```

Output:

```text
4
```

## Explanation / Notes

The root is fixed at vertex 1. Equal successive LCA depths are allowed. For $n=2$, the displayed condition has no indices to check.

## Constraints

- One test case.
- $2\le n\le1000$.
- $1\le u_i,v_i\le n$ and $u_i\ne v_i$ for every edge.
- The input describes a tree.
- Modulus: $1000000007$.

- Time limit: 5 seconds; memory limit: 256 MB.

## Problem Description

A rooted tree has $n$ vertices, root 1, and parent $p_i$ for each vertex $i\ge2$. Depth is the number of edges from the root, so $d_1=0$ and $d_i=d_{p_i}+1$. The tree's height is its maximum depth and is guaranteed to be at most the given bound $m$.

Assign a nonnegative integer $a_i$ to every vertex. Let $S_i$ be the set of assigned values appearing in the subtree of $i$, including vertex $i$. The **mex** of a set is its smallest missing nonnegative integer.

Find the maximum possible value of

$$\sum_{i=1}^{n}\operatorname{mex}(S_i)$$

over all assignments.

## Input Format

Read from standard input.

Read the number of cases $t$. Each case consists of:

1. A line containing $n,m$.
2. A line containing the $n-1$ parents $p_2,p_3,\ldots,p_n$.

## Output Format

Write to standard output.

For each case, print one nonnegative integer: the maximum tree value.

## Examples

Input:

```text
1
5 2
1 1 2 2
```

Output:

```text
9
```

## Explanation / Notes

Assign $(a_1,a_2,a_3,a_4,a_5)=(3,2,0,0,1)$. The subtree mex values sum to $4+3+1+1+0=9$, which is optimal.

Repeated values contribute only once to a subtree's set. The height bound $m$ concerns the input tree's structure; it is not an upper bound on assigned weights.

## Constraints

- $1\le t\le5$.
- $2\le n\le8000$.
- $1\le m\le\min(n-1,800)$.
- $1\le p_i\le i-1$ for every $2\le i\le n$.
- Vertex 1 is the root, with depth zero.
- The input tree's height does not exceed $m$.
- Chosen weights $a_i$ may be arbitrary nonnegative integers; they are decision variables rather than input values.

- Time limit: 2 seconds.
- Memory limit: 524288 KiB (512 MiB).

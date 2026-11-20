## Problem Description

This is an interactive problem.

"Everybody! Doremy's Perfect Data Structure Class is about to start! Come and do your best if you want to have as much IQ as me!" In today's Data Structure class, Doremy is teaching everyone a powerful data structure — Doremy tree! Now she gives you a quiz to prove that you are paying attention in class.

Given an array $a$ of length $m$, Doremy tree supports the query $Q(l,r,k)$, where $1 \leq l \leq r \leq m$ and $1 \leq k \leq m$, which returns the number of distinct integers in the array $\left[\lfloor\frac{a_l}{k} \rfloor, \lfloor\frac{a_{l+1}}{k} \rfloor, \ldots, \lfloor\frac{a_r}{k} \rfloor\right]$.

Doremy has a secret permutation $p$ of integers from $1$ to $n$. You can make queries, in one query, you give $3$ integers $l,r,k$ ($1 \leq l \leq r \leq n$, $1 \leq k \leq n$) and receive the value of $Q(l,r,k)$ for the array $p$. Can you find the index $y$ ($1 \leq y \leq n$) such that $p_y=1$?

Note that the permutation $p$ is fixed before any queries are made.

## Input Format

Read n.

## Output Format

```text
? l r k
! y
```

Read the number of distinct values obtained by taking floor(p[i]/k) for l ≤ i ≤ r. Report the position of 1 in the fixed permutation. The final answer is free.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| | `? 1 1 3` |
| `1` | |

## Constraints

- 3 ≤ n ≤ 1024; 1 ≤ l ≤ r ≤ n; 1 ≤ k ≤ n.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

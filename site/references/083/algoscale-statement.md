## Problem Description

This is an interactive problem.

There is a secret permutation $p$ of $[0,1,2,\ldots,n-1]$. Your task is to find $2$ indices $x$ and $y$ ($1 \leq x, y \leq n$, possibly $x=y$) such that $p_x=0$ or $p_y=0$.

In one query, you give two integers $i$ and $j$ ($1 \leq i, j \leq n$, $i \neq j$) and receive the value of $\gcd(p_i,p_j)^\dagger$.

Note that the permutation $p$ is fixed before any queries are made and does not depend on the queries.

$^\dagger$ $\gcd(x, y)$ denotes the greatest common divisor (GCD) of integers $x$ and $y$. Note that $\gcd(x,0)=\gcd(0,x)=x$ for all positive integers $x$.

## Input Format

Read n.

## Output Format

```text
? i j
! x y
```

Read gcd(p[i],p[j]) after querying two distinct positions. The two reported indices may coincide. Read 1 after a correct report and then exit. Exit immediately on −1.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `? 1 2` |
| `1` | |
| | `! 1 2` |
| `1` | |

## Constraints

- 2 ≤ n ≤ 20000. Indices are 1-based.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

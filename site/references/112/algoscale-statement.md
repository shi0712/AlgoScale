## Problem Description

This is an interactive problem.

You are given a simple polygon, but before giving you the $n$ vertices of the polygon, Little Q has shuffled them.

Now you can ask Little Q queries, each of which consists of two integers $p$ and $q$ $( 0 \leq p / q \leq 1000 , 1 \leq q \leq 1000)$. He will tell you the total length of the points on the line $x = p / q$ that lie inside or on the boundary of the polygon, which can be represented as $r / s$ with two integers $r$ and $s$ $(r \geq 0 , s \geq 1 , \gcd ( r , s ) = 1)$. Here $\gcd ( r , s )$ is the greatest common divisor of $r$ and $s$ .

You need to find the area of the polygon, which can also be represented as $u / v$ with two integers $u$ and $v$ $( u \geq 0 , v \geq 1 , \gcd ( u , v ) = 1)$.

The polygon and the order of the vertices are determined before the start of the interaction and do not depend on your queries. In other words, the interactor is not adaptive.

## Input Format

Read n, then the shuffled coordinates of all n vertices, one pair per line.

## Output Format

```text
? p q
! u v
```

Read two integers r and s in lowest terms giving the intersection length with the vertical line x=p/q. Report the area as u/v in lowest terms. Exit after the final answer. The polygon and the order in which the points are supplied are fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| `0 0` | |
| `2 0` | |
| `0 2` | |
| | `! 2 1` |

## Constraints

- 3 ≤ n ≤ 1000. Coordinates are integers in [0,1000]. For a query, 0 ≤ p/q ≤ 1000 and 1 ≤ q ≤ 1000.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

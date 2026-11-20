## Problem Description

This is an interactive problem.

Vasya enjoys solving quizzes. He found a strange device and wants to know how it works.

This device encrypted with the tree (connected undirected graph without cycles) with $n$ vertices, numbered with integers from $1$ to $n$. To solve this quiz you should guess this tree.

Fortunately, this device can make one operation, using which you should guess the cipher. You can give the device an array $d_1, d_2, \ldots, d_n$ of non-negative integers. On the device, there are $n$ lamps, $i$-th of them is connected with $i$-th vertex of the tree. For all $i$ the light will turn on the $i$-th lamp, if there exist such vertex of the tree with number $j \neq i$ that $dist(i, j) \leq d_j$. Let's define $dist(i, j)$ as the distance between vertices $i$ and $j$ in tree or number of edges on the simple path between vertices $i$ and $j$.

Vasya wants to solve this quiz by using the device to guess the tree. Help him!

## Input Format

Read n.

## Output Format

```text
? d1 ... dn
!
a1 b1
...
a(n-1) b(n-1)
```

Read a length-n binary string describing the lamps. The radius at a vertex does not affect its own lamp. Report the edges of the fixed undirected tree in any order, one edge per line.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `? 1 0` |
| `01` | |
| | `!` |
| | `1 2` |

## Constraints

- 2 ≤ n ≤ 1000; 0 ≤ d[i] < n; endpoints are distinct vertex labels in [1,n].
- Time limit: **10 seconds**. Memory limit: **256 MB**.

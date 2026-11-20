## Problem Description

This is an interactive problem.

While procrastinating at work, Dilhan the elf stumbled upon two arrays $a$ and $b$. Initially, both of them consist of a single integer $2^k$ (i.e., $a=b=[2^k]$), where $k$ is a non-negative integer.

Dilhan then applied the following two types of operations an arbitrary number of times (possibly zero), in any order:

1. Flatten — Choose either $a$ or $b$, and select any element $x$ that is maximal within that array ($x$ does not need to be maximal in the other array). Then, replace $x$ with two copies of $\frac x2$ in the same position. This operation can only be applied if $x$ is even.
2. Concatenate — Set both $a$ and $b$ to be $a+b$, where $+$ denotes array concatenation.

After performing these operations, Dilhan discards $b$, hides $a$ from you, and challenges you to a game.

Let $n$ be the length of the hidden array $a$. You may make the following query:

* Choose an interval $[l, r]$ ($1\le l\le r\le n$), and Dilhan will tell you the sum $a_l+a_{l+1}+\cdots+a_r$.

Determine the value of the maximum element of $a$.

## Input Format

Read n.

## Output Format

```text
? l r
! m
```

Read the sum on the inclusive interval [l,r]. Report the maximum element of the finalized hidden array and exit. Terminate on −1.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1` | |
| | `? 1 1` |
| `8` | |
| | `! 8` |

## Constraints

- 1 ≤ n ≤ 100000; 1 ≤ a[i] ≤ 2³⁰. Indices are 1-based.
- Time limit: **10 seconds**. Memory limit: **512 MB**.

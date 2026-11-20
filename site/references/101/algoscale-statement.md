## Problem Description

This is an interactive problem.

Jury initially had a sequence $a$ of length $n$, such that $a_i = i$.

The jury chose three integers $i$, $j$, $k$, such that $1 \leq i < j < k \leq n$, $j - i > 1$. After that, Jury reversed subsegments $[i, j - 1]$ and $[j, k]$ of the sequence $a$.

Reversing a subsegment $[l, r]$ of the sequence $a$ means reversing the order of elements $a_l, a_{l+1}, \ldots, a_r$ in the sequence, i. e. $a_l$ is swapped with $a_r$, $a_{l+1}$ is swapped with $a_{r-1}$, etc.

You are given the number $n$ and you should find $i$, $j$, $k$ after asking some questions.

In one question you can choose two integers $l$ and $r$ ($1 \leq l \leq r \leq n$) and ask the number of inversions on the subsegment $[l, r]$ of the sequence $a$. You will be given the number of pairs $(i, j)$ such that $l \leq i < j \leq r$, and $a_i > a_j$.

Find the chosen numbers $i$, $j$, $k$.

## Input Format

Read n.

## Output Format

```text
? l r
! i j k
```

Read the inversion count of the closed interval [l,r] in the modified permutation. Report the three indices of the two reversals. The permutation is fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `4` | |
| | `? 1 4` |
| `2` | |
| | `! 1 3 4` |

## Constraints

- 4 ≤ n ≤ 10⁹; 1 ≤ i < j < k ≤ n; j−i > 1; 1 ≤ l ≤ r ≤ n.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

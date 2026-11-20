## Problem Description

This is an interactive problem.

We have hidden a permutation $p_1, p_2, \dots, p_n$ of numbers from $1$ to $n$ from you, where $n$ is even. You can try to guess it using the following queries:

$?$ $k$ $a_1$ $a_2$ $\dots$ $a_k$.

In response, you will learn if the average of elements with indexes $a_1, a_2, \dots, a_k$ is an integer. In other words, you will receive $1$ if $\frac{p_{a_1} + p_{a_2} + \dots + p_{a_k}}{k}$ is integer, and $0$ otherwise.

You have to guess the permutation.

Note that permutations $[p_1, p_2, \dots, p_k]$ and $[n + 1 - p_1, n + 1 - p_2, \dots, n + 1 - p_k]$ are indistinguishable. Therefore, you are guaranteed that $p_1 \le \frac{n}{2}$.

Note that the permutation $p$ is fixed before the start of the interaction and doesn't depend on your queries. In other words, interactor is not adaptive.

## Input Format

Read n.

## Output Format

```text
? k a1 ... ak
! p1 ... pn
```

List distinct positions. Read 1 if the corresponding hidden values have integral mean, or 0 otherwise. The values form a fixed permutation. Output its canonical orientation as specified above. Terminate on −1.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `! 1 2` |

## Constraints

- n is even and 2 ≤ n ≤ 800. Positions are numbered from 1 to n and 1 ≤ k ≤ n.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

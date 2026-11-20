## Problem Description

This is an interactive problem.

We picked an array of whole numbers $a_1, a_2, \ldots, a_n$ ($0 \le a_i \le 10^9$) and concealed exactly one zero in it! Your goal is to find the location of this zero, that is, to find $i$ such that $a_i = 0$.

You are allowed to make several queries to guess the answer. For each query, you can think up three distinct indices $i, j, k$, and we will tell you the value of $\max(a_i, a_j, a_k) - \min(a_i, a_j, a_k)$. In other words, we will tell you the difference between the maximum and the minimum number among $a_i$, $a_j$ and $a_k$.

That is, you have to tell us two numbers $i$ and $j$ and you win if $a_i = 0$ or $a_j = 0$.

Can you guess where we hid the zero?

Note that the array in each test case is fixed beforehand and will not change during the game. In other words, the interactor is not adaptive.

## Input Format

Read n.

## Output Format

```text
? i j k
! i j
```

Use three distinct indices. Read the largest of their three values minus the smallest. Report two indices, at least one of which holds the unique zero; they may coincide. Exit after the report. Terminate on −1.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `4` | |
| | `? 1 2 3` |
| `2` | |
| | `! 1 4` |

## Constraints

- 4 ≤ n ≤ 1000. All array values lie in [0,10⁹].
- Time limit: **10 seconds**. Memory limit: **256 MB**.

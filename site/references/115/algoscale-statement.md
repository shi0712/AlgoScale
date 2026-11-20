## Problem Description

This is an interactive problem.

Jury has hidden a permutation *p* of integers from 0 to *n* - 1. You know only the length *n*. Remind that in permutation all integers are distinct.

Let $b$ be the inverse of $p$, so that $p_{b_i}=i$. You may ask for the bitwise XOR of $p_i$ and $b_j$. Some different permutations give identical replies to every possible query. Find the number of such permutations and output any one of them.

## Input Format

Read n.

## Output Format

```text
? i j
!
count
p0 ... p(n-1)
```

Read p[i] XOR b[j], with b the inverse permutation. The indices need not be distinct. Two permutations are indistinguishable if all n² legal queries have the same replies. Report the size of the equivalence class, including the hidden permutation, and one member on the next line. The permutation is fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1` | |
| | `? 0 0` |
| `0` | |
| | `!` |
| | `1` |
| | `0` |

## Constraints

- 1 ≤ n ≤ 5000. Labels and indices range from 0 to n−1.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

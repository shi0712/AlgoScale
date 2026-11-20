## Problem Description

This is an interactive problem.

Made in Heaven is a rather curious Stand. Of course, it is (arguably) the strongest Stand in existence, but it is also an ardent puzzle enjoyer. For example, it gave Qtaro the following problem recently:

Made in Heaven has $n$ hidden integers $a_1, a_2, \dots, a_n$ ($3 \le n \le 5000$, $1 \le a_i \le 4$). Qtaro must determine all the $a_i$ by asking Made in Heaven some queries of the following form:

* In one query Qtaro is allowed to give Made in Heaven three distinct indexes $i$, $j$ and $k$ ($1 \leq i, j, k \leq n$).
* If $a_i, a_j, a_k$ form the sides of a non-degenerate triangle$^\dagger$, Made in Heaven will respond with the area of this triangle.
* Otherwise, Made in Heaven will respond with $0$.

Qtaro must either tell Made in Heaven all the values of the $a_i$, or report that it is not possible to uniquely determine them.

Unfortunately due to the universe reboot, Qtaro is not as smart as Jotaro. Please help Qtaro solve Made In Heaven's problem.

$^\dagger$ Three positive integers $a, b, c$ are said to form the sides of a non-degenerate triangle if and only if all of the following three inequalities hold:

* $a+b > c$,
* $b+c > a$,
* $c+a > b$.

## Input Format

Read n.

## Output Format

```text
? i j k
! a1 ... an
! -1
```

Use i < j < k. The integer reply is 0 for a degenerate or impossible triangle and 16 times the squared area otherwise. Report the hidden values if the complete set of possible queries uniquely determines them. Otherwise report −1. The values are fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| | `? 1 2 3` |
| `0` | |
| | `! -1` |

## Constraints

- 3 ≤ n ≤ 5000; each hidden value is an integer from 1 to 4. Indices are 1-based.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

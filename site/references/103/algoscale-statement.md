## Problem Description

This is an interactive problem.

There is an unknown integer $x$ ($1\le x\le n$). You want to find $x$.

At first, you have a set of integers $\{1, 2, \ldots, n\}$. You can perform the following operations:

* A $a$: find how many numbers are multiples of $a$ in the current set.
* B $a$: find how many numbers are multiples of $a$ in this set, and then delete all multiples of $a$, but $x$ will never be deleted (even if it is a multiple of $a$). In this operation, $a$ must be greater than $1$.
* C $a$: it means that you know that $x=a$. This operation can be only performed once.

Remember that in the operation of type B $a>1$ must hold.

Write a program, that will find the value of $x$.

## Input Format

Read n.

## Output Format

```text
A a
B a
C a
```

Read the count before any deletion for A and B. B removes the multiples except for x; answer C reports x and receives no reply. A, B and the final C each count as one operation. The hidden integer is fixed.

Fewer operations earn a higher score. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `B 2` |
| `1` | |
| | `A 2` |
| `1` | |
| | `C 2` |

## Constraints

- 1 ≤ x ≤ n ≤ 100000; A and C require 1 ≤ a ≤ n, and B requires 2 ≤ a ≤ n.
- Time limit: **10 seconds**. Memory limit: **512 MB**.

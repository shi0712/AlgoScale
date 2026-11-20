## Problem Description

This is an interactive problem.

Bobo had an array $a$ , initially equal to $( 1 , 2 , \ldots , n )$ . He did the following operations with the array.

* For each $i$ from 1 to $n$ in this order, Bobo picked some index $j$ such that $i \leq j \leq \operatorname* { m i n } ( n , i + 2 )$ , and swapped $a _ { i }$ and $a _ { j }$ . Of course, if $i = j$ , then nothing happened after the operation.

Your goal is to determine the final array. You may ask questions of the following type.

* ? i j meaning the question “How do $a _ { i }$ and $a _ { j }$ compare to each other?”. Bobo will respond to this with one symbol $<$ or $>$ , meaning that $a _ { i } < a _ { j }$ or $a _ { i } > a _ { j }$ , respectively.

## Input Format

Read n.

## Output Format

```text
? i j
! a1 ... an
```

Compare two distinct positions and read < or >. Report the final array in order. Replies may be adaptive but must agree with at least one legal execution of the swaps.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `? 1 2` |
| `>` | |
| | `! 2 1` |

## Constraints

- 1 ≤ n ≤ 30000; indices are 1-based.
- Time limit: **10 seconds**. Memory limit: **512 MB**.

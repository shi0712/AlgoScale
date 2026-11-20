## Problem Description

This is an interactive problem.

There is an array of $n$ cells, numbered from 1 to $n$ . For each pair of integers $( i , j )$ , where $1 \leq i \leq j \leq n$ , there is a barrier covering all cells from $i$ to $j$ , inclusive. Each barrier is either active or inactive. A cell is visible if there are no active barriers that cover it. Otherwise, the cell is invisible.

The state of each barrier is unknown to you. All you can observe is the number of visible cells. But you can flip the state of any barrier: if it’s active, it turns inactive, and the other way around. Your task is to make all barriers inactive, so that all cells become visible.

## Input Format

Read n, then read the initial number of visible cells.

## Output Format

```text
i j
```

Flip barrier [i,j] and read the new number of visible cells. When the count reaches n, exit without further output. Every flip counts, including the final one. The initial barrier states are fixed.

Fewer flips earn a higher score. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1` | |
| `0` | |
| | `1 1` |
| `1` | |

## Constraints

- 1 ≤ n ≤ 10; 1 ≤ i ≤ j ≤ n. At least one cell is initially invisible.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

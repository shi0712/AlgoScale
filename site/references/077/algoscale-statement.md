## Problem Description

This is an interactive problem.

Andrew the mushroom expert is investigating mushrooms native to Singapore.

As part of his research, Andrew collected $n$ mushrooms labelled $0$ to $n-1$. Each mushroom is of one of two species, which are called A and B.

Andrew knows that mushroom $0$ belongs to species A, but as the two species look the same, he does not know the species of mushrooms $1$ to $n-1$.

Fortunately, Andrew has a machine in his lab that can help with this. To use this machine, one should place two or more mushrooms in a row inside the machine (in any order) and turn the machine on. Then, the machine calculates the number of adjacent pairs of mushrooms that are of different species. For example, if you place mushrooms of species $[A, B, B, A]$ (in that order) into the machine, the result will be $2$.

Use this machine to help Andrew count the number of mushrooms of species A collected.

## Input Format

First read $n$. Mushroom 0 belongs to species A. The judge may answer adaptively. After every answer there is at least one assignment of species consistent with the entire interaction.

## Output Format

To use the machine, print:

```text
? k i0 i1 ... i(k-1)
```

Supply $k$ distinct mushroom labels in the order in which the mushrooms are placed in the machine. Read the number of adjacent pairs in this sequence whose species differ. The same mushroom may be used in different queries. To report the total number of mushrooms of species A, print:

```text
! count
```

End every command with a newline and flush standard output. Each query counts once. Fewer queries earn a higher score. The final answer is free. Exit immediately after printing it, or if the input stream ends unexpectedly.

## Examples

| Judge sends | Program sends |
| --- | --- |
| `4` |  |
|  | `? 2 0 1` |
| `1` |  |
|  | `? 2 1 2` |
| `0` |  |
|  | `? 2 0 3` |
| `0` |  |
|  | `! 2` |

## Constraints

- $2 \le n \le 20\,000$.
- $2\le k\le n$, and each label is between $0$ and $n-1$.
- The sum of $k$ over all machine uses must not exceed $100\,000$.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

## Problem Description

This is an interactive problem.

You are given a positive integer $N$.

The judge has a hidden positive integer $R$ and $N$ integers $A_1, A_2, \dots, A_N$. It is guaranteed that $|A_i|\le R$ and $\left|\displaystyle\sum_{i=1}^{N}A_i\right| \le R$.

There is a blackboard on which you can write integers with absolute values not exceeding $R$. Initially, the blackboard is empty.

The judge has written the values $A_1, A_2, \dots, A_N$ on the blackboard **in this order**. Your task is to make the blackboard contain only one value $\displaystyle\sum_{i=1}^{N}A_i$.

You cannot learn the values of $R$ and $A_i$ directly, but you can interact with the judge.

For a positive integer $i$, let $X_i$ be the $i$-th integer written on the blackboard. Specifically, $X_i = A_i$ for $i=1,2,\dots,N$.

In one interaction, you can specify two distinct positive integers $i$ and $j$ and choose one of the following actions:

* Perform addition. The judge will erase $X_i$ and $X_j$ from the blackboard and write $X_i + X_j$ on the blackboard.
  + $|X_i + X_j| \leq R$ must hold.
* Perform comparison. The judge will tell you whether $X_i < X_j$ is true or false.

Here, at the beginning of each interaction, the $i$-th and $j$-th integers written on the blackboard must not have been erased.

Perform the interactions appropriately so that after all interactions, the blackboard contains only one value $\displaystyle\sum_{i=1}^{N}A_i$.

The values of $R$ and $A_i$ are determined before the start of the conversation between your program and the judge.

## Input Format

Read N.

## Output Format

```text
+ i j
? i j
!
```

For addition, erase both operands, append their sum, and read its new label. For comparison, read 1 if the first value is strictly smaller and 0 otherwise. Both operands must be distinct and still on the board. Addition is legal only when the magnitude of the result is at most R. When exactly one value remains, print ! and exit. Both comparisons and additions count. Terminate on −1.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `+ 1 2` |
| `3` | |
| | `!` |

## Constraints

- 2 ≤ N ≤ 1000; 1 ≤ R ≤ 10⁹; each initial magnitude and the magnitude of the initial sum are at most R.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

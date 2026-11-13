## Problem Description

There are $n$ casinos and $m+1$ players. You are player $m+1$. The other players have already placed their dice: $a_{i,j}$ is player $j$'s dice count at casino $i$. Choose a nonnegative integer $b_i$ giving your dice count at each casino.

A casino is resolved independently:

1. Every dice count shared by at least two players is cancelled: all dice belonging to those players at this casino are removed.
2. Among the remaining dice, the player with the largest positive count wins. If no dice remain, nobody wins.

Let $w_j$ be the number of casinos won by player $j$. Your placement must satisfy $w_{m+1}\ge w_j$ for every other player, so a tie for the most casino wins is sufficient. Minimize $\sum_{i=1}^{n}b_i$ and output an optimal placement.

## Input Format

Read from standard input.

The first line contains the number of test cases $T$.

For each test case, read $n,m$, followed by $n$ rows of $m$ integers. Entry $a_{i,j}$ describes the existing dice of player $j$ at casino $i$.

## Output Format

Write to standard output.

For each test case, print $n$ integers $b_1,\ldots,b_n$ describing a minimum-total placement. Each output count must be between $0$ and $10^9$. An optimal placement within this range always exists. Any optimal placement is accepted.

## Examples

Input:

```text
1
1 4
20 0 20 0
```

Output:

```text
0
```

## Explanation / Notes

The two existing counts of 20 cancel one another. Every remaining count is zero, so nobody wins the casino. Placing zero dice leaves every player with zero wins and meets the requirement with the minimum possible total.

## Constraints

- $1\le T\le100$.
- $1\le n,m\le50$ in each test case.
- $0\le a_{i,j}\le10^8$; all input values are integers.
- At most **three test cases** have $n>20$ **or** $m>20$.
- Each chosen value must satisfy $0\le b_i\le10^9$.
- A feasible optimal answer exists within the permitted output range.
- Zero dice do not constitute a winning positive count.
- A tie in the number of casino wins is allowed; ties in dice counts within one casino cause cancellation instead.
- Time limit: **3 seconds**. Memory limit: **1024 MB**.

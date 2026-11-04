# 56. Bitaro the Brave 3

## Problem Description

A battle lasts until time $T$. Choose an integer difficulty $\ell$ between $1$ and $L$. There are $N$ monsters. Monster $i$ appears at time $S_i$, has strength $P_i$, and starts with $\ell H_i$ hit points.

An attack occupies one second and removes one hit point from a monster that has already appeared. A monster with zero hit points is defeated and cannot be attacked again. You may perform attacks any number of times before the battle ends.

Let $h_i$ be monster $i$'s remaining hit points immediately after time $T$. The final penalty is

$$\sum_{i=1}^{N} h_iP_i.$$

For each of $Q$ thresholds $M_j$, find the greatest difficulty for which some attack schedule gives a penalty at most $M_j$. Report zero if even difficulty $1$ is impossible.

## Input Format

Read from standard input.

- The first line contains $N$, $L$, and $T$.
- Each of the next $N$ lines contains $S_i$, $H_i$, and $P_i$.
- The next line contains $Q$.
- Each of the next $Q$ lines contains one threshold $M_j$.

## Output Format

Write to standard output.

Print $Q$ lines. Line $j$ contains the maximum feasible difficulty in $[1,L]$ for threshold $M_j$, or `0` if there is none.

## Examples

Input:

```text
2 2 10
0 9 2
8 5 1
1
20
```

Output:

```text
1
```

## Explanation / Notes

At difficulty $1$, attack monster 1 eight times during $[0, 8]$, monster 2 once during $[8, 9]$, and monster 1 once during $[9, 10]$. The remaining hit points are 0 and 4, giving a penalty of 4, which meets the threshold 20.

At difficulty $2$, the minimum penalty is 26: attacking monster 1 throughout the ten seconds leaves 8 and 10 hit points, giving $8\cdot 2+10\cdot 1=26$. Difficulty 2 therefore fails the threshold, so the answer is 1.

## Constraints

All input values are integers. There is one battle description per input.

| Parameter or guarantee | Bound |
| --- | --- |
| Number of monsters | $1\le N\le6000$ |
| Maximum difficulty | $1\le L\le10^7$ |
| Battle duration | $1\le T\le10^{18}$ |
| Appearance times | $0\le S_i<T$ |
| Base hit points | $H_i\ge1$ |
| Strengths | $P_i\ge1$ |
| Weighted total at difficulty 1 | $\sum_{i=1}^{N}H_iP_i\le10^{11}$ |
| Number of queries | $1\le Q\le10^6$ |
| Thresholds | $0\le M_j\le10^{18}$ |
| Threshold ordering | $M_1<M_2<\cdots<M_Q$ |

$H_i$ and $P_i$ are positive and satisfy the weighted-total bound above. Appearance times need not be sorted.

Time limit: **2 seconds**. Memory limit: **1024 MB**.

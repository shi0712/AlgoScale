## Problem Description

A card has one of $n$ suits and a rank in $[1,m]$. Each suit is either a trump or a regular suit; there may be several trump suits or none. Multiple physical cards may have the same suit and rank. Both players know both hands.

A card $(s_1,r_1)$ beats $(s_2,r_2)$ exactly when either $s_1=s_2$ and $r_1>r_2$, or $s_1$ is a trump suit and $s_2$ is not.

The game proceeds as follows:

1. The attacker chooses any card in their hand as the initial attack.
2. The defender must play an unused card that beats the attacker's most recent card. If no such card exists, the defender loses.
3. The attacker must play an unused card whose rank equals the rank of some card previously played by either player. If no such card exists, the attacker loses.
4. Repeat from step 2.

Count the initial attacks from which the attacker can force a win when both players make optimal later choices. Different physical copies count as different starting attacks, even when suit and rank coincide.

## Input Format

Read from standard input.

Read $n,m$. Next read $n$ flags $t_i$, where 1 marks a trump suit and 0 a regular suit.

Read an $n\times m$ matrix of attacker counts, followed by an $n\times m$ matrix of defender counts. Entry $(i,j)$ is the number of cards with suit $i$ and rank $j$ in that hand.

## Output Format

Write to standard output.

Print the number of winning initial attacks as an integer.

## Examples

### Example

Input:

```text
1 2
0
4294967296 0
0 2147483647
```

Output:

```text
4294967296
```

## Explanation / Notes

An already played card cannot be reused. Ranks from all earlier attacks and defenses remain available for checking the attacker's rank restriction.

Different trump suits do not automatically beat each other: the ordinary higher-rank rule requires the same suit.

## Constraints

- One test case.
- $1\le n\le100$ and $1\le m\le18$.
- Every $t_i$ is either 0 or 1.
- Every entry in each hand matrix is an integer in $[0,10^{12}]$.
- Each player's total number of cards is positive.
- Duplicate cards are allowed and count separately in the answer.
- No output modulus is specified; print the exact count. The answer may exceed the signed 32-bit integer range, but is at most $1.8\times10^{15}$.
- Time limit: 0.5 seconds; memory limit: 256 MB.

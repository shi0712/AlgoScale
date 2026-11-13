## Problem Description

Player X has $n$ ordinary pieces with strengths $a_1,\ldots,a_n$ and $k$ bombs. Player Y has $m$ ordinary pieces with strengths $b_1,\ldots,b_m$. Each player uniformly shuffles their entire collection into an order.

Both players deploy the first remaining piece. For two ordinary pieces, the weaker one is removed; if their strengths are equal, both are removed. A surviving piece stays deployed while the opposing player deploys their next piece. Whenever X's bomb meets any Y piece, both are removed.

A player wins when the other player has no pieces remaining but they still have at least one. If both run out simultaneously, the outcome is a draw. Compute the probabilities of an X win, a Y win, and a draw, modulo $998244353$.

## Input Format

Read from standard input.

Read $n,m,k$, then the $n$ values $a_i$, then the $m$ values $b_j$.

## Output Format

Write to standard output.

Print three integers on three separate lines: the probability of X winning, the probability of Y winning, and the probability of a draw, in that order, modulo $998244353$. A rational probability is represented by multiplying its numerator by the modular inverse of its denominator.

## Examples

### Example

Input:

```text
5 5 1
11 4 2 11 9
20 1 6 20 1
```

Output:

```text
0
1
0
```

## Explanation / Notes

Bombs belong only to X and are shuffled together with X's ordinary pieces before play. A bomb causes mutual removal regardless of the opposing strength. A winning ordinary piece is not removed merely because it has fought once.

## Constraints

- One test case.
- $1\le n,m\le1000$.
- $0\le k\le20$.
- $1\le a_i,b_j\le10^5$; repeated strengths are allowed.
- X has $n+k$ pieces in total, and Y has $m$.
- The shuffled order for each player's collection is uniform.
- Modulus: $998244353$.
- Time limit: 2 seconds; memory limit: 1024 MB.

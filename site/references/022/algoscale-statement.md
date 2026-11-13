## Problem Description

Player X has $n$ ordinary pieces with strengths $a_1,\ldots,a_n$ and $k$ bombs. Player Y has $m$ ordinary pieces with strengths $b_1,\ldots,b_m$. Each player independently and uniformly shuffles their entire collection into an order. Treat every piece as distinctly labeled when shuffling: all permutations of the labels are equally likely, while labels do not affect the rules of play.

Both players deploy the first remaining piece. For two ordinary pieces, the weaker one is removed; if their strengths are equal, both are removed. A surviving piece stays deployed while the opposing player deploys their next piece. Whenever X's bomb meets any Y piece, both are removed.

A player wins when the other player has no pieces remaining but they still have at least one. If both run out simultaneously, the outcome is a draw. Bombs count as remaining pieces. Compute the probabilities of an X win, a Y win, and a draw, modulo $998244353$.

## Input Format

Read from standard input.

Read $n,m,k$, then the $n$ values $a_i$, then the $m$ values $b_j$.

## Output Format

Write to standard output.

Print three integers on three separate lines: the probability of X winning, the probability of Y winning, and the probability of a draw, in that order, modulo $998244353$. For a probability written in lowest terms as $u/v$, print the unique integer $r$ with $0\le r<998244353$ and $rv\equiv u\pmod{998244353}$. The denominator $v$ is guaranteed not to be divisible by $998244353$.

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
- $1\le n,m\le10^6$.
- $0\le k\le5000$.
- $(n+m)(k+1)\le10^8$.
- $1\le a_i,b_j\le10^5$; repeated strengths are allowed.
- X has $n+k$ pieces in total, and Y has $m$. The value of $k$ may be zero or exceed $m$.
- The shuffled order for each player's collection is uniform.
- Modulus: $998244353$.
- Time limit: 1.5 seconds; memory limit: 256 MB.

## Problem Description

There are $n$ stations in a fixed order. Station $j$ has level $a_j$. Each of $m$ train services has parameters $(p_i,x_i,y_i)$ and stops at precisely the stations satisfying either of these conditions:

- $j\le p_i$ and $a_j\ge x_i$;
- $j>p_i$ and $a_j\ge y_i$.

Two different stations are directly connected if some service stops at both. A network is efficient when every pair of different stations of equal level is directly connected.

The levels $a$, switching points $p$, and thresholds $x$ are given. Count the arrays $(y_1,\ldots,y_m)$, with every $y_i$ an integer in $[1,k]$, that make the network efficient. Return the count modulo $998244353$.

## Input Format

Read from standard input.

The first line contains $n,m,k$. The second line contains $a_1,\ldots,a_n$. Each of the next $m$ lines contains $p_i,x_i$ for one service.

## Output Format

Write to standard output.

Print the number of valid arrays modulo $998244353$.

## Examples

### Example

Input:

```text
4 2 10
3 7 2 5
1 4
3 3
```

Output:

```text
100
```

## Explanation / Notes

All four station levels in the example are distinct, so there is no pair of different stations with equal level to constrain the choices. Both thresholds may take any value in $[1,10]$, giving $10^2=100$ arrays.

A service with $x_i=y_i=1$ visits every station. With $x_i=y_i=k$, it visits only level-$k$ stations. With $x_i=1,y_i=k$, it visits every station up to $p_i$ and only level-$k$ stations afterwards.

A level need not occur in the station sequence. The condition concerns direct service, not connectivity through transfers.

## Constraints

- One test case.
- $2\le n\le 10^3$.
- $1\le m\le 10^3$.
- $1\le k\le500$.
- $1\le a_j\le k$ for every station.
- $1\le p_i<n$ and $1\le x_i\le k$ for every service.
- Each unknown $y_i$ must lie in $[1,k]$.
- No requirement says that all $k$ levels appear.
- Modulus: $998244353$.
- Time limit: 3 seconds; memory limit: 2048 MB.

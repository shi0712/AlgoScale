# 8. Infinite Strife,

## Problem Description

For each positive integer $i$, let $a_i$ count the paths from $(0,0)$ to $(i,i)$ within the $i\times i$ grid using these moves:

- $(x,y)\to(x,y+2)$.
- $(x,y)\to(x+2,y)$.
- $(x,y)\to(x+1,y+1)$.

Different move sequences count as different paths.

One **prefix-product operation** replaces the entire sequence by $b$, where

$$b_i=\prod_{j=1}^{i}a_j.$$

For each query $(n,c)$, apply this operation $c$ times and compute the resulting $n$-th term modulo the prime $11$. Each query starts from the original path-count sequence.

## Input Format

Read from standard input.

Read the query count $T$, followed by $T$ lines containing two integers $n,c$.

## Output Format

Write to standard output.

Print one answer modulo $11$ for each query, on a separate line.

## Examples

Input:

```text
1
5 1
```

Output:

```text
10
```

## Explanation / Notes

The original sequence begins $1,3,7,19,51$. After one prefix-product operation, these terms become $1,3,21,399,20349$. Therefore the query $(5,1)$ has answer $20349\bmod11=10$.

## Constraints

- $1\le T\le5$.
- $1\le n\le10^{18}$.
- $1\le c\le3\times10^4$.
- $T,n,c$ are integers.
- Output values are reduced modulo $11$.

- Time limit: 2.5 seconds.
- Memory limit: 524288 KiB (512 MiB).

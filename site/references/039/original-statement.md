# 39. Mental Abuse To Humans

## Problem Description

Let $S_n=\{0,1,\ldots,n-1\}$. For sets of nonnegative integers, define the modular sumset

$$A+_nB=\{(a+b)\bmod n:\ a\in A,\ b\in B\}.$$

Count the subsets $A\subseteq S_n$ satisfying both requirements:

1. For each supplied pair $(x_i,t_i)$, $x_i\in A$ if and only if $t_i=1$.
2. $A+_n(S_n\setminus A)=S_n$.

Report the count modulo $998244353$.

## Input Format

Read from standard input.

The first line contains $T$. For each test case, read $n,m$, followed by $m$ lines containing $x_i,t_i$.

## Output Format

Write to standard output.

For each case, print the number of valid subsets modulo $998244353$.

## Examples

Input:

```text
1
3 0
```

Output:

```text
0
```

## Explanation / Notes

The second condition requires every residue modulo $n$ to be expressible as a sum of one element inside $A$ and one element outside it. Repeated representations of a residue do not matter because the result is a set.

A condition with $t_i=0$ forbids membership, while $t_i=1$ requires it. When $m=0$, there are no prescribed memberships.

## Constraints

- $1\le T\le5$.
- $1\le n\le10^{18}$.
- $0\le m\le5$.
- Every $x_i$ is an integer with $0\le x_i<n$.
- $t_i\in\{0,1\}$.
- All prescribed positions $x_1,\ldots,x_m$ are pairwise distinct, which also implies $m\le n$ for a valid instance.
- Output modulus: $998244353$.
- Time limit: 3 seconds.
- Memory limit: 1024 MB.

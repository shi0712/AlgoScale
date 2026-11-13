## Problem Description

For a permutation $p$ of $1,\ldots,n$, define:

- Its number of excedances: the number of indices $i\in[1,n]$ with $p_i>i$.
- Its number of descents: the number of indices $i\in[1,n-1]$ with $p_i>p_{i+1}$.

Let $h(n,m,k)$ count the permutations with exactly $m$ excedances and $k$ descents. Compute every $h(n,m,k)$ for the given $n$, modulo $M$.

## Input Format

Read from standard input.

Read $n,M$ on one line.

## Output Format

Write to standard output.

Print $n$ rows of $n$ integers. Entry $(i,j)$, using one-based row and column indices, must be

$$h(n,i-1,j-1)\bmod M.$$

## Examples

### Example

Input:

```text
3 998244353
```

Output:

```text
1 0 0
0 3 1
0 1 0
```

## Explanation / Notes

For $n=3$, the permutations grouped by (excedances, descents) are:

- $(0,0)$: $[1,2,3]$.
- $(1,1)$: $[2,1,3]$, $[3,1,2]$, $[1,3,2]$.
- $(1,2)$: $[3,2,1]$.
- $(2,1)$: $[2,3,1]$.

## Constraints

- One test case.
- $1\le n\le60$.
- $10^8\le M\le10^9$, and $M$ is prime.
- Both output statistics range from 0 to $n-1$.

- Time limit: 3 seconds; memory limit: 1024 MB.

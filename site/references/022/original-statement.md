# 22. EI Gave Me a Zero

## Problem Description

For given positive integers $n,m$, count arrays $a_1,\ldots,a_n$ with $1\le a_i\le m$ for which there is no pair $1\le i<j\le n$ satisfying

$$\max_{1\le k\le i}a_k=\min_{j\le k\le n}a_k.$$

Return the count modulo $998244353$.

## Input Format

Read from standard input.

Read $T$, followed by $T$ lines, each containing $n,m$.

## Output Format

Write to standard output.

For each test case, print its count modulo $998244353$ on a separate line.

## Examples

### Example

Input:

```text
1
3 2
```

Output:

```text
2
```

## Explanation / Notes

The forbidden equality compares a nonempty prefix with a nonempty suffix, and their endpoints must satisfy $i<j$. They need not be adjacent. Arrays are distinguished by their ordered entries.

## Constraints

- $1\le T\le10^5$.
- $1\le n\le300$.
- $1\le m\le10^9$.
- Every counted array entry lies in $[1,m]$.

- Modulus: $998244353$.
- Time limit: 2 seconds; memory limit: 256 MB.

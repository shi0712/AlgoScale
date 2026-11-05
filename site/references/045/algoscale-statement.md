## Problem Description

For integers $n,k$, define $f(n,k)$ as the number of ordered integer pairs $(a,b)$ satisfying

$$0\le a,b<n,\qquad k(a+b)\equiv ab\pmod n.$$

Given $n,m$, compute

$$\sum_{k=0}^{m}f(n,k)\pmod{998244353}.$$

## Input Format

Read from standard input.

Read the test-case count $T$. Each of the next $T$ lines contains $n,m$ for one case.

## Output Format

Write to standard output.

Print one line per case containing the required sum modulo $998244353$.

## Examples

Input:

```text
1
7 2
```

Output:

```text
25
```

## Explanation / Notes

The sum includes both endpoints $k=0$ and $k=m$. The pair coordinates are independently chosen residues modulo $n$, so $(a,b)$ and $(b,a)$ are separate pairs when $a\ne b$.

No primality assumption is made about $n$. When $n=1$, the constraints force $m=0$, and the only possible pair is $(0,0)$.

## Constraints

- $1\le T\le10^4$.
- $1\le n\le10^{12}$.
- $0\le m<n$.
- The counted pair coordinates satisfy the strict upper bounds $a<n$ and $b<n$.
- Output modulus: $998244353$.
- Time limit: 5 seconds.
- Memory limit: 256 MB.

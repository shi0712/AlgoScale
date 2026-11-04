# 38. Discrete Fourier Transform

## Problem Description

For integers $f_0,\ldots,f_{n-1}$, define

$$F_t=\sum_{s=0}^{n-1} f_s\exp\left(-\frac{2\pi\mathrm{i}st}{n}\right),\qquad 0\le t<n,$$

where $\mathrm{i}^2=-1$ and $\exp(\mathrm{i}\theta)=\cos\theta+\mathrm{i}\sin\theta$.

The index $k$ is fixed. Replace $f_k$ by any integer and minimize

$$\max_{0\le t<n}|F_t|.$$

Here $|p+q\mathrm{i}|=\sqrt{p^2+q^2}$ is the complex modulus.

## Input Format

Read from standard input.

The first line contains $n,k$. The second line contains $f_0,f_1,\ldots,f_{n-1}$ in that order. Indices are zero-based.

## Output Format

Write to standard output.

Print the minimum achievable maximum complex modulus as a real number. For submitted value $a$ and reference value $b$, the acceptance condition is

$$\frac{|a-b|}{\max(1,|b|)}\le10^{-9}.$$

## Examples

Input:

```text
3 2
1 1 0
```

Output:

```text
2.0
```

## Explanation / Notes

Only the entry at the supplied index $k$ may change. Its new value must be an integer, but it is **not** restricted to the input range $[-2000,2000]$; any integer is allowed.

The unchanged choice is permitted. For the sample, retaining the last entry as 0 yields a maximum modulus of 2, which is optimal.

## Constraints

- There is one input instance.
- $1\le n\le2000$.
- $0\le k<n$.
- Initially, every $f_i$ is an integer satisfying $-2000\le f_i\le2000$.
- The replacement value of $f_k$ is an arbitrary integer; no finite replacement bound is specified.
- The transform contains $n$ complex values indexed from 0 through $n-1$.
- The required absolute/relative error is at most $10^{-9}$ under the formula in Output Format.
- Time limit: 2 seconds.
- Memory limit: 512 MB.

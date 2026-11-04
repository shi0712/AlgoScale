# 15. Square Kingdom

## Problem Description

For each index $i\in[1,n]$, define a height

$$h_i=\left(i+\frac ba\right)^2.$$

For every unordered pair of distinct indices, create one ladder of length $|h_i-h_j|$. Sort the resulting $n(n-1)/2$ lengths in nondecreasing order, counting equal lengths separately. Find the $k$-th length and express it as a reduced fraction.

## Input Format

Read from standard input.

Read one line containing four integers $n,k,a,b$.

## Output Format

Write to standard output.

Print integers $p,q$ such that the requested length is $p/q$, $p\ge0$, $q\ge1$, and $\gcd(p,q)=1$.

## Examples

### Example

Input:

```text
3 1 3 1
```

Output:

```text
11 3
```

## Explanation / Notes

Each pair contributes its own ladder, even if its length equals another ladder's length. The rank $k$ is one-based. The offset inside the square is $b/a$.

## Constraints

- One test case.
- $2\le n\le10^{12}$.
- $1\le k\le\min\{n(n-1)/2,10^{12}\}$.
- $1\le a\le10^6$.
- $0\le b\le10^{12}$.
- All four input parameters are integers.
- The output fraction must be reduced, with a positive denominator.
- Time limit: 4 seconds; memory limit: 1024 MB.

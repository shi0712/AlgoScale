# 25. Old Problems

## Problem Description

Let $d(x)$ denote the number of positive divisors of $x$. For each triple $A,B,C$, compute

$$\sum_{i=1}^{A}\sum_{j=1}^{B}\sum_{k=1}^{C}d(ijk)\pmod{10^9+7}.$$

## Input Format

Read from standard input.

Read the number of cases $T$, followed by $T$ lines containing $A,B,C$.

## Output Format

Write to standard output.

For each case, print one line containing the requested value modulo $10^9+7$.

## Examples

### Example

Input:

```text
1
10 10 10
```

Output:

```text
11536
```

## Explanation / Notes

All three summation indices vary independently and include both endpoints. The divisor count is taken for their product $ijk$.

## Constraints

- $1\le T\le10$.
- For every case, $1\le A,B,C\le10^5$.
- Over the full input, $1\le\sum\max(A,B,C)\le2\times10^5$, where the sum is over test cases.

- Modulus: $1000000007$.
- Time limit: 5 seconds; memory limit: 512 MB.

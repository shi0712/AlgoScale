# 59. Sort

## Problem Description

You are given a permutation $p$ of $1,\ldots,n$. In one operation, choose $i$ uniformly from $1,\ldots,n$, sort the prefix $p_1,\ldots,p_i$ in ascending order, and separately sort the suffix $p_{i+1},\ldots,p_n$ in ascending order.

Repeat until the entire permutation is increasing. Find the expected number of operations, represented modulo $998244353$.

## Input Format

Read from standard input.

The first line contains $T$, the number of test cases. Each case has a line containing $n$, followed by a line containing the permutation $p_1,\ldots,p_n$.

## Output Format

Write to standard output.

For each test case, print the expected operation count modulo $998244353$ on a separate line.

## Examples

Input:

```text
1
2
2 1
```

Output:

```text
2
```

## Explanation / Notes

For the shown permutation, choosing $i=1$ changes nothing, while choosing $i=2$ sorts the whole permutation. Each choice has probability $1/2$, so the expected number of operations is 2.

When $i=n$, the suffix is empty and the operation sorts the whole permutation. Sorting the prefix and suffix separately does not otherwise sort across their boundary.

If the initial permutation is already increasing, no operation is needed. Division in a rational expectation modulo a prime uses a modular multiplicative inverse.

## Constraints

- $1\le T\le100$.
- $1\le n\le500$ in each test case.
- $1\le p_j\le n$ and every number in $1,\ldots,n$ appears exactly once.
- Across all test cases in one input, $\sum n\le2000$.
- The random split is chosen uniformly from the **$n$ indices $1,\ldots,n$**; index zero is not an option.
- Answer modulus: $998244353$.
- Time limit: **4 seconds**. Memory limit: **512 MB**.

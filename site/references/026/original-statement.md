## Problem Description

For a permutation $p$ of $1,\ldots,n$, let $q_i$ be the position of value $i$, so $p_{q_i}=i$. Call $p$ a turning permutation if, for every $i=2,\ldots,n-1$,

$$(q_i-q_{i-1})(q_i-q_{i+1})>0.$$

Find the $k$-th turning permutation in lexicographic order. If fewer than $k$ exist, report that fact. Lexicographic order compares the entries of $p$ from left to right, using the first differing entry.

## Input Format

Read from standard input.

Read $n,k$ on one line.

## Output Format

Write to standard output.

Print `-1` if fewer than $k$ turning permutations exist. Otherwise print the requested permutation on one line.

## Examples

### Example

Input:

```text
3 5
```

Output:

```text
-1
```

## Explanation / Notes

There are four turning permutations of length 3, so the fifth does not exist.

The turning condition concerns the positions of consecutive values, while the requested lexicographic order concerns the permutation itself.

## Constraints

- One test case.
- $3\le n\le50$.
- $1\le k\le10^{18}$; ranks are one-based.
- Every returned integer from 1 through $n$ must occur exactly once.
- The product in the turning condition must be strictly positive.
- Time limit: 1 second; memory limit: 512 MB.

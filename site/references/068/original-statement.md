## Problem Description

Choose an $n$-element subset of $\{1,\ldots,m\}$ uniformly at random. Apply these steps:

1. Sort the selected numbers as $a_1<\cdots<a_n$.
2. Form $d_1=a_1$ and $d_i=a_i-a_{i-1}$ for $i\ge2$.
3. Sort the $n$ differences in nondecreasing order, obtaining $b_1,\ldots,b_n$.
4. Form the final sequence $c_i=\sum_{j=1}^{i}b_j$.

Compute the expected value of every final entry $c_i$.

## Input Format

Read from standard input.

One line contains the integers $n$ and $m$.

## Output Format

Write to standard output.

Print $n$ lines. Line $i$ contains the expected value of $c_i$ as a real number. Each answer may have absolute or relative error at most $10^{-6}$.

## Examples

Input:

```text
3 5
```

Output:

```text
1
2.3
4.5
```

## Explanation / Notes

There are ten equally likely three-element subsets of $\{1, 2, 3, 4, 5\}$. Averaging the final sequence over these choices gives $1$, $2.3$, and $4.5$.

The difference sequence contains exactly $n$ terms: it includes the initial gap from zero to $a_1$ but does **not** append a gap from $a_n$ to $m$.

## Constraints

- $1\le n\le50$.
- $n\le m\le10000$.
- $n,m$ are integers.
- The initially selected integers are distinct and belong to $[1,m]$.
- Every valid $n$-element subset has the same selection probability.
- Output tolerance: absolute or relative error at most $10^{-6}$ for each of the $n$ expectations.
- There is one instance per input.
- Time limit: **1 second**.

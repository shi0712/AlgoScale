## Problem Description

Given sequences $a_1,\ldots,a_n$ and $b_1,\ldots,b_n$ and a threshold $k$, construct a bipartite graph with $n$ left vertices and $n$ right vertices. Connect left vertex $i$ to right vertex $j$ exactly when

$$a_i\mathbin{\oplus}b_j\ge k,$$

where $\oplus$ is bitwise XOR.

For each size $x=1,\ldots,n$, count the matchings containing exactly $x$ edges, modulo $998244353$.

## Input Format

Read from standard input.

Read three lines: $n,k$; then the $n$ entries of $a$; then the $n$ entries of $b$.

## Output Format

Write to standard output.

Print one line containing $n$ nonnegative integers, in increasing order of matching size. The $i$-th integer is the count for size $i$ modulo $998244353$. Print 0 for a size for which no matching exists.

## Examples

Input:

```text
9 5
1 7 2 8 4 9 2 5 10
1 3 2 4 5 8 8 8 9
```

Output:

```text
51 1034 10768 62195 200965 348924 294444 97344 7200
```

## Explanation / Notes

A matching is a set of edges in which no vertex is used twice. Vertices are distinguished by their indices, even if their associated numeric values are equal.

The threshold comparison is inclusive: XOR equal to $k$ creates an edge. A size-zero matching is not requested.

## Constraints

- There is one input instance.
- $1\le n\le10^5$.
- $0\le k<2^{60}$.
- $0\le a_i<2^{60}$ and $0\le b_i<2^{60}$ for every index.
- All inputs are integers; the upper bound $2^{60}$ is **exclusive**.
- Repeated values in either sequence are allowed.
- Every count is taken modulo $998244353$.
- Time limit: 1 second.
- Memory limit: 256 MB.

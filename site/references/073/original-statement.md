## Problem Description

There are $n$ students, numbered from $1$ to $n$. Their initial **affection** values are $a_1,a_2,\ldots,a_n$.

Sensei performs exactly one operation per day:

- Choose two integers $l$ and $r$ with $1\le l\le r\le n$, then increase $a_i$ by $1$ for every $i$ in the interval $[l,r]$.

Find the minimum number of days required so that at most $m$ distinct integers remain in the sequence $a_1,a_2,\ldots,a_n$.

## Input Format

Read from standard input.

The first line contains an integer $T$, the number of test cases.

For each test case:

- The first line contains two integers $n$ and $m$.
- The second line contains $n$ integers $a_1,a_2,\ldots,a_n$.

## Output Format

Write to standard output.

For each test case, print one non-negative integer: the minimum number of days required.

## Examples

Input:

```text
4
3 1
3 1 3
5 1
5 1 6 1 8
5 2
5 1 6 1 8
5 2
1 3 2 3 1
```

Output:

```text
2
12
3
1
```

## Constraints

- $1\le T\le10$.
- $1\le n\le400$.
- $1\le m\le2$.
- $0\le a_i\le100$.

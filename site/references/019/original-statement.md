# 19. Spirited Away

## Problem Description

Given nonnegative integers $a_1,\ldots,a_n$, define $B(l,r)$ as the set of all XOR values obtainable by choosing a subset of the entries in $a_l,\ldots,a_r$. The empty subset is allowed and contributes 0:

$$B(l,r)=\left\{\bigoplus_{i=l}^{r}c_i a_i:\ c_i\in\{0,1\}\right\}.$$

Count the distinct sets $B(l,r)$ over all nonempty contiguous intervals $1\le l\le r\le n$. In linear-algebra terminology, these are the distinct subspaces spanned by interval vectors in $\mathbb F_2^{32}$.

## Input Format

Read from standard input.

Read the test count $T$. Each case contains $n$ on one line and $a_1,\ldots,a_n$ on the next line.

## Output Format

Write to standard output.

For each test case, print one integer: the number of distinct sets $B(l,r)$.

## Examples

### Example

Input:

```text
1
3
1 2 3
```

Output:

```text
4
```

## Explanation / Notes

In the example, the four sets are $\{0,1\}$, $\{0,2\}$, $\{0,3\}$, and $\{0,1,2,3\}$.

The result counts different XOR-closure sets, not different intervals. XOR is the bitwise operation; for example, $4\oplus6=2$.

## Constraints

- $T\ge1$.
- $1\le n\le10^5$ per case.
- $\sum n\le10^5$ over all test cases.
- $0\le a_i<2^{32}$, so the largest allowed value is $4294967295$.
- Intervals are nonempty, but the subset used to form an XOR may be empty.
- Print the exact count; no output modulus is specified.
- Time limit: 4 seconds; memory limit: 1024 MB.

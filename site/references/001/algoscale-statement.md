## Problem Description

Given nonnegative integer arrays $A=(a_1,\ldots,a_n)$ and $B=(b_1,\ldots,b_m)$, transform $A$ using either of the following operations while its current length is at least two:

1. Remove the two leftmost elements and append their bitwise XOR to the right end.
2. Remove the two rightmost elements and prepend their bitwise XOR to the left end.

Each operation shortens the array by one. Decide whether exactly $n-m$ operations can produce $B$, and, if possible, construct one such sequence of operations.

## Input Format

Read from standard input.

The first line contains a nonnegative integer identifier $c$ and the number of cases $t$. Read and ignore $c$; $c=0$ identifies the sample input.

For each case, read:

1. $n,m$.
2. The $n$ values of $A$.
3. The $m$ values of $B$.

## Output Format

Write to standard output.

For each case, print `Yes` or `No` on one line. If the answer is `Yes`, print a second line containing exactly $n-m$ integers, each either `1` or `2`, in execution order. When $n=m$, this second line is empty.

The operation line is required for every `Yes` answer. Any valid operation sequence is accepted. The capitalization of `Yes` and `No` must match exactly. The checker ignores extra whitespace separating output items, but no extra non-whitespace content is allowed.

Each input file is scored independently. Every feasibility decision and every required operation sequence in that file must be correct to receive its score.

## Examples

Input:

```text
0 1
3 2
3 4 2
6 3
```

Output:

```text
Yes
2
```

## Explanation / Notes

Applying operation 2 removes the two rightmost values $4,2$ and prepends $4\mathbin{\mathrm{xor}}2=6$. The array changes from $[3,4,2]$ to $[6,3]$, as required.

## Constraints

- $1\le t\le1000$.
- $1\le m\le n\le2000$.
- Within each input file, $\sum_{i=1}^{t} n_i m_i\le4\times10^6$, where $n_i,m_i$ are the two array lengths in case $i$. All cases, including $m=1$ and $n=m$, count toward this sum. There are no separate bounds on $\sum n$ or $\sum m$.
- $0\le a_i<2^{30}$ for every $1\le i\le n$.
- $0\le b_i<2^{30}$ for every $1\le i\le m$.
- All array entries are integers. The upper bound $2^{30}$ is **excluded**.
- $c$ is a nonnegative integer and does not affect the requested result.

- Time limit: 1 second.
- Memory limit: 256 MB.

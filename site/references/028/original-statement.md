## Problem Description

Let $[n]=\{1,\ldots,n\}$. Arrange all nonempty subsets of $[n]$ of size at most two into a sequence. Each singleton and each unordered pair appears exactly once, so the sequence has $n(n+1)/2$ entries.

The arrangement must obey two rules:

- For each $x<y$, the pair $\{x,y\}$ appears between the singletons $\{x\}$ and $\{y\}$.
- The singleton $\{x\}$ appears before $\{x+1\}$ for every $x<n$.

For every $1\le i\le n$ and every position $1\le j\le n(n+1)/2$, count the valid arrangements in which singleton $\{i\}$ is at position $j$. Return all counts modulo the given prime $p$.

## Input Format

Read from standard input.

Read $n,p$ on one line.

## Output Format

Write to standard output.

Print $n$ rows, each with $n(n+1)/2$ integers. Row $i$, column $j$ is the number of valid arrangements placing $\{i\}$ at position $j$, modulo $p$.

## Examples

### Example

Input:

```text
1 851214401
```

Output:

```text
1
```

## Explanation / Notes

For $n=1$, the only subset is $\{1\}$, and it occupies position 1 in the unique valid arrangement.

Positions are one-based. Because the singletons occur in increasing order, a pair $\{x,y\}$ with $x<y$ must be after $\{x\}$ and before $\{y\}$.

## Constraints

- One test case.
- $1\le n\le40$.
- $10^8<p<10^9$; both inequalities are strict.
- $p$ is guaranteed to be prime.
- There are $n(n+1)/2$ positions in every arrangement.

- Time limit: 5 seconds; memory limit: 1024 MB.

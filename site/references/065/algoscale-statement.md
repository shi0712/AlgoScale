## Problem Description

An apple tree is a tree rooted at vertex $1$ in which every non-root vertex has **strictly fewer direct children than its parent**.

For a given integer $k$, a configuration is a pair $(v,S)$: $v$ is a center vertex, and $S$ is a set of $k-1$ other vertices, all at one common distance from $v$. Distance is the number of edges on the unique path between two vertices.

Count these pairs modulo $998244353$.

## Input Format

Read from standard input.

The first line contains the number of test cases $t$.

Each case starts with $n,k$. The next line contains $p_2,\ldots,p_n$, where $p_i$ is the parent of vertex $i$.

## Output Format

Write to standard output.

For every test case, print the number of valid pairs $(v,S)$ modulo $998244353$.

## Examples

Input:

```text
1
3 3
1 1
```

Output:

```text
1
```

## Explanation / Notes

The only valid pair has center $1$ and $S=\{2, 3\}$.

The center is distinguished from the selected set. The common distance is only required to exist; it is not an additional component of the counted object. When $k=1$, the selected set is empty, giving one pair for each possible center.

The condition compares numbers of **children**, not undirected degrees or subtree sizes.

## Constraints

- $1\le t\le10^4$.
- $2\le n\le10^6$ per test case.
- $1\le k\le n$.
- $1\le p_i<i$ for every $i=2,\ldots,n$.
- The input is guaranteed to describe an apple tree rooted at vertex $1$.
- For every non-root vertex $v$, its number of direct children is strictly less than its parent's number of direct children.
- Across all cases, $\sum n\le10^6$.
- The members of $S$ are distinct and do not include the center; $|S|=k-1$.
- Answer modulus: $998244353$.
- Time limit: **0.5 seconds**. Memory limit: **256 MB**.

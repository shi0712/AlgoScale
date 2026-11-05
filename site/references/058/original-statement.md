## Problem Description

There are $n$ items. Item $i$ has base power $p_i$ and bonus values $w_{i,1},\ldots,w_{i,p_i}$. Choose an ordering in which to process all items. A total base-power allowance of $k$ determines how much of each item is upgraded.

Before an item of base power $p_i$ is processed, let $s$ be the sum of the base powers of all earlier items. Its bonus is:

- $w_{i,p_i}$ if $s+p_i\le k$;
- $0$ if $s\ge k$;
- $w_{i,k-s}$ otherwise.

Maximize the sum of the bonuses by choosing the processing order. The base powers themselves are not part of the requested answer.

## Input Format

Read from standard input.

The first line contains $n$ and $k$.

Each of the next $n$ lines starts with $p_i$, followed by exactly $p_i$ integers $w_{i,1},\ldots,w_{i,p_i}$.

## Output Format

Write to standard output.

Print one integer: the maximum total bonus power.

## Examples

Input:

```text
4 5
2 1 3
2 1 1
2 3 1
2 1 3
```

Output:

```text
9
```

## Explanation / Notes

Bonus values need not increase with the upgraded amount. In particular, it is possible to have $w_{i,a}>w_{i,b}$ even when $a<b$.

For the example, process items 1 and 4 first, then item 3. Their bonuses are $3$, $3$, and $3$ respectively: four base-power units are fully upgraded before one unit of item 3 is upgraded. Item 2 receives no bonus. The total is $9$.

## Constraints

- $1\le n\le3000$.
- $0\le k\le3000$.
- $1\le p_i\le10$ for every item.
- $1\le w_{i,j}\le10^5$ for every $1\le j\le p_i$.
- All input values are integers.
- There is no monotonicity guarantee for $w_{i,1},\ldots,w_{i,p_i}$.
- Every item is processed exactly once, but its position in the order is freely chosen.
- There is one test instance per input.
- Time limit: **1 second**. Memory limit: **1024 MB**.

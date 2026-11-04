# 54. World Map

## Problem Description

There are $N$ countries and $M$ distinct unordered pairs of adjacent countries. Construct a square $K\times K$ grid colored with country numbers $1,\ldots,N$ so that:

1. Every country appears in at least one cell.
2. Every specified adjacent-country pair appears across at least one pair of side-sharing cells.
3. Every side-sharing cell pair with different colors corresponds to a specified adjacent-country pair.

A country may occupy several disconnected regions. A valid map is guaranteed to exist. Choose $K\le240$ and output a valid map. Finding the smallest possible $K$ is not required.

## Input Format

Read from standard input.

The first line contains an integer $T$, the number of test cases.

For each test case, the first line contains two integers $N$ and $M$. The next $M$ lines each contain two integers $A_i$ and $B_i$, specifying a pair of adjacent countries.

## Output Format

Write to standard output.

For each test case, print an integer $K$ on the first line. Then print $K$ lines, each containing $K$ integers. The integer in row $i$, column $j$ is the country assigned to that cell.

Every entry must lie in $[1,N]$, and the grid must satisfy all the stated conditions. Any valid map with $1\le K\le240$ is accepted.

## Examples

Input:

```text
1
4 4
1 2
1 3
2 4
3 4
```

Output:

```text
2
3 1
4 2
```

## Explanation / Notes

The only allowed distinct-color adjacencies are $(1,2),(1,3),(2,4),(3,4)$. In the displayed $2\times2$ map, each required pair shares a side, while countries 1 and 4 and countries 2 and 3 do not share a side.

Handle each test case independently.

## Constraints

- $1\le T\le50$.
- $1\le N\le40$.
- $0\le M\le N(N-1)/2$.
- For every $1\le i\le M$, $1\le A_i<B_i\le N$.
- All $M$ adjacency pairs are distinct.
- A map satisfying the required conditions exists.
- The grid dimension is a positive integer with $1\le K\le240$.
- Every row must have length $K$ and every cell color must belong to $[1,N]$.
- Every country must appear; diagonal contact alone does not create adjacency.
- Country regions need not be connected.

## Problem Description

A tree has $n$ vertices numbered $1$ through $n$. The numbers $1,2,\ldots,n$ occupy its vertices, with each number occurring exactly once.

Perform exactly $n-1$ operations. In each operation, choose an edge that has not yet been deleted, swap the numbers on its endpoints, and then delete the edge. Thus every original edge is used once.

After all operations, let $P_i$ be the vertex containing number $i$. Find the lexicographically smallest possible sequence $(P_1,P_2,\ldots,P_n)$.

## Input Format

Read from standard input.

The first line contains the number of test cases $T$. Each test case consists of:

1. A line containing $n$.
2. A line of $n$ integers; its $i$-th integer is the **initial vertex containing number $i$**, not the number initially stored at vertex $i$.
3. $n-1$ lines containing endpoints $x,y$ of the tree's edges.

## Output Format

Write to standard output.

For each test case, print one line containing $n$ space-separated integers: the lexicographically smallest possible final position sequence $P_1,\ldots,P_n$.

## Examples

Input:

```text
1
5
2 1 3 5 4
1 3
1 4
2 4
4 5
```

Output:

```text
1 3 4 2 5
```

## Explanation / Notes

Lexicographic comparison uses the first position at which two sequences differ. The smaller vertex number at that position makes the whole sequence smaller.

In the example, numbers $1,2,3,4,5$ initially occupy vertices $2,1,3,5,4$. An optimal ordering of edge deletions leaves them at vertices $1,3,4,2,5$, respectively.

When $n=1$, there are no edges to process and the only output is `1`. Output only the final position sequence, not an edge-deletion order.

## Constraints

- There are $1\le T\le10$ test cases.
- Each case is a tree: it has $n$ vertices and $n-1$ undirected edges and is connected.
- Vertex labels and initial positions lie in $[1,n]$; the initial position list is a permutation of $1,\ldots,n$.
- $1\le n\le10^5$.
- Across all test cases in one input file, $\sum n\le10^5$.
- Time limit: 1.5 seconds.
- Memory limit: 256 MB.

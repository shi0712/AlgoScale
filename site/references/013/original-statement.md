## Problem Description

There are $k$ independent trees. Tree $i$ has $n_i$ vertices, with vertex 1 designated as its hole, and a ball initially at vertex $s_i$.

For each stroke, choose one tree. Its ball moves uniformly at random to one of the adjacent vertices. The other balls stay where they are. The process stops as soon as any ball reaches vertex 1 of its tree.

Choices may depend on the current positions. Find the minimum possible expected number of strokes before the process stops.

## Input Format

Read from standard input.

The first line contains $k$. For each tree, read a line containing $n_i,s_i$, then $n_i-1$ lines containing edge endpoints $u_{i,j},v_{i,j}$.

## Output Format

Write to standard output.

Print the minimum expected number of strokes as a real number. If your answer is $a$ and the reference answer is $b$, it is accepted when

$$\frac{|a-b|}{\max(1,|b|)}\le10^{-6}.$$

## Examples

### Example

Input:

```text
2
4 2
1 2
2 3
3 4
4 3
1 2
2 3
3 4
```

Output:

```text
4.666666666666666
```

## Explanation / Notes

Only one ball moves on a stroke. The stopping condition is the first ball to enter a hole; reaching every hole is not required. Each initial vertex differs from the hole.

## Constraints

- The input contains exactly one test case.
- $1\le k\le300$.
- For every tree, $2\le s_i\le n_i\le300$.
- Every edge endpoint lies in $[1,n_i]$.
- The listed $n_i-1$ edges are guaranteed to form a tree.

- Required absolute-or-relative tolerance: $10^{-6}$, with the acceptance formula above.
- Time limit: 4 seconds; memory limit: 1024 MB.

## Problem Description

An undirected network on $n$ vertices is described by a matrix $W$. For distinct vertices $i,j$, the value $w_{i,j}$ is the width of their tunnel; zero denotes the absence of a tunnel. There is at most one tunnel per pair.

Assign nonnegative real values $e_1,\ldots,e_n$ with total 1. Maximize

$$E=\sum_{i=1}^{n}\sum_{j=i+1}^{n} e_i e_j w_{i,j}.$$

## Input Format

Read from standard input.

Read $n$, followed by $n$ rows of $n$ integers describing $W$. The $j$-th entry of row $i$ is $w_{i,j}$.

## Output Format

Write to standard output.

Print the maximum value of $E$ as a real number.

Your answer is accepted if

$$\frac{|A-B|}{\max(|A|,1)}\le10^{-6},$$

where $A$ is the submitted answer and $B$ is the reference answer.

## Examples

Input:

```text
2
0 1
1 0
```

Output:

```text
0.250000
```

## Explanation / Notes

Energy may be divided continuously; the $e_i$ are not restricted to integers. Vertices may receive zero energy. The expression includes each unordered pair once and does not use matrix diagonal entries.

For the example, assigning $1/2$ to each vertex gives $E=1/4$.

## Constraints

- There is one input instance.
- $1\le n\le10$.
- Every matrix entry is an integer with $0\le w_{i,j}\le1000$.
- The network is undirected and has at most one tunnel between each pair of vertices.
- Feasible energy assignments satisfy $e_i\ge0$ and $\sum_i e_i=1$.
- The network need not be connected.
- Precision: the explicit relative/absolute condition in Output Format is at most $10^{-6}$.
- Time limit: 1 second.
- Memory limit: 256 MB.

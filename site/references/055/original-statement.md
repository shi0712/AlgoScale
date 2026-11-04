# 55. Folding Game of Ohto Ai

## Problem Description

Start with an unrooted tree $G=(V,E)$. A path is a sequence of distinct vertices joined by consecutive edges. A diameter is a path with the greatest possible number of vertices.

While the tree contains more than one vertex, choose any current diameter $d_1,\ldots,d_k$, listed from one endpoint to the other, and fold it by merging symmetric vertices. Define

$$f(v)=\begin{cases}
d_{\min(i,k-i+1)},&v=d_i\text{ for some }1\le i\le k,\\
v,&v\text{ is not on the chosen diameter}.
\end{cases}$$

The resulting vertex set is $V'=\{f(v):v\in V\}$. Each old edge $\{u,v\}$ becomes $\{f(u),f(v)\}$ unless its endpoints coincide; such loops disappear. If multiple edges become the same unordered pair, retain one copy.

The resulting graph is again a tree. Repeat until only one vertex remains. Find the maximum possible number of folds over all valid choices of diameters.

## Input Format

Read from standard input.

The first line contains the test-case count $T$. For each case, read $N$, followed by $N-1$ lines containing edge endpoints $x,y$.

## Output Format

Write to standard output.

Print one integer per case: the maximum number of folds before the tree becomes a single vertex.

## Examples

Input:

```text
1
1
```

Output:

```text
0
```

## Explanation / Notes

The example tree has one vertex, so no fold is permitted and the answer is 0.

For larger trees, the chosen path must be a diameter of the **current** tree at each step. The goal is to maximize the number of operations.

## Constraints

- $1\le T\le3\times10^3$.
- $1\le N\le5\times10^4$ for each case.
- $\sum N\le5\times10^4$ across all test cases.
- Every edge satisfies $1\le x,y\le N$ and $x\ne y$.
- The $N-1$ edges form a tree.
- A fold is permitted only while the current vertex count exceeds 1.
- Off-diameter vertices are not directly identified with other vertices, although their incident edges follow the specified endpoint mapping.
- Loops are removed and parallel edges are merged after every fold; the result is guaranteed to remain a tree.
- Time limit: 3 seconds.
- Memory limit: 512 MB.

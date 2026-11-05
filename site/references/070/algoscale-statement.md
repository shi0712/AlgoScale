## Problem Description

Transform one weighted rooted tree into another at minimum cost. The children of every vertex have a fixed left-to-right order. The allowed operations are:

1. **Growing.** Add a new leaf $z$ as a child of $x$, inserting it anywhere in $x$'s ordered child list. If the new edge has weight $w$, the cost is $c_1w$.
2. **Expansion.** Choose a nonempty consecutive group of children $y_l,\ldots,y_r$ of $x$. Add a new vertex $z$ between $x$ and that group. Replace the group in $x$'s list by $z$, and make $y_l,\ldots,y_r$ the ordered children of $z$. Their incident edge weights remain unchanged. If the new edge $(x,z)$ has weight $w$, the cost is $c_1w$.
3. **Contraction.** Choose a child $y$ of $x$, delete $y$ and edge $(x,y)$, and splice $y$'s ordered children into $x$'s child list at $y$'s former position. Their incident edge weights remain unchanged. If the removed edge has weight $w$, the cost is $c_2w$.
4. **Relabeling.** Change an edge weight from $w_1$ to $w_2$, at cost $c_3|w_1-w_2|$.

Two restrictions apply throughout the sequence of operations:

- An edge newly added by growing or expansion cannot later be relabeled.
- An edge that has been relabeled cannot later be contracted.

The resulting tree must match the target under a vertex bijection preserving the root, child order, and every corresponding edge weight. Vertex names themselves need not match.

## Input Format

Read from standard input.

The first line contains the cost coefficients $c_1,c_2,c_3$.

Two tree descriptions follow, first the initial tree and then the target tree. Each description begins with its vertex count $n$, followed by $n$ vertex records in vertex-number order $1,\ldots,n$. The record for vertex $i$ starts with its number of children $k$, followed by $k$ ordered pairs

```text
child_1 weight_1 child_2 weight_2 ... child_k weight_k
```

The listed order is the left-to-right child order. A leaf has a record containing only `0`. The root is the unique vertex without a parent and need not be vertex $1$.

## Output Format

Write to standard output.

Print one integer: the minimum total transformation cost.

## Examples

Input:

```text
1 1 2
4
2 2 5 4 2
1 3 1
0
0
3
2 2 1 3 2
0
0
```

Output:

```text
5
```

## Explanation / Notes

In the sample, contract the weight-5 edge from the initial root to its first child. This promotes the weight-1 edge below that child to the root. The root is then incident to two ordered leaf edges of weights 1 and 2, matching the target. The cost is $1\cdot5=5$.

An expansion groups a consecutive interval of children, so it cannot arbitrarily reorder them. Similarly, contraction preserves the order of the promoted children and of the surrounding siblings. A contracted leaf simply disappears.

## Constraints

- $1\le c_1,c_2,c_3\le10^6$; these coefficients are integers.
- The initial tree has $1\le n\le100$ vertices.
- The target tree has $1\le m\le1000$ vertices.
- Every edge weight satisfies $0\le w\le10^6$.
- Both inputs are weighted rooted trees with ordered child lists.
- The answer fits in a signed 64-bit integer.
- Each vertex record contains exactly $k$ child/weight pairs after its child count.
- A tree with $n$ vertices has $n-1$ parent-child edges; each non-root vertex has exactly one parent.
- Expansion requires $1\le l\le r\le k$ for the selected parent's current child list.
- Newly introduced growing/expansion edges cannot be relabeled; relabeled edges cannot be contracted.
- Time limit: **1 second**. Memory limit: **256 MB**.

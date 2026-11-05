## Problem Description

Given two labeled, unrooted trees $S,T$ on the same $n$ vertices, transform $S$ into $T$ using at most $400$ operations.

An operation selects four distinct vertices $x,y,z,w$ that form the consecutive path $x-y-z-w$ in the **current** tree. Remove its three edges, then add three edges whose endpoints all belong to $\{x,y,z,w\}$. After the replacement, the whole graph must still be a simple tree. A replacement edge may be identical to one of the three removed edges. No self-loops or parallel edges are allowed, and edges not removed by this operation remain unchanged.

Produce a valid sequence within the operation budget, or report that none exists. You do not need to minimize the number of operations, and zero operations are allowed.

## Input Format

Read from standard input.

There is one test case. Read $n$, then $n-1$ pairs describing the edges of $S$, followed by another $n-1$ pairs describing the edges of $T$.

## Output Format

Write to standard output.

If no permitted transformation exists within the budget, print `NO`.

Otherwise print `YES` on the first line and the operation count $m$ on the second line. For every operation, print two lines:

1. $x\ y\ z\ w$, describing the path to remove from the current tree;
2. $a_1\ b_1\ a_2\ b_2\ a_3\ b_3$, describing the three replacement edges.

The final edge set must equal that of $T$. Any valid sequence with $0\le m\le400$ is accepted.

## Examples

Input:

```text
4
1 2
1 3
1 4
1 2
2 3
3 4
```

Output:

```text
NO
```

## Explanation / Notes

Tree equality uses vertex labels: the same unordered endpoint pairs must be edges in both trees. An isomorphic shape with different labels is insufficient.

In the example, the initial star has no path on four distinct vertices, so no operation is possible. It differs from the target path, so the answer is `NO`.

## Constraints

- $4\le n\le100$.
- Both input graphs are guaranteed to be trees with vertex labels in $[1,n]$.
- Every printed vertex label must lie in $[1,n]$.
- Each operation uses four pairwise distinct vertices forming a three-edge path at that exact stage.
- Every new edge endpoint must be one of those four vertices.
- The graph must remain a tree after **every** complete operation, not only at the end.
- The final graph must have exactly the target's labeled edge set.
- The number of operations must satisfy $0\le m\le400$.
- Time limit: 0.5 seconds.
- Memory limit: 256 MB.

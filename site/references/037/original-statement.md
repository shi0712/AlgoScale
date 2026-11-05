## Problem Description

A park is the region enclosed by a simple polygon. A receiver is placed at each polygon vertex. All receivers use the same radius $r$, and each covers the disk of radius $r$ centered at its vertex.

Find the smallest $r$ for which the union of these disks covers the entire park. Coverage includes points inside the polygon, not just its boundary.

## Input Format

Read from standard input.

Read $n$, followed by $n$ lines of integer coordinates $x,y$. These are the polygon's vertices in counter-clockwise order; the final vertex is connected back to the first.

## Output Format

Write to standard output.

Print the minimum common radius. The allowed absolute or relative error is at most $10^{-6}$.

## Examples

Input:

```text
5
0 0
170 0
140 30
60 30
0 70
```

Output:

```text
50
```

## Explanation / Notes

Receivers cover circular regions without a line-of-sight restriction. It does not matter if parts of their disks extend outside the park.

For the example, radius 35 leaves some park points uncovered, while radius 50 covers the whole polygon and is optimal.

## Constraints

- There is one input instance.
- $3\le n\le2000$.
- All coordinates are integers and satisfy $|x|,|y|\le10^4$.
- Vertices are supplied counter-clockwise.
- All polygon vertices are distinct.
- Edges do not intersect or touch, except that consecutive edges meet at their shared endpoint; the polygon is simple.
- Convexity is not guaranteed.
- Error tolerance: at most $10^{-6}$, absolute or relative.
- Time limit: 10 seconds.

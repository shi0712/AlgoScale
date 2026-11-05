## Problem Description

A lake occupies the interior of a simple polygon in the Cartesian plane. A runner starts at $(0,0)$, travels outward along a land-only path of length $k$, and then returns. The origin is not strictly inside the lake.

Find the minimum area of a region that contains every possible such route, regardless of how the runner chooses the path. Travel through the lake's interior is forbidden, but touching or moving along its boundary is allowed. Equivalently, the region must contain the land locations reachable from the origin with a path length of at most $k$.

## Input Format

Read from standard input.

Read $n,k$. Then read $n$ pairs $(x_i,y_i)$, the polygon's vertices in clockwise order.

## Output Format

Write to standard output.

Print the minimum area as a real number. The absolute error or relative error must be at most $10^{-6}$.

## Examples

### Example

Input:

```text
4 2
0 0
0 1
1 1
1 0
```

Output:

```text
10.995574287564
```

## Explanation / Notes

The minimum area in the example is $7\pi/2$.

The origin may lie on the lake boundary. In this example, it is a polygon vertex.

## Constraints

- One test case.
- $3\le n\le10^6$.
- $1\le k\le3000$.
- All coordinates are integers, with $|x_i|,|y_i|\le200$.
- The polygon is simple and its vertices are given clockwise.
- No three consecutive polygon vertices are collinear.
- $(0,0)$ is not strictly inside the polygon; it may be on the boundary.
- Required absolute or relative error: at most $10^{-6}$.
- There are no subtasks.
- Time limit: 2 seconds; memory limit: 512 MB.

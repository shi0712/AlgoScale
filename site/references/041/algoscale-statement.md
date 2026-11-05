## Problem Description

A gallery is a simple polygon containing $m$ cameras. A point is monitored if at least one camera can be joined to it by a line segment lying entirely in the closed polygonal region. Portions of the segment may lie along the polygon boundary.

Compute the area of the union of all monitored regions. Regions seen by multiple cameras are counted only once.

## Input Format

Read from standard input.

Read $n$, followed by $n$ pairs of coordinates for the polygon vertices in clockwise order. Then read $m$, followed by $m$ pairs of camera coordinates.

## Output Format

Write to standard output.

Print the monitored area as a real number. For submitted value $a$ and reference value $b$, the acceptance condition is

$$\frac{|a-b|}{\max(1,|b|)}\le10^{-6}.$$

## Examples

Input:

```text
8
0 0
0 5
5 5
5 10
10 10
10 5
50 5
50 0
1
8 7
```

Output:

```text
79.166666667
```

## Explanation / Notes

The polygon may be non-convex, so a camera does not necessarily see every interior point. Visibility is determined by the whole connecting segment, not solely by the locations of its endpoints. Boundary conventions do not change the requested area.

## Constraints

- There is one input instance.
- $3\le n\le200$.
- $1\le m\le200$.
- Polygon-vertex and camera coordinates are integers satisfying $|x|,|y|\le100$.
- The polygon is simple and its vertices are listed clockwise.
- No three consecutive polygon vertices are collinear, including consecutive triples crossing the end of the vertex list.
- Every camera lies strictly inside the polygon.
- Camera positions need not be distinct.
- Error tolerance follows the explicit inequality above, with $10^{-6}$ included.
- Time limit: 12 seconds.
- Memory limit: 256 MB.

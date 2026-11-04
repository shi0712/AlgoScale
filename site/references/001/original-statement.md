# 1. Enclosure of Land and Water

## Problem Description

A straight coastline divides the plane into land and water. A traveller starts on the land, at perpendicular distance $d$ from the coast, and must follow a closed route returning to that point within time $t$.

Movement has speed $v$ on land and speed $w$ in water. Each unit of enclosed land area is worth $V$, and each unit of enclosed water area is worth $W$.

Choose a route maximizing

$$V\cdot A_{\mathrm{land}}+W\cdot A_{\mathrm{water}},$$

where the two areas are the land and water enclosed by the route. Compute this maximum value.

## Input Format

Read from standard input.

Read the number of test cases $T$. Each test case consists of six real numbers, in this order:

```text
d v w V W t
```

## Output Format

Write to standard output.

For each test case, print the maximum enclosed value on its own line, with exactly three digits after the decimal point.

## Examples

Input:

```text
1
0.5 0.1 10 0.1 10 10
```

Output:

```text
0.008
```

## Explanation / Notes

The starting point is on the **land** side of the coastline. The route must end at its starting point, and its total travel time cannot exceed $t$. Land and water may have different travel speeds and different values per unit area.

## Constraints

| Quantity | Constraint |
| --- | --- |
| Number of cases $T$ | At most $100$ |
| $d,v,w,V,W,t$ | Each is a real number in $[0.1,10]$ |
| Coastline | A straight line |
| Start | On land, distance $d$ from the coast |
| Route time | At most $t$ |
| Output precision | Three digits after the decimal point |

- Time limit: 4 seconds.
- Memory limit: 524288 KiB (512 MiB).

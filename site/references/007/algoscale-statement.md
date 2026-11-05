## Problem Description

There are $n$ circles in the plane. Circle $i$ has center $(x_i,y_i)$, radius $r_i$, and travel cost $v_i$ per unit of arc length. Circle boundaries do not cross, but internal or external tangency is allowed. No two circles coincide.

Points $S$ and $T$ lie on the circles' circumferences, possibly on the same circle. Movement is allowed only along these circumferences, in either direction, and the direction may be changed along the way. Changing circles is possible at a shared tangent point and has no additional cost.

Find the least total travel cost from $S$ to $T$, or determine that no route exists.

## Input Format

Read from standard input.

The first line contains

```text
n Sx Sy Tx Ty
```

The next $n$ lines each contain $x_i,y_i,r_i,v_i$. All input quantities are integers.

## Output Format

Write to standard output.

Print `-1` if $T$ is unreachable. Otherwise, print the minimum cost as a finite real number, using decimal notation or decimal scientific notation. Do not print `NaN`, infinity, or any additional content. The value `-1` exclusively denotes unreachability and is not compared using the numerical tolerance.

Reachability must match the judge's answer exactly. For a reachable instance with exact answer $A$ and reported answer $Y$, your answer is accepted if either

$$\frac{|Y-A|}{A+1}\le10^{-6}\quad\text{or}\quad |Y-A|\le10^{-6}.$$

## Examples

Input:

```text
3 0 1 3 0
1 1 1 1
3 1 1 2
4 1 4 3
```

Output:

```text
6.283185307179586
```

## Explanation / Notes

A minimum route follows a semicircle on circle 1 to its tangency with circle 2, then a quarter-circle on circle 2 to $T$. Its cost is $\pi+2(\pi/2)=2\pi$.

## Constraints

| Parameter | Inclusive range |
| --- | --- |
| $n$ | $1\le n\le10^6$ |
| $S_x,S_y,T_x,T_y$ | Each has absolute value at most $10^9$ |
| $x_i,y_i$ | Each has absolute value at most $10^9$ |
| $r_i$ | $1\le r_i\le10^8$ |
| $v_i$ | $0\le v_i\le10^5$ |

- Exactly one instance is provided; there is no test-count field.
- Both endpoints lie on at least one supplied circle.
- Circles may be nested, disjoint, internally tangent, or externally tangent, but their boundaries may not cross.
- Coincident circles are forbidden: distinct circles cannot have both the same center and the same radius.
- Zero travel cost is allowed, and $S$ and $T$ may coincide.
- The error condition is given in Output Format, with denominator $A+1$.

- Time limit: 10 seconds.
- Memory limit: 512 MB.

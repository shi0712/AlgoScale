# Radio Direction Finding

Use zero-based coordinates, allowing the unfolded coordinate $n$ to represent zero. Write $n=2m+1$, $k=\lfloor(m+1)/2\rfloor$, and let $f(x)$ denote the receiver reading. Probe 0 and $k$ to obtain $u,v$, respectively. Set $S=u+v$ and $D=u-v$. For $n=3$, a reading of two at 0 identifies the other two points immediately. Otherwise a probe at 1 distinguishes the remaining possibilities.

For $n\ge5$, a single hidden point $p$ contributes the following pair of distances.

| Arc | Unfolded coordinate | Distance to 0 | Distance to $k$ |
| --- | --- | --- | --- |
| I | $[0,k]$ | $p$ | $k-p$ |
| II | $[k,m]$ | $p$ | $p-k$ |
| III | $[m+1,m+k]$ | $n-p$ | $p-k$ |
| IV | $[m+k+1,n]$ | $n-p$ | $n+k-p$ |

**One short arc.** Test the extrema first. If $S=2k$ or $D=2k$, both points attain the corresponding single-point extremum on arc I or II, and their coordinate sum is $P=u$. If $S=2(n-k)$ or $D=-2k$, the points lie on III or IV and $P=2n-u$. A reading at $\lfloor P/2\rfloor$ gives the gap $g$ since both points and their midpoint lie on one short arc. Return $(P-g)/2$ and $(P+g)/2$. Reuse a reading if the midpoint has already been probed.

**Opposite arcs.** After removing the extrema, $S=n$ places the points on I and III, while $D=0$ places them on II and IV, allowing shared endpoint representations. Substituting the four rows verifies these conditions. In either case the second unfolded coordinate is $b=a+n-u$. The candidate interval for $a$ is

| Condition | Lower endpoint | Upper endpoint |
| --- | --- | --- |
| $S=n$ | $\max(0,u-m)$ | $\min(k,u-m+k-1)$ |
| $D=0$ | $\max(k,u-m+k)$ | $\min(m,u)$ |

For a candidate probe $x$, set $h=x-a$. Then $f(x)=|h|+\min(n-u-h,u+h)$. For $u\le m$ the reading equals $u$ exactly when $x\le a$. For $u>m$ it equals $n-u$ exactly when $a\le x$. Binary search therefore recovers $a$ and then $b$.

**Adjacent arcs.** In all remaining cases the two linear readings uniquely determine the coordinates. Substituting the arc formulas gives

| Condition | Coordinates |
| --- | --- |
| $S$ even, $D>0$ | $D/2,\ S/2$ |
| $S$ even, $D<0$ | $k+D/2,\ n+k-S/2$ |
| $S$ odd, $D>0$ | $(S-n+2k)/2,\ (n+2k-D)/2$ |
| $S$ odd, $D<0$ | $(n-D)/2,\ (3n-S)/2$ |

No additional query is needed in these cases. Reduce unfolded coordinates modulo $n$ and add one before reporting.

The binary-search interval contains at most $\lceil(n+1)/4\rceil$ values. The initial probes and binary search together cost at most $2+\lceil\log_2\lceil(n+1)/4\rceil\rceil=\lceil\log_2n\rceil$ for odd $n\ge5$. The same-arc branch needs at most three readings, and $n=3$ needs at most two. Therefore the reference uses at most **30 receiver queries**. Multiple supplied cases attain 30. The implementation uses $O(\log n)$ time, $O(1)$ extra space and 64-bit intermediate arithmetic.

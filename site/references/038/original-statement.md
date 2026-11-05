## Problem Description

The ground is a piecewise-linear profile through points $(x_1,y_1),\ldots,(x_n,y_n)$ in increasing horizontal order. Build a horizontal bridge at elevation $h$ from $x_1$ to $x_n$.

Pillars must be placed at the first and last profile points; additional pillars may be placed only at other supplied points. A pillar at $(x_i,y_i)$ has height $h-y_i$. Between consecutive pillars a distance $d$ apart, the bridge has a semicircular arch of radius $r=d/2$, with its highest point at elevation $h$. The arch may touch the ground but may never lie below it.

For pillar heights $h_1,\ldots,h_k$ and consecutive horizontal gaps $d_1,\ldots,d_{k-1}$, the cost is

$$\alpha\sum_{j=1}^{k}h_j+\beta\sum_{j=1}^{k-1}d_j^2.$$

Find the minimum cost, or determine that no valid bridge can be built.

## Input Format

Read from standard input.

Read $n,h,\alpha,\beta$. The next $n$ lines contain $x_i,y_i$ in increasing order of $x_i$.

## Output Format

Write to standard output.

Print the minimum cost. If no valid construction exists, print the lowercase word `impossible`.

## Examples

Input:

```text
4 10 1 1
0 0
1 9
9 9
10 0
```

Output:

```text
impossible
```

## Explanation / Notes

For pillars at horizontal positions $a<b$, put $r=(b-a)/2$ and $c=(a+b)/2$. The arch elevation at $x\in[a,b]$ is

$$h-r+\sqrt{r^2-(x-c)^2}.$$

The ground profile must be at or below this curve throughout the span. The bridge height $h$ is an elevation above sea level, not a clearance measured independently from the changing ground.

For the example, no valid bridge can be constructed, so the answer is `impossible`.

## Constraints

- There is one input instance.
- $2\le n\le10^4$.
- $1\le h\le10^5$.
- $1\le\alpha,\beta\le10^4$.
- $0\le x_1<x_2<\cdots<x_n\le10^5$.
- Every ground elevation satisfies $0\le y_i<h$; the upper inequality is strict.
- All supplied values are integers.
- Only supplied profile points may support pillars; the first and last must do so.
- Contact between an arch and the ground is allowed. Extending below the ground is forbidden.
- The problem does not guarantee that a feasible bridge exists.
- Time limit: 10 seconds.

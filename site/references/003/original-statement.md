# 3. Years

## Problem Description

Let $G$ be a connected, simple, weighted undirected graph with vertices $1,\ldots,n$ and edges $(u_i,v_i,w_i)$.

Independently for every undirected edge, construct a directed graph $G'$ by choosing one of these four equally likely outcomes:

- Keep both arcs $u_i\to v_i$ and $v_i\to u_i$.
- Keep only $u_i\to v_i$.
- Keep only $v_i\to u_i$.
- Keep neither arc.

Every retained arc has weight $w_i$.

An **outward spanning arborescence** is a set of $n-1$ directed edges for which some root can reach every vertex using only those edges. The root may be any vertex. Its weight is the sum of its edges' weights.

Compute the probability that $G'$ has an outward spanning arborescence and that the minimum weight of such an arborescence equals the weight of a minimum spanning tree of $G$. Report the probability modulo $10^9+7$.

## Input Format

Read from standard input.

Read an unused integer $c$ followed by the number of cases $T$. Ignore $c$.

Each case starts with $n,m$, followed by $m$ lines. Line $i$ contains $u_i,v_i,w_i$.

## Output Format

Write to standard output.

Print one modular probability per case. If the reduced exact probability is $a/b$, output the unique $x$ with

$$0\le x<10^9+7,\qquad bx\equiv a\pmod{10^9+7}.$$

The denominator is guaranteed not to be divisible by $10^9+7$.

## Examples

Input:

```text
0 1
2 1
1 2 1
```

Output:

```text
750000006
```

## Explanation / Notes

There is only one undirected edge. The condition holds whenever at least one directed arc remains, which occurs with probability $3/4$. Its representation modulo $10^9+7$ is `750000006`.

## Constraints

- $1\le T\le5$.
- $2\le n\le15$.
- $n-1\le m\le n(n-1)/2$.
- $1\le u_i<v_i\le n$.
- $1\le w_i\le m$; all weights are integers.
- All unordered endpoint pairs are distinct; there are no loops or parallel edges.
- $G$ is connected.
- The $m$ random edge outcomes are independent, with probability $1/4$ for each of the four outcomes.

- Time limit: 3 seconds.
- Memory limit: 524288 KiB (512 MiB).

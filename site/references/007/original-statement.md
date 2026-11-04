# 7. Industrial System

## Problem Description

An undirected tree connects $n$ devices numbered $1,\ldots,n$. Choosing a final device $x$ roots the tree at $x$ and directs every edge toward that root.

For that root, define device $y$'s product recursively as the **multiset of the products of all its proper descendants**, not just its children:

$$a_{x,y}=\{a_{x,z}:z\text{ is a proper descendant of }y\}.$$

A leaf's product is the empty multiset. Multiplicities matter.

To compare two products, sort their multiset elements in decreasing product order and compare the resulting sequences lexicographically. At the first unequal position, the larger element determines the larger product. If one sequence is a proper prefix of the other, the longer sequence is larger. Equal sequences represent equal products. This recursively defines the comparison, starting with empty products.

Define the descending rank, allowing ties, by

$$f(x,y)=1+\#\{z:a_{x,z}>a_{x,y}\}.$$

For each query $(s,t,o_x,o_y,r)$, let $P(s,t)$ be the set of vertices on the unique simple path from $s$ to $t$, **including both endpoints**. Define

$$X=\begin{cases}\{s\},&o_x=0,\\P(s,t),&o_x=1,\end{cases}\qquad
Y=\begin{cases}\{t\},&o_y=0,\\P(s,t),&o_y=1.\end{cases}$$

Count the ordered pairs $(x,y)\in X\times Y$ for which $f(x,y)\le r$.

## Input Format

Read from standard input.

Read the following, in order:

1. An unused nonnegative integer $c$, which should be read and ignored.
2. $n$.
3. $n-1$ lines containing tree edges $u_i,v_i$.
4. The query count $m$.
5. $m$ lines, each containing $s,t,o_x,o_y,r$.

There is one tree in the input; $c$ is not a case count.

## Output Format

Write to standard output.

Print one nonnegative integer per query: the number of qualifying ordered pairs.

## Examples

Input:

```text
0
3
1 2
2 3
1
1 2 0 0 2
```

Output:

```text
1
```

## Explanation / Notes

Rooting the path $1-2-3$ at vertex 1 gives

$$a_{1,3}=\varnothing,\quad a_{1,2}=\{\varnothing\},\quad a_{1,1}=\{\varnothing,\{\varnothing\}\}.$$

Thus $f(1,2)=2$. The query has $X=\{1\}$, $Y=\{2\}$, and $r=2$, so its only candidate pair qualifies.

## Constraints

- $2\le n\le10^5$.
- Exactly $n-1$ edges are given; their endpoints are in $[1,n]$ and they form a tree.
- $1\le m\le10^5$.
- For every query, $1\le s,t,r\le n$ and $o_x,o_y\in\{0,1\}$.
- Queries with $s=t$ and pairs with $x=y$ are allowed.
- Equal products have equal ranks; each rank counts all strictly larger products, rather than the number of distinct larger values.
- An answer is at most $n^2\le10^{10}$, so it need not fit in a signed 32-bit integer.

- Time limit: 4 seconds.
- Memory limit: 1048576 KiB (1024 MiB).

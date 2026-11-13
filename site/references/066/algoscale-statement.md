## Problem Description

You are given a directed graph. Find the infimum variance among all walks from vertex $1$ to vertex $n$.

A walk from vertex $1$ to vertex $n$ is a sequence of edges $p=(e_1,e_2,\ldots,e_l)$ such that:

- the starting vertex of $e_1$ is vertex $1$;
- the ending vertex of $e_l$ is vertex $n$;
- for every consecutive pair, the ending vertex of $e_i$ is the starting vertex of $e_{i+1}$.

Edges in a walk do not need to be distinct.

The variance of a walk is

$$
D(p)=\frac{1}{l}\sum_{i=1}^{l}\left(w_{e_i}-\overline{w}\right)^2,
$$

where

$$
\overline{w}=\frac{1}{l}\sum_{i=1}^{l}w_{e_i}
$$

is the average edge weight of the walk.

## Input Format

Read from standard input.

The first line contains two integers $n$ and $m$, the number of vertices and directed edges.

Each of the next $m$ lines contains three integers $x_i$, $y_i$, and $w_i$, describing a directed edge from $x_i$ to $y_i$ with weight $w_i$. Parallel edges are allowed and are treated as distinct edges.

## Output Format

Write to standard output.

Print one real number: the infimum variance among all walks from vertex $1$ to vertex $n$. If no such walk exists, print `-1`.

If the required infimum is $a$ and your reported value is $b$, the answer is accepted when

$$
|a-b|\le10^{-9}\max\{1,|a|,|b|\}.
$$

## Examples

Input:

```text
4 4
1 2 3
1 3 4
2 4 5
3 4 7
```

Output:

```text
1.000000000000000
```

## Explanation / Notes

The walk $1\to2\to4$ uses edge weights $3$ and $5$. Its average weight is $4$, and its variance is $1$.

Because edges may be repeated, the set of attainable walk variances need not contain its greatest lower bound; therefore the problem asks for the infimum rather than necessarily a minimum.

## Constraints

- $2\le n\le100$.
- $1\le m\le1000$.
- $1\le x_i,y_i\le n$.
- $x_i\ne y_i$.
- $0\le w_i\le20$.
- Answers are checked using the tolerance formula in the output specification.
- Time limit: 1 second. Memory limit: 256 MB.

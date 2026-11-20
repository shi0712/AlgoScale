## Problem Description

This is an interactive problem.

The jury has a hidden tree with $n$ vertices, numbered from 1 to $n$ . The tree has $n - 1$ edges, the $i$ -th edge connects vertices $s _ { i }$ and $f _ { i }$ . You do not know the edges, but you can ask jury questions about the tree.

Each query you make must contain one vertex $v$ and a binary string $b$ of length $n - 1$ . The jury does the following things for the query:

* The tree is considered as a rooted tree with vertex $v$ being the root.   
* Each edge of the tree becomes directed. More specifically, if $b _ { i } = 1$ , then the $i$ -th edge becomes directed from $s _ { i }$ to $f _ { i }$ . Otherwise, it is directed from $f _ { i }$ to $s _ { i }$ .   
* The answer to the query is the number of edges that are directed towards the root. In other words, the answer to the query is the number of tree edges for which the distance between its start to $\boldsymbol { v }$ is larger than the distance between its end and $v$ . Here the distance between two vertices is the number of edges on the simple undirected path between these vertices.

Your task is to guess all $s _ { i }$ and $f _ { i }$.

## Input Format

Read n.

## Output Format

```text
? v b
! s1 f1 s2 f2 ... s(n-1) f(n-1)
```

Use root v and a bit string b of length n−1. Read the number of edges directed toward the root. Report both endpoints of every edge in its original edge order and orientation. The ordered edges are fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `? 1 1` |
| `0` | |
| | `! 1 2` |

## Constraints

- 2 ≤ n ≤ 500; 1 ≤ v ≤ n.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

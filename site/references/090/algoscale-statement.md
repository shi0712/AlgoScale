## Problem Description

This is an interactive problem.

Baudelaire is very rich, so he bought a tree of size $n$ that is rooted at some arbitrary node. Additionally, every node has a value of $1$ or $-1$. Every node is adjacent to node $1$. However, please note that node $1$ is not necessarily the root.

Cow the Nerd saw the tree and fell in love with it. However, computer science doesn't pay him enough, so he can't afford to buy it. Baudelaire decided to play a game with Cow the Nerd, and if he won, he would gift him the tree.

Cow the Nerd does not know which node is the root, and he doesn't know the values of the nodes either. However, he can ask Baudelaire queries of two types:

* $1$ $k$ $u_1$ $u_2$ $...$ $u_k$: Let $f(u)$ be the sum of the values of all nodes in the path from the root of the tree to node $u$. Cow the Nerd may choose an integer $k$ $(1 \le k \le n)$ and $k$ nodes $u_1, u_2, ..., u_k$, and he will receive the value $f(u_1) + f(u_2) + ... + f(u_k)$.
* $2$ $u$: Baudelaire will toggle the value of node $u$. Specifically, if the value of $u$ is $1$ it will become $-1$, and vice versa.

Cow the Nerd wins if he guesses the value of every node correctly (the values of the final tree, after performing the queries). Can you help him win?

## Input Format

Read n, followed by n−1 undirected edges, one pair of endpoints per line.

## Output Format

```text
? 1 k u1 ... uk
? 2 u
! a1 ... an
```

After type 1 read the sum of the k requested root-path sums. Repeated vertices are permitted. Type 2 flips one vertex and has no reply. Both operations count. Report the node values after all flips, in label order. The root and initial values are fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| `1 2` | |
| | `? 1 1 1` |
| `1` | |
| | `? 2 2` |
| | `! 1 -1` |

## Constraints

- 2 ≤ n ≤ 1000; 1 ≤ k ≤ n. Node labels lie in [1,n] and values are +1 or −1. Every edge is incident to vertex 1.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

## Problem Description

This is an interactive problem.

Bob has a simple undirected connected graph (without self-loops and multiple edges). He wants to learn whether his graph is bipartite (that is, you can paint all vertices of the graph into two colors so that there is no edge connecting two vertices of the same color) or not. As he is not very good at programming, he asked Alice for help. He does not want to disclose his graph to Alice, but he agreed that Alice can ask him some questions about the graph.

The only question that Alice can ask is the following: she sends $s$ — a subset of vertices of the original graph. Bob answers with the number of edges that have both endpoints in $s$. Furthermore, he suspects that Alice might introduce false messages to their communication channel, so when Alice finally tells him whether the graph is bipartite or not, she also needs to provide a proof — either the partitions themselves or a cycle of odd length.

Your task is to help Alice to construct the queries, find whether the graph is bipartite.

## Input Format

Read the number of vertices, n.

## Output Format

```text
? k
s1 ... sk
Y s
a1 ... as
N l
c1 ... cl
```

For a query, list k distinct vertices on the line after '? k'. Read the number of edges with both endpoints in this set. For a bipartite graph, list one side after 'Y s'. For a non-bipartite graph, list the distinct vertices of an odd cycle in cyclic order after 'N l'. Either partition side and any valid certificate are accepted. The graph is fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `? 2` |
| | `1 2` |
| `1` | |
| | `Y 1` |
| | `1` |

## Constraints

- 1 ≤ n ≤ 600. Vertices are numbered from 1 to n; the graph is simple, undirected and connected. A query set is nonempty.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

## Problem Description

This is an interactive problem.

In the kingdom of Duloc, Lord Farquaad is developing a network of watchtowers to monitor every corner of his land. He has a map of towers and the roads that connect them, forming an undirected simple graph $G = ( V , E )$ , where each tower is a vertex and each road is an edge between two towers. However, Farquaad is worried that some parts of Duloc might be isolated, making it impossible to reach every tower from any other.

To ensure full connectivity, he tasks you with verifying whether his network is connected. However, there’s a catch: you’re only allowed limited access to information about the graph.

You can query the network to investigate its connectivity. A query allows you to select a subset of towers $S$ and receive a count of the towers not in $S$ that have direct roads connecting them to at least one tower in $S$ . More precisely, $q u e r y ( S ) = | N ( S ) \setminus S |$ , where $S \subseteq V$ and $N ( S ) = \{ x \vert \exists y \in S$ such that $( x , y ) \in E \}$ .

Your goal is to use these queries efficiently to determine if the network is connected.

## Input Format

Read n, the number of towers.

## Output Format

```text
? s
! x
```

Supply a length-n bit string marking S. Read the number of vertices outside S adjacent to at least one member of S. Report 1 for connected or 0 for disconnected. The graph is fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| | `? 100` |
| `2` | |
| | `! 1` |

## Constraints

- The hidden graph is simple and undirected, with at most 200 vertices.
- Time limit: **10 seconds**. Memory limit: **512 MB**.

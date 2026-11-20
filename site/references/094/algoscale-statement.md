## Problem Description

This is an interactive problem.

Ivan and Petr like to play with cacti — special graphs where each edge belongs to at most one simple cycle, and the graph is connected. Multiple edges between pairs of vertices and loops are allowed.

They invent the following game:

* Petr secretly builds a cactus with $n$ vertices and $m$ edges. The edges are labeled from 1 to $m$ .

* Petr only tells Ivan the number $m$ .

* Ivan is then allowed to ask questions of the following form:

– He chooses a subset $S$ of edge labels (see below about limitations on the subset), and asks: “If we only keep the edges whose labels are in $S$ (and all $n$ vertices), is the resulting graph connected?”   
– Petr must answer either “yes” or “no”.

Ivan must determine, for every edge:

1. whether this edge lies on some cycle in the cactus;   
2. if it does, what is the length of that simple cycle.

In this problem, each loop is considered a simple cycle of length 1 and two edges between the same pair of vertices form a simple cycle of length 2.

However, Ivan is still very young and only knows numbers up to 14. So:

* if an edge lies on a simple cycle of length at most 14, he must output that exact length; * if an edge lies on a simple cycle of length greater than 14, he must say that this edge lies on a big cycle.

Also, to avoid having to list a lot of edges each time, Ivan always asks about an edge set obtained from the set used in one of the previous queries, or from the set of all edges, by removing exactly one edge.

Can you design a strategy that allows Ivan to complete this task?

## Input Format

Read m.

## Output Format

```text
? p e
! e1 ... em
```

A query uses the edge set of query p (all edges for p=0) and removes edge e. The answer is 1 if the resulting graph is connected on the original vertex set, and 0 otherwise. Queries are numbered from 1 in chronological order. For each edge report −1 for a bridge, its cycle length when at most 14, or 0 for a longer cycle. The cactus is fixed. After your final answer, read the judge's confirmation: 1 for success or −1 for failure, then exit.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1` | |
| | `? 0 1` |
| `0` | |
| | `! -1` |
| `1` | |

## Constraints

- 1 ≤ m ≤ 10000. Edge labels run from 1 to m, and p must refer to the initial set or a previous query.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

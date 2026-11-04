# 69. Spaceship

## Problem Description

A directed graph has $N$ rooms and doors between some ordered pairs of rooms. A remote has buttons numbered $1$ through $K$.

In each visit to a room, exactly one button must be pressed. After that press, either follow one outgoing door, possibly back to the same room, or stop. Pressing button $x$ makes it unavailable and restores the availability of every button numbered below $x$. Equivalently, using $x$ twice is legal only if a larger button was pressed between those uses.

For each query $(b_s,s,b_t,t)$, start afresh with all buttons available, in room $s$, and press $b_s$. Pressing an unavailable button invalidates a sequence. Count all valid sequences of room visits and button presses that stop in room $t$ immediately after pressing $b_t$. Return the count modulo $10^9+7$.

## Input Format

Read from standard input.

The first line contains $N,K,Q$.

The next $N$ lines are strings of $N$ bits. Bit $j$ in row $i$ is `1` exactly when there is a door from room $i$ to room $j$.

Each of the next $Q$ lines contains **$b_s,s,b_t,t$**, in that order: starting button, starting room, final button, final room.

## Output Format

Write to standard output.

Print one answer per query on its own line, modulo $1000000007$.

## Examples

Input:

```text
6 3 1
010000
001000
000100
000010
000000
000001
1 1 1 1
```

Output:

```text
1
```

## Explanation / Notes

The doors form the chain $1\to 2\to 3\to 4\to 5$ and the self-loop $6\to 6$. The query starts and ends in room 1 with button 1. Stopping immediately is valid. Leaving room 1 makes returning impossible, so there is exactly one valid sequence.

## Constraints

- $1\le N,K,Q\le60$.
- $1\le s,t\le N$ in each query.
- $1\le b_s,b_t\le K$ in each query.
- The adjacency matrix has exactly $N$ rows of $N$ bits each.
- Doors are directed, self-loops are allowed, and no ordered pair of rooms has more than one door.
- Exactly one button is pressed per room visit, including the initial and final visits.
- The same button may recur only after a strictly larger button has occurred in between.
- Answer modulus: $10^9+7$.
- Time limit: **1 second**. Memory limit: **256 MB**.

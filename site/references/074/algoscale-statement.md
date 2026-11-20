## Problem Description

This is an interactive problem.

A tourist attraction is modeled as a cycle of $n$ points numbered clockwise from $1$ to $n$. Two radio transmitters are hidden at distinct points on the cycle. Their positions are fixed before the interaction begins.

You have a receiver that can be used at any point $x$. In response, it reports the sum of the shortest cycle distances from $x$ to the two transmitters, where distance is measured by the number of traversed edges.

Determine the two transmitter positions.

## Input Format

Read from standard input.

First read the odd integer $n$, the number of points in the cycle.

After every valid receiver query, read one integer `dist`: the sum of the shortest distances from the queried point to the two transmitters.

## Output Format

Write to standard output.

To use the receiver at point $x$, print:

```text
? x
```

End the line and flush the output before reading the response.

To report the transmitter positions, print:

```text
! x y
```

The order of $x$ and $y$ does not matter. End the answer with a newline, flush it and exit. There is no acknowledgement. If the judge closes the stream, exit immediately. Reporting an answer does not count as a receiver query.

Typical flushing calls are `fflush(stdout)` or `cout.flush()` in C++, `System.out.flush()` in Java, and `sys.stdout.flush()` in Python.

Fewer receiver queries earn a higher score. Repeated queries each count once.

## Examples

The table shows one possible complete interaction in chronological order. A blank cell means that the other side sends the next message.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| | `? 1` |
| `1` | |
| | `? 2` |
| `1` | |
| | `? 3` |
| `2` | |
| | `! 1 2` |

## Explanation / Notes

In this example, the transmitters are at points $1$ and $2$ on a three-point cycle. The reported values for queries at points $1$, $2$, and $3$ are therefore $1$, $1$, and $2$, respectively.

The transcript is only an example. Judge responses depend on the hidden transmitter positions and are not ordinary batch input that can be read in advance.

## Constraints

- $3\le n\le10^9$.
- $n$ is odd.
- The two transmitters occupy distinct points of the cycle.
- Every receiver query satisfies $1\le x\le n$.
- Reporting an answer does not consume the query budget.
- Time limit: **6 seconds**. Memory limit: **256 MB**.

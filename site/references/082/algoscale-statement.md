## Problem Description

This is an interactive problem.

A tourist attraction is modeled as a cycle of $n$ points numbered clockwise from $1$ to $n$. Two radio transmitters are hidden at distinct points on the cycle. Their positions are fixed before each interaction case begins.

You have a receiver that can be used at any point $x$. In response, it reports the sum of the shortest cycle distances from $x$ to the two transmitters, where distance is measured by the number of traversed edges.

Determine the two transmitter positions using at most $30$ receiver queries per case.

## Input Format

Read from standard input.

First read the integer $T$, the number of test cases.

At the beginning of each test case, read the odd integer $n$, the number of points in the cycle.

After every valid receiver query, read one integer `dist`: the sum of the shortest distances from the queried point to the two transmitters.

## Output Format

Write to standard output.

To use the receiver at point $x$, print:

```text
? x
```

End the line and flush the output before reading the response. Repeating a query counts again. If reading from the interactor fails, terminate immediately.

To report the transmitter positions, print:

```text
! x y
```

The order of $x$ and $y$ does not matter; they must be distinct vertices in $[1,n]$ and equal the hidden pair. End the answer with a newline and flush it. After a correct answer there is no acknowledgement token: read the next case's $n$ directly, or exit normally after the last case. Reporting an answer does not count as a receiver query.

Typical flushing calls are `fflush(stdout)` or `cout.flush()` in C++, `System.out.flush()` in Java, and `sys.stdout.flush()` in Python.

## Examples

The table shows one possible complete interaction in chronological order. A blank cell means that the other side sends the next message.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| `3` | |
| | `? 1` |
| `1` | |
| | `? 2` |
| `1` | |
| | `? 3` |
| `2` | |
| | `! 1 2` |
| `5` | |
| | `? 5` |
| `4` | |
| | `? 1` |
| `3` | |
| | `! 2 3` |

## Explanation / Notes

In the first test case, the transmitters are at points $1$ and $2$ on a three-point cycle. The reported values for queries at points $1$, $2$, and $3$ are therefore $1$, $1$, and $2$, respectively.

The transcript is only an example. Judge responses depend on the hidden transmitter positions and are not ordinary batch input that can be read in advance.

## Constraints

- $1\le T\le10^3$.
- $3\le n\le10^9$ in each test case.
- $n$ is odd.
- The two transmitters occupy distinct points of the cycle.
- Every receiver query satisfies $1\le x\le n$.
- At most $30$ receiver queries may be issued in each test case.
- Reporting an answer does not consume the query budget.
- Time limit: **6 seconds**. Memory limit: **256 MB**.

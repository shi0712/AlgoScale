## Problem Description

This is an interactive problem.

There are $n$ cyclic wheels. Every wheel contains the same $n$ distinct symbols in the same cyclic order, and each wheel currently displays one symbol. Their initial positions are hidden.

You may rotate one wheel by any allowed integer number of positions. After each action, the judge reports how many distinct symbols are currently displayed across the wheels. Make all wheels display the same symbol within the allowed number of actions.

## Input Format

Read from standard input.

First read the integer $n$.

Then read the current distinct-symbol count $k$. This count is provided before the first action and again after each action. If $k>1$, perform another action. If $k=1$, terminate immediately without producing further non-whitespace output. Also terminate if the input stream closes unexpectedly.

The initial configuration is fixed before interaction begins; the judge does not change hidden positions adaptively in response to your strategy.

## Output Format

Write to standard output.

Whenever $k>1$, output one line containing:

```text
i j
```

This rotates wheel $i$ by $j$ positions. Negative $j$ rotates in the opposite direction. Do not print a final answer marker: success is indicated by reading $k=1$, after which the program must exit.

Flush an emitted action before waiting for the next response. At most $1333$ actions may be printed. Every valid action counts once, including $j=0$ or a multiple of $n$; reading the initial $n$ and $k$ does not count. All wheels use the same positive rotation direction, and $n$ steps make one full rotation.

## Examples

The table shows the complete interaction in chronological order. A blank cell means that the other side sends the next message.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| `3` | |
| | `2 -1` |
| `2` | |
| | `3 -1` |
| `2` | |
| | `2 -1` |
| `1` | |

## Explanation / Notes

The judge reveals only the number of different visible symbols, not their identities or individual wheel positions. A rotation can leave this count unchanged even though the configuration changes. The interaction stops as soon as the response is `1`.

## Constraints

- $3\le n\le50$.
- Every wheel contains exactly $n$ distinct symbols, with the same cyclic ordering on every wheel.
- Every response satisfies $1\le k\le n$.
- The first response satisfies $k>1$; the machine is not already solved.
- For every action, $1\le i\le n$ and $-10^9\le j\le10^9$, with integer $i,j$.
- At most $1333$ actions are allowed.
- The hidden initial configuration is fixed before the first action; the judge is non-adaptive.
- After receiving $k=1$, produce no further non-whitespace output.
- Time limit: **0.5 seconds**. Memory limit: **256 MB**.

## Problem Description

This is an interactive problem.

Polycarp is playing a computer game. In this game, an array consisting of zeros and ones is hidden. Polycarp wins if he guesses the position of the $k$-th zero from the left $t$ times.

Polycarp can make requests of the following type:

* ? $l$ $r$ — find out the sum of all elements in positions from $l$ to $r$ ($1 \le l \le r \le n$) inclusive.

To make the game more interesting, each guessed zero turns into one and the game continues on the changed array. More formally, if the position of the $k$-th zero was $x$, then after Polycarp guesses this position, the $x$-th element of the array will be replaced from $0$ to $1$.

Help Polycarp win the game.

## Input Format

Read the array length n and the number of rounds t. All rounds share the same hidden array. Read the next k only after answering the previous round.

## Output Format

```text
? l r
! x
```

Read the sum of the current binary values on [l,r]. Report the position x of the k-th zero from the left. It is turned into a one, and the judge supplies the next k if a round remains. Counts cover all rounds.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `3 2` | |
| `1` | |
| | `? 1 1` |
| `0` | |
| | `! 1` |
| `1` | |
| | `! 3` |

## Constraints

- 1 ≤ n ≤ 200000; 1 ≤ t ≤ min(n,10000); 1 ≤ l ≤ r ≤ n. At least k zeros remain at each request.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

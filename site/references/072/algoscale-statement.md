## Problem Description

This is an interactive problem.

Menji is lost, and everyone misses him. Your task is to find him.

There is a known rooted binary tree $T$. Every edge has length $1$, and Menji is hiding at an unknown vertex $X$.

You may send a signal from a chosen vertex $u$ with a chosen strength $k$. If the distance between $u$ and $X$ is at most $k$, Menji receives the signal and sends one back; otherwise, you receive nothing.

Uniquely determine the hidden vertex using as few signals as possible.

## Input Format

Read from standard input.

- Read the integer $n$, the number of vertices.
- Read $n-1$ integers $f_{a_2},f_{a_3},\ldots,f_{a_n}$, where $f_{a_i}$ is the parent of vertex $i$. The tree is rooted at vertex $1$.

After every valid signal query, read one integer $o$. A response of `1` means that $\operatorname{dist}(u,X)\le k$; a response of `0` means that $\operatorname{dist}(u,X)>k$.

## Output Format

Write to standard output.

To send a signal from vertex $u$ with strength $k$, print:

```text
? u k
```

End the line and flush the output before reading the response.

To report Menji's position, print:

```text
! u
```

Report $u$ only when it is the unique vertex consistent with the complete query history. End the answer with a newline and flush it. Then terminate without additional output. Reporting an answer does not count as sending a signal. If reading from the interactor fails, terminate immediately.

The interactor is **adaptive**: the hidden position may change in response to your queries, provided that it remains consistent with the tree and every previous response.

Typical flushing calls are `fflush(stdout)` or `cout.flush()` in C++, `System.out.flush()` in Java, and `sys.stdout.flush()` in Python.

Fewer signals earn a higher score.

## Examples

One possible complete interaction is shown below.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| `1` | |
| | `? 1 0` |
| `1` | |
| | `! 1` |

## Explanation / Notes

The transcript is only an example. Because the interactor is adaptive, replies may depend on the complete interaction history.

## Constraints

- $2\le n\le3\times10^4$.
- For every $i$, $1\le f_{a_i}<i$.
- The given rooted tree is binary: no vertex has more than two children.
- Every signal query satisfies $1\le u\le n$ and $0\le k\le n$.
- Every query response belongs to $\{0,1\}$.
- Reporting the answer does not consume the signal budget.
- The interactor is adaptive but must remain consistent with all previous responses.
- Time limit: **1 second**. Memory limit: **256 MB**.

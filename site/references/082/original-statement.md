## Problem Description

This is an interactive problem.

There is a hidden permutation $p_1,p_2,\ldots,p_n$ of the integers $1,2,\ldots,n$.

You may query an interval $[l,r]$. The interactor returns the parity of the number of inversions inside that interval:

$$
\left(\sum_{l\le i<j\le r}[p_i>p_j]\right)\bmod 2,
$$

where $[p_i>p_j]$ equals $1$ when $p_i>p_j$, and $0$ otherwise.

Determine the entire hidden permutation.

## Input Format

Read from standard input.

First read the integer $n$. The hidden permutation itself is not supplied.

After every valid query, read one integer: the inversion-count parity of the queried interval.

## Output Format

Write to standard output.

To query an interval, print:

```text
? l r
```

where $l$ and $r$ are valid interval endpoints. End the line and flush the output before reading the response.

To report the hidden permutation, print:

```text
! p_1 p_2 ... p_n
```

The final answer does not count as a query. Terminate the program after printing it.

Typical flushing calls are `fflush(stdout)` or `cout.flush()` in C++, `System.out.flush()` in Java, `flush(output)` in Pascal, and `sys.stdout.flush()` in Python. Failing to flush a query may cause an idleness or time-limit verdict.

## Examples

The table shows one possible complete interaction in chronological order. A blank cell means that the other side sends the next message.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| | `? 1 2` |
| `0` | |
| | `? 1 3` |
| `0` | |
| | `? 2 3` |
| `1` | |
| | `! 2 3 1` |

## Explanation / Notes

The hidden permutation is fixed before the interaction begins. The interactor is **non-adaptive**: it does not change the permutation in response to your queries.

The transcript above is only an example. The judge responses depend on the hidden permutation and are not ordinary batch input that can be read in advance.

## Constraints

- $1\le n\le2000$.
- $(p_1,p_2,\ldots,p_n)$ is a permutation of $(1,2,\ldots,n)$.
- Every query satisfies $1\le l\le r\le n$.
- At most $4\times10^4$ queries may be issued.
- Every query response is either $0$ or $1$.
- The interactor is non-adaptive.
- Time limit: **1 second**. Memory limit: **1024 MB**.

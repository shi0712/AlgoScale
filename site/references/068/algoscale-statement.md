## Problem Description

This is an interactive problem.

A hidden lock has $n$ dials, represented by $a_1,\ldots,a_n$, where every value lies in $[1,n]$. Initially the values are not all equal. The initial state is fixed before interaction begins; the interactor is non-adaptive.

One operation chooses position $p$ and advances that dial by one cyclic step:

$$a_p\leftarrow(a_p\bmod n)+1.$$

After the operation, the judge reports whether some **other** position has the same value as the newly changed $a_p$.

Make all dial values equal using at most $n^2$ operations. The final common value can be any number in $[1,n]$.

## Input Format

Read from standard input.

First read the number of test cases $T$. At the start of each case, read $n$; the hidden array itself is not supplied.

After each valid query, read one integer:

- `0`: no other dial equals the changed dial;
- `1`: at least one other dial equals the changed dial;
- `-1`: an illegal operation or an exceeded operation budget; terminate the entire program immediately.

Also terminate immediately if the input stream closes unexpectedly.

## Output Format

Write to standard output.

To advance dial $p$, print:

```text
? p
```

End the line and flush output before reading the response. When you believe every value is equal, print:

```text
!
```

End the declaration with a newline and flush it. There is no confirmation after a correct `!`: read the next case's $n$ directly, or terminate after the last case. This declaration does not count as an operation. A declaration made before all values are equal results in Wrong Answer. Even when all dials are equal, you must print `!` to finish the case.

Typical flushing calls are `fflush(stdout)` or `cout.flush()` in C++, `System.out.flush()` in Java, `flush(output)` in Pascal, and `sys.stdout.flush()` in Python. Failing to flush queries may cause an idleness or time-limit verdict.

## Examples

Input received:

```text
1
4
1
1
0
1
```

Output emitted:

```text
? 1
? 3
? 4
? 4
!
```

The first two input values are $T=1$ and $n=4$. Each remaining value is the response to the next query; they are not an ordinary batch input that can be read in advance.

## Explanation / Notes

The hidden array in the example starts as $[1,2,1,4]$. Its successive states are:

| Query | New state | Reply |
| --- | --- | ---: |
| `? 1` | $[2,2,1,4]$ | 1 |
| `? 3` | $[2,2,2,4]$ | 1 |
| `? 4` | $[2,2,2,1]$ | 0 |
| `? 4` | $[2,2,2,2]$ | 1 |

The program then declares success with `!`. A reply of `1` merely means that the dial just changed has a matching value elsewhere; it does **not** by itself certify that every dial is equal.

## Constraints

- $1\le T\le100$.
- $3\le n\le100$ in each case.
- Across all test cases, $\sum n\le10^3$.
- Every hidden value always belongs to $[1,n]$.
- The hidden initial values are not all equal.
- Each query position satisfies $1\le p\le n$.
- One query advances exactly one dial by exactly one step, with $n$ wrapping to $1$.
- Per case, at most $n^2$ operations are permitted; success declarations do not consume this budget.
- Query replies are `0` or `1`, with `-1` reserved for illegal interaction or an exceeded budget.
- On `-1`, stop immediately rather than attempting to continue to another case.
- Every query must end with a newline and be flushed.
- Time limit: **3.5 seconds**. Memory limit: **256 MB**.

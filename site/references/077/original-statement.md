## Problem Description

This is an interactive problem.

For a sequence $A=(A_1,A_2,\ldots,A_n)$, let $MSS(A)$ denote its maximum subarray sum. The empty subarray is allowed, so

$$
MSS(A)=\max\left(0,\max_{1\le i\le j\le n}\sum_{k=i}^{j}A_k\right).
$$

For example, $MSS([-2,1,4,-3,5])=7$, while $MSS([-5])=0$ and $MSS([-1,-2])=0$.

Professor Kubic has a hidden integer sequence $a_1,a_2,\ldots,a_n$. You may ask for the maximum subarray sum of any contiguous interval of this sequence.

Your task is to report a sequence $b_1,b_2,\ldots,b_n$ such that

$$
MSS(a_l,a_{l+1},\ldots,a_r)=MSS(b_l,b_{l+1},\ldots,b_r)
$$

for every subarray $[l,r]$.

## Input Format

Read from standard input.

First read the integer $T$, the number of test cases. At the beginning of each test case, read the integer $n$. The hidden sequence itself is not supplied.

After every valid query, read one integer: the maximum subarray sum of the queried interval.

## Output Format

Write to standard output.

To query an interval, print:

```text
? l r
```

where $l$ and $r$ are valid interval endpoints. End the line and flush the output before reading the response.

To answer a test case, print:

```text
! b_1 b_2 ... b_n
```

The final answer does not count as a query. After printing it, continue with the next test case or terminate the program if all test cases are complete.

Typical flushing calls are `fflush(stdout)` or `cout.flush()` in C++, `System.out.flush()` in Java, `flush(output)` in Pascal, and `sys.stdout.flush()` in Python. Failing to flush a query may cause an idleness or time-limit verdict.

## Examples

The table shows one possible complete interaction in chronological order. A blank cell means that the other side sends the next message.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| `3` | |
| | `? 1 1` |
| `1` | |
| | `? 2 2` |
| `0` | |
| | `? 3 3` |
| `1` | |
| | `! 1 -1 1` |
| `5` | |
| | `? 1 3` |
| `4` | |
| | `? 3 5` |
| `5` | |
| | `! 2 -1 3 -4 5` |

## Explanation / Notes

The hidden sequence for every test case is fixed before the interaction begins. The interactor is **non-adaptive**: it does not change the hidden values in response to your queries.

The transcript above is only an example. The judge responses depend on the hidden sequences and are not ordinary batch input that can be read in advance.

## Constraints

- $1\le T\le10^4$.
- $1\le n\le2000$ in each test case.
- Across all test cases, $\sum n\le10^4$.
- $-10^9\le a_i\le10^9$.
- Every query satisfies $1\le l\le r\le n$.
- At most $2n$ queries may be issued in each test case.
- Every reported value satisfies $-10^{15}\le b_i\le10^{15}$.
- The interactor is non-adaptive.
- Time limit: **1 second**. Memory limit: **1024 MB**.

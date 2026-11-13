## Problem Description

For an integer sequence $a_1,\ldots,a_n$, an operation selects a contiguous subarray of odd length strictly greater than 1 and replaces **every element** of that subarray by its median.

Apply any number of operations until all elements are equal. Define the value of the original sequence to be the largest final common value that can be obtained.

Given intervals $[l_i,r_i]$, consider every integer sequence satisfying $l_i\le a_i\le r_i$ for all $i$. Find the sum of their values, modulo $10^9+7$.

## Input Format

Read from standard input.

The first line contains $T$, the test-case count. For each case, read:

1. $n$;
2. $l_1,l_2,\ldots,l_n$;
3. $r_1,r_2,\ldots,r_n$.

## Output Format

Write to standard output.

For each case, print one integer: the required sum modulo $1\,000\,000\,007$.

## Examples

Input:

```text
1
3
1 1 1
1 1 1
```

Output:

```text
1
```

## Explanation / Notes

An odd-length list has a unique middle element after sorting; that element is its median. The permitted operation lengths are 3, 5, 7, and so on.

The only permitted input sequence in the example is $(1,1,1)$. Its value is 1, so the required sum is 1.

## Constraints

- $1\le T\le10$.
- $3\le n\le150$ for each case; $n$ itself need not be odd.
- $1\le l_i\le r_i\le10^9$ for every position.
- All sequence entries and interval endpoints are integers.
- A replacement interval must be contiguous, have odd length, and contain at least 3 elements.
- No bound on the number of allowed operations is specified.
- The required modulus is $10^9+7$.

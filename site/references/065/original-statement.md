# 65. Magical String

## Problem Description

A magical string initially contains lowercase English letters and has no run of three or more equal consecutive letters.

In one operation, change one letter into another letter. The change is permitted only if it immediately creates a run of at least three equal consecutive letters. The whole resulting maximal run is then replaced by one asterisk, `*`.

An asterisk is inert: it cannot be edited, cannot join a run of letters, and is never removed, even when several asterisks become adjacent.

Given a magical string $S$ and an integer $K$, perform at most $K$ operations to maximize the number of original characters absorbed into asterisks. Replacing a run of length $r$ contributes $r$ to this count, not one.

## Input Format

Read from standard input.

The first line contains the number of test cases $T$. Each of the next $T$ lines contains the string $S$ and integer $K$, separated by a space.

## Output Format

Write to standard output.

For each case, print the maximum number of characters that can be transformed into asterisks using at most $K$ edits.

## Examples

Input:

```text
1
cabacbc 2
```

Output:

```text
6
```

## Explanation / Notes

Edit the third character of `cabacbc` from `b` to `a`, obtaining `caaacbc`, then replace `aaa` to get `c*cbc`. Editing the remaining `b` to `c` removes another three letters, giving `c**` and a total of six absorbed characters.

## Constraints

- $1\le T\le50$.
- $1\le |S|\le1000$ for each test case.
- $1\le K\le1000$.
- The initial string uses only the 26 lowercase English letters `a`–`z`.
- Initially, every run of equal consecutive letters has length at most two.
- Each operation must immediately create a run of at least three equal letters; arbitrary preparatory edits are not permitted.
- Each created run is replaced by exactly one permanent asterisk.
- Fewer than $K$ operations may be used, including zero.
- Time limit: **1 second**. Memory limit: **1024 MB**.

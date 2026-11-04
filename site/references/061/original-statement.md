# 61. Athlete Welcome Ceremony

## Problem Description

A row of $n$ positions must be filled with letters `a`, `b`, and `c`, with different letters at every pair of adjacent positions. A given string $s$ fixes some letters; each `?` is a position that must be assigned a letter.

For each of $Q$ independent plans $(x,y,z)$, count assignments using at most $x$ new `a` letters, $y$ new `b` letters, and $z$ new `c` letters. These limits apply only to the original `?` positions, not to letters already fixed in $s$.

Assignments differ if any position receives a different letter. Copies of the same letter are indistinguishable. Return every count modulo $10^9+7$.

## Input Format

Read from standard input.

The first line contains $n$ and $Q$. The second line contains the length-$n$ string $s$.

Each of the next $Q$ lines contains $x,y,z$ for one plan.

## Output Format

Write to standard output.

Print $Q$ lines, each containing the number of valid assignments for the corresponding plan, modulo $1000000007$.

## Examples

Input:

```text
6 1
a?b??c
2 2 2
```

Output:

```text
3
```

## Explanation / Notes

The three valid strings are `acbabc`, `acbcac`, and `acbcbc`.

Plans give upper bounds, so unused new letters are allowed. The original fixed letters cannot be changed. Every adjacent pair must differ after all question marks have been replaced.

## Constraints

- $1\le n\le300$.
- $1\le Q\le10^5$.
- $|s|=n$ and every character is one of `a`, `b`, `c`, `?`.
- For every plan, $0\le x,y,z\le300$; these values are integers.
- $x+y+z$ is at least the number of `?` characters in $s$.
- Plans are independent and all refer to the same original string.
- There is no promise that the fixed letters already permit a valid final arrangement; a plan may have answer zero.
- Arithmetic modulus: $10^9+7$.
- Time limit: **2.5 seconds**. Memory limit: **1024 MB**.

There is one instance per input.

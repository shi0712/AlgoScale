## Problem Description

Start with an array $A=(a_1,\ldots,a_n)$ whose entries are in $[1,m]$. The following operation may be repeated any number of times, including zero:

1. Choose an index $s$ whose entry is a **strict prefix maximum**: $a_s>a_j$ for every $j<s$. The first entry always qualifies.
2. If $a_s\ne1$, append $a_s-1$ to the end of the array.
3. Delete the first $s$ entries.

Count the distinct **nonempty** arrays obtainable this way, including the original array. Two arrays are distinct if their lengths differ or some corresponding entries differ. Return the count modulo $998244353$.

## Input Format

Read from standard input.

Read an unused integer $c$ followed by the number of cases $T$. Ignore $c$. Each case contains a line with $n,m$, then a line with $a_1,\ldots,a_n$.

## Output Format

Write to standard output.

For every case, print the number of reachable nonempty arrays modulo $998244353$.

## Examples

Input:

```text
0 1
3 2
1 2 1
```

Output:

```text
4
```

## Explanation / Notes

The reachable nonempty arrays are $[1,2,1]$, $[2,1]$, $[1,1]$, and $[1]$. The original array is included because zero operations are allowed.

## Constraints

- $1\le T\le10$.
- $1\le n\le10^6$ and $1\le m\le2500$.
- The sum of $n$ over all cases in one input file is at most $10^6$.
- $1\le a_i\le m$, with integer entries.
- $0\le c\le25$. It is only an input-file identifier and does not imply any additional properties.
- Only nonempty resulting arrays are counted.

- There are no subtasks.
- Time limit: 10 seconds.
- Memory limit: 256 MB.

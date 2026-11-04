# 63. Bridging the Gap

## Problem Description

Initially, $n$ walkers and one torch are on the same bank of a river. A bridge can carry at most $c$ walkers at once. Every crossing, in either direction, requires the torch, so someone must carry it back whenever another outward crossing still needs it.

Walker $i$ takes $t_i$ time to cross. A group crosses together at its slowest member's speed. Find the minimum total time until all walkers have reached the other bank.

## Input Format

Read from standard input.

The first line contains $n$ and $c$. The next line contains $t_1,\ldots,t_n$.

## Output Format

Write to standard output.

Print the minimum total crossing time.

## Examples

Input:

```text
4 2
1 2 10 5
```

Output:

```text
17
```

## Explanation / Notes

The walkers with times 1 and 2 cross in 2 time units; the time-1 walker returns; the time-5 and time-10 walkers cross; the time-2 walker returns; then the two fastest cross again. The total is $2+1+10+2+2=17$.

## Constraints

- $2\le n\le10^4$.
- $2\le c\le10^4$.
- $1\le t_i\le10^9$ for all $i$.
- All input values are integers.
- The capacity may exceed $n$.
- The crossing times need not be sorted or distinct.
- There is exactly one torch, and every crossing must include it.
- There is one test instance per input.
- Time limit: **4 seconds**.

## Problem Description

Initially, $n$ walkers and one torch are on the same bank of a river. A bridge can carry at most $c$ walkers at once. Every crossing, in either direction, requires the torch, so someone must carry it back whenever another outward crossing still needs it.

Walker $i$ takes $t_i$ time to cross. Each crossing takes between one and $c$ walkers currently on the same bank as the torch to the other bank, taking the torch with them. A group departs and arrives together, and its crossing time equals the largest $t_i$ among its members. Find the minimum total time until all walkers have reached the other bank.

## Input Format

Read from standard input.

The first line contains $n$ and $c$. The next line contains $t_1,\ldots,t_n$.

## Output Format

Write to standard output.

Print the minimum total crossing time.

## Examples

### Example 1

Input:

```text
4 2
1 2 10 5
```

Output:

```text
17
```

### Example 2

Input:

```text
4 4
1 2 10 5
```

Output:

```text
10
```

## Explanation / Notes

The walkers with times 1 and 2 cross in 2 time units; the time-1 walker returns; the time-5 and time-10 walkers cross; the time-2 walker returns; then the two fastest cross again. The total is $2+1+10+2+2=17$.

In the second example, all four walkers cross together in 10 time units.

## Constraints

- $2\le n\le50000$.
- $2\le c\le n$.
- $1\le t_i\le10^9$ for all $i$.
- All input values are integers.
- The crossing times need not be sorted or distinct.
- There is exactly one torch, and every crossing must include it.
- There is one test instance per input.
- Time limit: **2 seconds**. Memory limit: **256 MB**.

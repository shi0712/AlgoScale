# 5. Robots

## Problem Description

Choose integer heights $h_i\in[A_i,B_i]$ for $n$ pillars arranged in order. For each possible starting index $s$, two robots start on pillar $s$ and move only between adjacent pillars.

- Robot P moves left while the next pillar's height is **at most** $h_s$. It stops at $l$ such that all heights from $l$ through $s$ are at most $h_s$, and either $l=1$ or $h_{l-1}>h_s$.
- Robot Q moves right while the next pillar's height is **strictly less than** $h_s$. It stops at $r$ such that all heights from $s+1$ through $r$ are less than $h_s$, and either $r=n$ or $h_{r+1}\ge h_s$.

The numbers of moves are $s-l$ and $r-s$, respectively. Count the height assignments for which

$$\left|(s-l)-(r-s)\right|\le2$$

holds for **every** starting index $s$. Two assignments differ when any pillar's height differs. Return the count modulo $10^9+7$.

## Input Format

Read from standard input.

Read $n$, followed by $n$ lines. Line $i$ contains the inclusive lower and upper height bounds $A_i,B_i$.

## Output Format

Write to standard output.

Print the number of valid height assignments modulo $10^9+7$.

## Examples

Input:

```text
5
3 3
2 2
3 4
2 2
3 3
```

Output:

```text
1
```

## Explanation / Notes

The example permits two assignments. For $[3,2,3,2,3]$, starting at pillar 5 gives P four moves and Q zero, so that assignment is invalid. The assignment $[3,2,4,2,3]$ is valid:

| Start | P's stopping pillar | P's moves | Q's stopping pillar | Q's moves |
| ---: | ---: | ---: | ---: | ---: |
| 1 | 1 | 0 | 2 | 1 |
| 2 | 2 | 0 | 2 | 0 |
| 3 | 1 | 2 | 5 | 2 |
| 4 | 4 | 0 | 4 | 0 |
| 5 | 4 | 1 | 5 | 0 |

The starting pillar itself does not count as a move. The asymmetric treatment of equal heights is part of the problem.

## Constraints

- One test case is given; there is no leading case count.
- $1\le n\le300$.
- $1\le A_i\le B_i\le10^9$ for every pillar.
- Every chosen height $h_i$ is an integer in the inclusive interval $[A_i,B_i]$.
- The balance condition must hold for all $n$ starting positions.

- Time limit: 3 seconds.
- Memory limit: 512000 KiB (500 MiB).

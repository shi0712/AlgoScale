## Problem Description

This is an interactive problem.

There are $N$ small towns in Kazakhstan, numbered from $0$ through $N - 1$. There is also an unknown number of large cities. The small towns and large cities of Kazakhstan are jointly called settlements.

All the settlements of Kazakhstan are connected by a single network of bidirectional highways. Each highway connects two distinct settlements, and each pair of settlements is directly connected by at most one highway. For each pair of settlements $a$ and $b$ there is a unique way in which one can go from $a$ to $b$ using the highways, as long as no highway is used more than once.

It is known that each small town is directly connected to a single other settlement, and each large city is directly connected to three or more settlements.

Every highway has a positive integer length. The distance between two settlements is the minimum sum of the lengths of the highways one needs to travel in order to get from one settlement to the other.

For each large city $C$ we can measure the distance $r(C)$ to the small town that is the farthest away from that city. A large city $C$ is a hub if the distance $r(C)$ is the smallest among all large cities. The distance between a hub and a small town that is farthest away from the hub will be denoted by $R$. Thus, $R$ is the smallest of all values $r(C)$.

Removing a hub divides the network into multiple connected pieces. A hub is balanced if each of those pieces contains at most $\lfloor \frac{N}{2} \rfloor$ small towns. (We stress that we do not count the large cities.) Note that $\lfloor x \rfloor$ denotes the largest integer which is not greater than $x$.

Determine the distance $R$ and whether there is a balanced hub.

## Input Format

First read $N$. The network and all highway lengths are fixed before the interaction.

## Output Format

To measure the distance between towns, print:

```text
? i j
```

Read their distance. Equal town labels or integer labels outside the town range give distance zero. Such a request still counts. To finish, print:

```text
! value
```

Report $R$ if at least one hub is balanced and $-R$ otherwise.

End every command with a newline and flush standard output. Each query counts once. Fewer queries earn a higher score. The final answer is free. Exit immediately after printing it, or if the input stream ends unexpectedly.

## Examples

| Judge sends | Program sends |
| --- | --- |
| `6` |  |
|  | `? 0 1` |
| `2` |  |
|  | `? 0 0` |
| `0` |  |
|  | `! 1` |

## Explanation / Notes

In this possible interaction, all six small towns are joined by unit-length roads to the same large city. Removing it separates the six towns.

## Constraints

- $6 \le N \le 110$.
- The distance between any two distinct small towns is between $1$ and $1\,000\,000$, inclusive.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

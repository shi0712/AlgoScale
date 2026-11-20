## Problem Description

This is an interactive problem.

You are given an integer $n$ and an $n\times n$ grid of numbers $G$. The grid of numbers contains each number from $1$ to $n^2$ exactly once.

Let's define a snake of length $l$ as a deque $[(x_1,y_1), (x_2,y_2), \ldots, (x_l,y_l)]$, where $(x_1,y_1)$ is the head of the snake, and $(x_l,y_l)$ is the tail. At second $1$, $x_1=x_2=\ldots = x_l=1$ and $y_i=i$ for all $1 \leq i \leq l$. In other words, the snake is entirely located in the first row, with the head at $(1,1)$ and the rest of the snake to the right of the head.

Each subsequent second, the snake moves down or right in the grid. Formally, the tail $(x_l,y_l)$ is removed, and either $(x_1+1,y_1)$ or $(x_1,y_1+1)$ is added as the new head. The first move of the snake is always downwards. It can be shown that the snake will never intersect itself under these restrictions. The snake will move exactly $2n-2$ times, never moving outside the grid. At second $2n-1$, the head reaches $(n,n)$, and the movement stops. It can be shown that the snake moves exactly $n-1$ times to the right and exactly $n-1$ times downwards.

There are $n$ hidden snakes, with the $i$-th snake having length $i$ for $1 \leq i \leq n$, each moving independently according to the rule above. You do not know how the snakes move. Define $f(l,T)$ as the maximum number that the snake with length $l$ covers at second $T$.

Now, you are also given an integer $m$. Your task is to find the $m$ smallest values of $f(l,T)$, using queries asking for the value of $f(l,T)$ for some $1 \leq l \leq n$ and $1 \leq T \leq 2n-1$.

## Input Format

Read n and m, followed by n rows of the public grid.

## Output Format

```text
? l T
! s1 ... sm
```

Read f(l,T) for snake length l at time T. Report the m smallest values in nondecreasing order, with multiplicities, and exit. The trajectories must satisfy the movement rules above.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2 1` | |
| `1 2` | |
| `3 4` | |
| | `? 1 1` |
| `1` | |

## Constraints

- 2 ≤ n ≤ 500; 1 ≤ m ≤ n(2n−1); 1 ≤ l ≤ n; 1 ≤ T ≤ 2n−1. The grid is a permutation of 1 through n². m ≤ 50000.
- Time limit: **10 seconds**. Memory limit: **512 MB**.

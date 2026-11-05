## Problem Description

There are $n$ positions numbered $1$ through $n$. Position $i$ has an interval $[a_i,b_i]$, and from $i$ you may move to any position in that interval.

Choose a starting position $u$, make any finite number of allowed moves, and finish at $u$. Taking zero moves is allowed. Let $S$ be the set of positions visited at least once, including the starting position.

Count the distinct sets $S$ that can be obtained in this way, modulo $998244353$. Different walks or starting positions that produce the same set contribute only once.

## Input Format

Read from standard input.

The first line contains $n$. The next $n$ lines contain $a_i,b_i$ for positions $i=1,\ldots,n$.

## Output Format

Write to standard output.

Print the number of distinct achievable visited-position sets modulo $998244353$.

## Examples

Input:

```text
5
1 2
2 5
3 4
2 5
4 5
```

Output:

```text
12
```

## Explanation / Notes

The required walk is closed: reaching some other position without returning to the chosen start is insufficient. A zero-move walk gives the singleton set containing its start. No empty visited set is possible.

The interval condition describes directed moves; a move from $i$ to $j$ does not by itself guarantee that $j$ can move directly back to $i$.

## Constraints

- $1\le n\le50$.
- For every $i$, $1\le a_i\le i\le b_i\le n$.
- All input values are integers.
- Interval endpoints are inclusive.
- Every position belongs to its own permitted destination interval.
- Walk length has no stated finite upper bound; the object being counted is the visited set, not the walk.
- Modulus: $998244353$.
- There is one instance per input.
- Time limit: **2 seconds**. Memory limit: **1024 MB**.

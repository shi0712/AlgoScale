## Problem Description

There are $n$ players at coordinates $1,2,\ldots,n$ on a line. Each faces left or right and initially has normal size.

Choose a permutation $p$ of the players as their action order. When player $x$ gets a turn:

- If already shrunk, the player does nothing.
- Otherwise, the player fires in their facing direction. The first player in that direction who has not been shrunk is hit and immediately shrinks. If there is no such player, the shot hits nobody.

For every $k=1,\ldots,n$, count the permutations that leave exactly $k$ players unshrunk after all turns. Output these counts modulo $998244353$.

## Input Format

Read from standard input.

Read $n$, followed by a string $s$ of length $n$. Character `L` means the corresponding player faces toward decreasing coordinates; `R` means increasing coordinates.

## Output Format

Write to standard output.

Print $n$ integers on one line. The $k$-th integer is the number of action permutations leaving exactly $k$ unshrunk players, modulo $998244353$.

## Examples

### Example

Input:

```text
2
RL
```

Output:

```text
2 0
```

## Explanation / Notes

Shrunk players are skipped when a shot searches for its target. A player may be hit before their own scheduled turn, in which case that turn is skipped. Each original player appears once in the action permutation.

## Constraints

- One test case.
- $2\le n\le500$.
- $|s|=n$ and every character belongs to $\{\texttt{L},\texttt{R}\}$.
- All $n!$ possible action permutations are counted.
- Modulus: $998244353$.
- Time limit: 1 second; memory limit: 512 MB.

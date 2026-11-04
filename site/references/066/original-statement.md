# 66. Angle Beats

## Problem Description

An $n\times m$ board contains `*`, `+`, and `.` cells. Place as many pairwise disjoint trominoes as possible. Each tromino consists of a center cell and two distinct cells sharing edges with that center.

- An L-shaped tromino uses two perpendicular neighbors; its center must contain `*` or `+`.
- An I-shaped tromino uses two opposite neighbors; its center must contain `+`.
- Both non-center cells must contain `.`.

No cell may belong to more than one tromino. Output a board showing a placement with the maximum possible number of trominoes.

## Input Format

Read from standard input.

The first line contains $n,m$. Each of the next $n$ lines contains $m$ board characters.

## Output Format

Write to standard output.

Print $n$ lines of $m$ characters. Replace the three cells of each chosen tromino by the same lowercase English letter. Edge-adjacent cells belonging to different trominoes must receive different letters. A cell not covered by a tromino must retain its original character.

Any optimal placement and valid lettering are accepted. Do not print the number of trominoes separately.

## Examples

Input:

```text
2 2
*.
..
```

Output:

```text
aa
a.
```

## Explanation / Notes

The letters identify trominoes in the output; they are not part of the original board. Different trominoes may reuse a letter when no cells from the two trominoes share an edge. A `*` center permits only an L, while a `+` center permits either shape.

## Constraints

- $2\le n,m\le100$.
- Every input row has exactly $m$ characters, each one `*`, `+`, or `.`.
- Each tromino occupies exactly three distinct grid cells.
- The two non-center cells must have been `.` in the input.
- Trominoes may not overlap; edge contact between different trominoes is allowed subject to different output letters.
- Output labels use only lowercase English letters `a`–`z`.
- There is one board per input.
- Time limit: **2 seconds**. Memory limit: **256 MiB**.

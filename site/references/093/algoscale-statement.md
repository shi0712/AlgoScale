## Problem Description

This is an interactive problem.

You are standing at the very center of a field, which is divided into a grid of cells running north-south and east-west. A great treasure is hidden somewhere within one of these cells.

John Belzoni, a descendant of the famous treasure hunter Giovanni Battista Belzoni, actually discovered that treasure. Unfortunately enough, he died of heatstroke before successfully digging it out; he seems to have spent too long time wandering around the field.

John started his exploration from the central cell of the field where you stand now. All of his footprints leading to the treasure are left on the field, but you cannot identify the footprint on a cell until you reach there. The footprint on a cell indicates which of the four adjacent grid cells he proceeded to. It is known that John did not visit the same grid cell twice or more. You see a footprint on the central cell indicating that John’s first step was northward.

There is exactly one treasure cell in the field, and you will recognize it only when you are on it.

Your task is to find the treasure in a limited number of steps. In a single step, you decide to take one of the four directions north, west, south, or east, and proceed to the adjacent cell in that direction. When you move on to the cell, you may find either the treasure, John’s footprint, or nothing. You do not have to follow John’s footprints. Unlike John’s route, you may visit the same cell more than once. John’s footprints will remain the same regardless of your exploration.

## Input Format

Read n. The field has (2n+1) rows and (2n+1) columns. Start in its central cell.

## Output Format

```text
^
<
v
>
```

Print one direction per move. The reply is G at the treasure, one of ^, <, v, > for a footprint, or a dot for an unmarked cell. Stay inside the field. Upon G, exit; the move that reaches the treasure counts. The field is fixed.

Fewer moves earn a higher score. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1` | |
| | `^` |
| `G` | |

## Constraints

- 1 ≤ n ≤ 2000. The original path begins at the center, first goes north and never visits a cell twice.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

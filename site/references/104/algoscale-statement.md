## Problem Description

This is an interactive problem.

Because work in the mine is hard and time-consuming, the dwarves decided to seek help in automating some of their tasks. Their current task is to classify $N$ minerals (identified by numbers from 1 to $N$ ) into different classes. To assist with this, Dwarf the Worker has installed a new conveyor belt. The dwarves can add samples of minerals to the beginning of the belt and remove them from its end (i.e., remove the minerals that were added earliest).

Unfortunately, this conveyor belt has no knowledge base, so it cannot identify the class of any mineral on it. However, it has another useful feature: it displays how many distinct classes of minerals are currently on the belt. It turns out this is sufficient for the mine, as the dwarves do not need to know the class of each mineral: they just need to segregate the minerals into piles of the same class.

Can you help the dwarves solve this task using the conveyor belt?

## Input Format

Read N.

## Output Format

```text
+ id
-
! k
c id1 ... idc
```

Enqueue the specified mineral or dequeue the oldest mineral. Removing from an empty belt leaves it empty. After each operation read the number of distinct classes on the belt. To report the partition, print ! k followed by k lines, one per class. Every identifier must occur exactly once across those lines. The class labels are fixed; classes and their members may be listed in any order.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `+ 1` |
| `1` | |
| | `+ 2` |
| `1` | |
| | `! 1` |
| | `2 1 2` |

## Constraints

- 1 ≤ N ≤ 500. Mineral identifiers are numbered from 1 to N.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

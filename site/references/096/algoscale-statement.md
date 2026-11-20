## Problem Description

This is an interactive problem.

There are $N$ coins numbered $1, 2, \dots, N$.  
Exactly $M$ of these coins are counterfeit.

An appraiser can, in one appraisal, determine whether two coins are of the same type or different types. Specifically:

* If the two coins are both genuine or both counterfeit, they are judged to be of the same type.
* Otherwise, they are judged to be of different types.

Identify all the counterfeit coins.

## Input Format

Read N and M, the total number of coins and the number of counterfeit coins.

## Output Format

```text
? x y
! i1 ... iM
```

Choose two distinct coin labels. Read 0 when their kinds agree and 1 when they differ. Report the distinct counterfeit labels in any order. Every response must remain compatible with one assignment of kinds to all coins.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1000 10` | |
| | `? 1 2` |
| `0` | |

## Constraints

- N = 1000 and M = 10. Coin labels range from 1 to N.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.

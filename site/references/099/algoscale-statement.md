## Problem Description

This is an interactive problem.

Petya participates in such contest that the participants only learn results of judging their submissions after the end of the contest.
There are  $n$  problems numbered from 1 to  $n$  in the contest. For each problem he has made a submission. After the contest he was very eager to learn his points and his place. But the results would only be published at the closing ceremony, which is a few hours away.
Petya decided to find out his result before that: to get his points for each problem from the jury, he would ask weird questions. Let Petya's points be  $c_{1}, c_{2}, \ldots, c_{n}$  for problems  $1, 2, \ldots, n$ , respectively. Petya doesn't know these numbers and wants to find them out.
Petya chooses three distinct numbers of the problems and asks the jury to tell him the sum of the minimum and the maximum points he received for these three problems. Fortunately, the jury answers such queries. Formally, if he queries about  $i$ ,  $j$ ,  $k$ , he gets  $\min(c_i, c_j, c_k) + \max(c_i, c_j, c_k)$ .
Petya still doesn't know how to find out all his points. Help him find out  $c_{1}, c_{2}, \ldots, c_{n}$.

## Input Format

Read n.

## Output Format

```text
? i j k
! c1 ... cn
```

Query three distinct positions and read the sum of the minimum and maximum of their three scores. Report all hidden scores in problem order. The scores are fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `5` | |
| | `? 1 2 3` |
| `4` | |

## Constraints

- 5 ≤ n ≤ 1000; 0 ≤ c[i] ≤ 10⁹. All labels are 1-based.
- Time limit: **10 seconds**. Memory limit: **512 MB**.

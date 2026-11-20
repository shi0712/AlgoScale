## Problem Description

This is an interactive problem.

Väinö has been selected as the new chairman for the Iconic Character Poetry Contest (ICPC), a televised battle of wits and verse boasting the nation’s best and brightest. Unfortunately, ICPC’s popularity has exploded in recent years, and Väinö is beset with hoards of new applicants wishing to test their mettle.

The program has a limited number of slots to accept applicants, and Väinö is tasked with ensuring that only the best applicants get said slots. He has decided to host a qualifying tournament to decide who advances. In the tournament, he will invite two applicants at a time to compete with each other to assess which has the greater skill at poetry, or runous. He is certain that no two applicants will have the same skill level. The skill levels are transitive, so if $a$ defeats $b$ and $b$ defeats $c$ , then $a$ would also defeat $c$ .

However, his time is limited and he wants to limit how many matches he must hold in order to determine the top applicants that will advance. Can you help him choose which matches to hold?

## Input Format

Read N and K.

## Output Format

```text
? a b
! a1 ... aK
```

Compare two distinct applicants and read the label of the winner. Report the K strongest labels in any order. The comparisons must establish that this set is uniquely determined under every strict ranking consistent with the results. Skills are distinct and fixed.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2 1` | |
| | `? 1 2` |
| `2` | |
| | `! 2` |

## Constraints

- 1 ≤ N ≤ 15000; 1 ≤ K ≤ min(N,30). Labels range from 1 to N.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

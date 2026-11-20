## Problem Description

This is an interactive problem.

Mr. 1048576 is one of those faculty who hates wasting his time in taking class attendance. Instead of taking attendance the old-fashioned way, he decided to try out something new today.

There are $n$ students in his class, having roll numbers $1$ to $n$. He knows that exactly $1$ student is absent today. In order to determine who is absent, he can ask some queries to the class. In each query, he can provide two integers $l$ and $r$ ($1\leq l\leq r\leq n$) and all students whose roll numbers are between $l$ and $r$ (inclusive) will raise their hands. He then counts them to determine if the roll number of the absent student lies between these values.

Things seemed fine until his teaching assistant noticed something — the students are dishonest! Some students whose roll numbers lie in the given range may not raise their hands, while some other students whose roll number does not lie in the given range may raise their hands. But the students don't want to raise much suspicion. So, only the following $4$ cases are possible for a particular query $(l,r)$ —

1. True Positive: $r-l+1$ students are present and $r-l+1$ students raised their hands.
2. True Negative: $r-l$ students are present and $r-l$ students raised their hands.
3. False Positive: $r-l$ students are present but $r-l+1$ students raised their hands.
4. False Negative: $r-l+1$ students are present but $r-l$ students raised their hands.

In the first two cases, the students are said to be answering honestly, while in the last two cases, the students are said to be answering dishonestly. The students can mutually decide upon their strategy, not known to Mr. 1048576. Also, the students do not want to raise any suspicion and at the same time, want to create a lot of confusion. So, their strategy always meets the following two conditions —

1. The students will never answer honestly $3$ times in a row.
2. The students will never answer dishonestly $3$ times in a row.

Mr. 1048576 is frustrated by this act of students. So, he is willing to mark at most $2$ students as absent (though he knows that only one is). The attendance is said to be successful if the student who is actually absent is among those two. Help him complete a successful attendance.

## Input Format

Read n.

## Output Format

```text
? l r
! a
#
```

Read the number of raised hands in [l,r]. A nomination ! a receives 1 if correct and 0 otherwise. You may nominate at most two students. Finish with #. Only interval questions count. The judge may choose replies consistent with the rules above.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| | `? 1 1` |
| `0` | |
| | `! 1` |
| `1` | |
| | `#` |

## Constraints

- 3 ≤ n ≤ 100000; 1 ≤ l ≤ r ≤ n and 1 ≤ a ≤ n.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

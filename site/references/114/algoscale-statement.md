## Problem Description

This is an interactive problem.

Gustaw is the chief bank manager in a huge bank. He has unlimited access to the database system of the bank, in a few clicks he can move any amount of money from the bank's reserves to his own private account. However, the bank uses some fancy AI fraud detection system that makes stealing more difficult.

Gustaw knows that the anti-fraud system just detects any operation that exceeds some fixed limit $M$ euros and these operations are checked manually by a number of clerks. Thus, any fraud operation exceeding this limit is detected, while any smaller operation gets unnoticed.

Gustaw doesn't know the limit $M$ and wants to find it out. In one operation, he can choose some integer $X$ and try to move $X$ euros from the bank's reserves to his own account. Then, the following happens.

* If $X \le M$, the operation is unnoticed and Gustaw's account balance raises by $X$ euros.
* Otherwise, if $X > M$, the fraud is detected and cancelled. Moreover, Gustaw has to pay $X$ euros from his own account as a fine. If he has less than $X$ euros on the account, he is fired and taken to the police.

Initially Gustaw has $1$ euro on his account. Help him find the exact value of $M$ without getting him fired.

## Input Format

There is no initial input. Send your first operation or your final answer.

## Output Format

```text
? X
! M
```

Read Lucky! if X ≤ M, Fraudster! after a failed transfer whose fine can be paid, or Fired! if the fine cannot be paid. Exit immediately on Fired!. Exit after reporting the answer.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| | `? 1` |
| `Fraudster!` | |
| | `! 0` |

## Constraints

- 0 ≤ M ≤ 10¹⁴; 1 ≤ X ≤ 10¹⁴. Start with one euro.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

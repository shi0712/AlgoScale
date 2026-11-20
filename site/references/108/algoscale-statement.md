## Problem Description

This is an interactive problem.

Kira has a hidden positive integer $n$, and Hayato needs to guess it.

Initially, Kira gives Hayato the value $\mathrm{cnt}$ — the number of unit bits in the binary notation of $n$. To guess $n$, Hayato can only do operations of one kind: choose an integer $x$ and subtract it from $n$. Note that after each operation, the number $n$ changes. Kira doesn't like bad requests, so if Hayato tries to subtract a number $x$ greater than $n$, he will lose to Kira. After each operation, Kira gives Hayato the updated value $\mathrm{cnt}$ — the number of unit bits in the binary notation of the updated value of $n$.

Kira doesn't have much patience, so Hayato must guess the original value of $n$.

Since Hayato is in elementary school, he asks for your help. Write a program that guesses the number $n$. Kira is an honest person, so he chooses the initial number $n$ before all operations and does not change it afterward.

## Input Format

Read the number of set bits in the secret initial integer.

## Output Format

```text
- x
! n
```

Subtract x from the current value and read the popcount of the new value. A subtraction may not exceed the current value. Report the initial value. Exit on −1.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1` | |
| | `- 1` |
| `0` | |
| | `! 1` |

## Constraints

- 1 ≤ n ≤ 10⁹; 1 ≤ x ≤ 10⁹.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

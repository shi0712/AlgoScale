# 58. Iroha and the Kingdom of Construction

## Problem Description

A deck contains $2n$ cards. Its ranks are $1,\ldots,n$, each occurring exactly twice. Process the cards in their given order, placing each card on top of one of at most **260 stacks**.

After all cards have been placed, the stacks must allow complete removal of every card using this operation: choose two different nonempty stacks whose top cards have equal rank, and remove both top cards simultaneously.

Output a stack assignment that makes complete removal possible. You need not output the removal sequence.

## Input Format

Read from standard input.

The first line contains $n$. The second line contains the $2n$ ranks $a_1,\ldots,a_{2n}$, in processing order.

## Output Format

Write to standard output.

Print $2n$ integers $c_1,\ldots,c_{2n}$. Card $i$ is pushed onto stack $c_i$, where $1\le c_i\le260$.

Any valid assignment is accepted. Unused stacks are allowed.

## Examples

Input:

```text
3
1 2 3 1 3 2
```

Output:

```text
1 1 1 2 2 3
```

## Explanation / Notes

The shown sample assignment produces stacks $[1,2,3]$, $[1,3]$, and $[2]$, listed from bottom to top. The other 257 stacks are empty. Remove rank 3 from stacks 1 and 2, rank 2 from stacks 1 and 3, and finally rank 1 from stacks 1 and 2.

The deck is generated randomly as specified below. You do not need to minimize the number of stacks; using at most 260 is sufficient.

## Constraints

- $1\le n\le5\times10^5$.
- There are exactly $2n$ input cards.
- $1\le a_i\le n$, with integer ranks.
- Each rank in $\{1,\ldots,n\}$ occurs exactly twice.
- Each input deck is selected **uniformly at random** from all length-$2n$ sequences satisfying the multiplicity condition.
- The test set consists of exactly 50 instances, including the sample. Each run receives one instance.
- Every output stack index must satisfy $1\le c_i\le260$.
- A removal must use two **different** stacks.
- Time limit: **3 seconds**. Memory limit: **512 MB**.

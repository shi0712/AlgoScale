## Problem Description

There are $n$ sets containing $2m$ distinct elements in total. Initially, set $i$ contains $a_i$ elements, and each element belongs to exactly one set.

In each round, randomly partition all elements into $m$ pairs. Independently for each pair, randomly choose one of its two elements and move that element into the set containing its partner. The random matching and endpoint selections are uniform.

Find the expected number of rounds until all elements belong to a single set. Express this expectation modulo $998244353$.

## Input Format

Read from standard input.

The first line contains the number of test cases $T$.

Each case contains a line with $n,m$, followed by a line with the $n$ initial set sizes $a_1,\ldots,a_n$.

## Output Format

Write to standard output.

For each case, print one integer. If the expectation is $P/Q$, output $P\cdot Q^{-1}\bmod998244353$, where the inverse satisfies $QQ^{-1}\equiv1\pmod{998244353}$.

## Examples

Input:

```text
1
2 1
1 1
```

Output:

```text
1
```

## Explanation / Notes

There are two elements, initially in separate sets. There is only one possible pair, and moving either endpoint puts both elements in one set. Exactly one round is required.

Elements remain distinguishable even when they share a set. A set may become empty during the process. If all elements are already in one set, the required number of rounds is zero.

## Constraints

- $1\le T\le100$.
- $n,m$ are integers satisfying $1\le n\le2m\le400$.
- Each initial size satisfies $1\le a_i\le2m$.
- $\sum_{i=1}^{n}a_i=2m$ in every test case.
- All $2m$ elements are distinct and initially belong to exactly one set each.
- Across all test cases, $\sum n\le800$ and $\sum m\le400$.
- Every round pairs all elements, with exactly one selected endpoint per pair.
- Answer modulus: $998244353$.

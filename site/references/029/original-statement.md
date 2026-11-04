# 29. Queue Sorting

## Problem Description

For each value $i\in[1,n]$, there are $a_i$ copies. Let $m=\sum_i a_i$. Arrange this multiset into a sequence $b$ of length $m$.

To sort it, start with two empty FIFO queues. Process $b$ from left to right and append each entry to one of the queues. Only after every entry has been inserted, repeatedly remove the front entry from either nonempty queue and append it to the output sequence.

A sequence $b$ is valid if some choices of queues and removals produce a nondecreasing output. Count the distinct valid value sequences modulo $998244353$. Sequences differ if their values differ at any position; identical copies are not separately labeled.

## Input Format

Read from standard input.

Read $n$, followed by $a_1,\ldots,a_n$.

## Output Format

Write to standard output.

Print the number of valid sequences modulo $998244353$.

## Examples

### Example

Input:

```text
4
1 1 1 1
```

Output:

```text
14
```

## Explanation / Notes

Each queue supports insertion only at the back and removal only at the front. No removal is allowed until all entries have been inserted.

## Constraints

- One test case.
- $1\le n\le500$.
- $0\le a_i\le n$ for every value $i$.
- $1\le\sum_{i=1}^n a_i\le500$.
- A value may be absent because its multiplicity can be zero.
- Modulus: $998244353$.
- Time limit: 1 second; memory limit: 256 MB.

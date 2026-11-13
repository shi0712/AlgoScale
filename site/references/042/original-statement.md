## Problem Description

Start with $n$ variables $a_1,\ldots,a_n$, all equal to zero. Perform $k$ rounds. In each round, independently choose one of the $n$ variables uniformly at random and increase it by 1.

Let $X=\max(a_1,\ldots,a_n)$ after the final round. Compute

$$n^k\,\mathbb{E}[X]\pmod{998244353}.$$

## Input Format

Read from standard input.

Read the two positive integers $n,k$.

## Output Format

Write to standard output.

Print the required integer modulo $998244353$.

## Examples

Input:

```text
2 5
```

Output:

```text
110
```

## Explanation / Notes

There are $n^k$ equally likely ordered sequences of variable choices. Therefore $n^k\mathbb{E}[X]$ is the integer obtained by adding the final maximum over all those sequences; the output is that integer reduced modulo the modulus.

The factor $n^k$ is part of the requested answer. Do not output only the modular expectation.

## Constraints

- There is one input instance.
- $1\le n\le10$.
- $1\le k\le10^5$.
- Both inputs are integers.
- Output modulus: $998244353$.
- Time limit: 3 seconds.
- Memory limit: 512 MB.

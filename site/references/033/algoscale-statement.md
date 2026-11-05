## Problem Description

A pool contains $n$ cards in $m$ types. Type $i$ initially has $a_i$ cards, and the target collection requires $b_i$ cards of that type.

Each refresh draws one card uniformly from the cards currently remaining in the pool. If the target still needs that type, keep the card and remove it from the pool. Otherwise, immediately return it to the pool. Every draw counts as a refresh, including a draw whose card is returned. Continue until all target counts have been collected.

Compute the expected number of refreshes. Represent this rational expectation modulo $998244353$.

## Input Format

Read from standard input.

Read three lines:

1. $n,m$;
2. $a_1,\ldots,a_m$;
3. $b_1,\ldots,b_m$.

Integers on the same line are separated by spaces or tabs. Trailing whitespace at the end of a line or the file is allowed; no additional non-whitespace content is allowed.

## Output Format

Write to standard output.

Let the expectation in lowest terms be $p/q$. Print

$$p\,q^{-1}\pmod{998244353}$$

as an integer from 0 through $998244352$. It is guaranteed that the denominator is not divisible by the modulus.

## Examples

Input:

```text
2 2
1 1
1 1
```

Output:

```text
2
```

## Explanation / Notes

A returned card remains available for later refreshes. A kept card is no longer eligible to be drawn. Sampling is uniform over individual remaining cards, so the probability of a type depends on its current multiplicity.

In the example, both cards are required, so every draw keeps a card and exactly two refreshes are needed. If every $b_i$ is zero, the target is already complete and the expectation is zero.

## Constraints

- There is one input instance.
- $1\le n\le2000$.
- $1\le m\le\min(n,50)$.
- $1\le a_i\le n$ and $\sum_{i=1}^{m}a_i=n$.
- $0\le b_i\le a_i$ for every type.
- All input counts are integers.
- Empty target requirements are allowed, including $b_i=0$ for every type.
- For the reduced expectation $p/q$, $q\not\equiv0\pmod{998244353}$.
- Time limit: 0.5 seconds.
- Memory limit: 256 MB.

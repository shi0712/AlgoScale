# 10. Mountain Climbing

## Problem Description

Given $n$ nonnegative mountain heights $h_1,\ldots,h_n$, minimize their sum after applying two kinds of height-reduction operation:

- Use at most $P$ operations of type 1, replacing a height $H$ by $\lfloor\sqrt H\rfloor$.
- Use at most $Q$ operations of type 2, replacing a height $H$ by $\lfloor H/2\rfloor$.

Both operation types may be applied repeatedly to the same mountain, in any order. Compute the smallest achievable total height.

## Input Format

Read from standard input.

The first line contains $n,P,Q$. The second line contains the $n$ heights $h_1,\ldots,h_n$.

## Output Format

Write to standard output.

Print one integer: the minimum possible sum of heights.

## Examples

Input:

```text
4 1 1
4 5 6 49
```

Output:

```text
18
```

## Explanation / Notes

For the sample, applying both operations to height 49 gives $49\to7\to3$, leaving heights $4,5,6,3$ with total $18$.

Each operation uses the mountain's **current** height, and both transformations round downward. The budgets count operations over all mountains, not separately for each mountain. Since reductions never increase a height, allowing unused operations does not change the optimum.

## Constraints

- One test case is provided.
- $1\le n\le10^5$.
- $0\le P,Q\le n$.
- $0\le h_i\le10^5$ for every $i$.
- All quantities are integers; zero heights and zero operation budgets are allowed.
- The initial sum can be as large as $10^{10}$, so 32-bit signed arithmetic is not sufficient for all possible sums.

- Time limit: 1 second.
- Memory limit: 524288 KiB (512 MiB).

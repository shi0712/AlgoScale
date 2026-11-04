# 68. Cakes

## Problem Description

There are $N$ topping types, numbered $0$ through $N-1$, with $a_i$ pieces of type $i$. A cake's tastiness is the largest number of pieces of a single topping type on that cake.

For each of $Q$ independent requested counts $K_j$, determine whether all available topping pieces can be distributed among exactly $K_j$ nonempty cakes so that every cake has the same tastiness. Cakes may have different total numbers of pieces and different sets of topping types. No piece may be left over.

## Input Format

Read from standard input.

The first line contains $N,Q$. The second contains $a_0,\ldots,a_{N-1}$. Each of the next $Q$ lines contains one requested cake count $K_j$.

## Output Format

Write to standard output.

For every query, print `YES` if such a distribution exists, and `NO` otherwise.

## Examples

Input:

```text
1 1
4
2
```

Output:

```text
YES
```

## Explanation / Notes

Give each of the two cakes two pieces of the only topping type. Both cakes then have tastiness 2, and all four pieces are used.

Each query starts with the original inventory; it does not consume pieces for subsequent queries.

## Constraints

- $1\le N,Q\le100000$.
- $1\le a_i\le100000$ for all topping types.
- $1\le K_j\le10^{18}$ for all queries.
- All values are integers.
- Every cake must receive at least one topping piece.
- All topping pieces must be used exactly once within a scenario.
- The tastiness must be equal across cakes, but it may differ between scenarios.

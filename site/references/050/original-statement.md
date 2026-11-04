# 50. Path Killer

## Problem Description

A tree with $n$ vertices is rooted at vertex 1. There are $m$ specified paths. Path $i$ joins $a_i$ to $b_i$, and $a_i$ is an ancestor of $b_i$, possibly the same vertex.

Repeatedly choose a vertex uniformly at random from all $n$ vertices and delete every remaining specified path that contains it. Stop when all specified paths have been deleted. Determine the expected number of operations, represented modulo $998244353$.

## Input Format

Read from standard input.

Read $n,m$. The second line supplies $n-1$ parent values: its $i$-th entry is the parent of vertex $i+1$. Then read $m$ lines containing path endpoints $a_i,b_i$.

## Output Format

Write to standard output.

If the expectation in lowest terms is $x/y$, print the unique integer $d\in[0,998244352]$ satisfying

$$dy\equiv x\pmod{998244353}.$$

It is guaranteed that the expectation is finite and $y$ is not divisible by the modulus.

## Examples

Input:

```text
3 3
1 1
1 2
3 3
1 1
```

Output:

```text
499122181
```

## Explanation / Notes

Deleting a specified path does not delete tree vertices or edges. Each later draw still uses all $n$ vertices, including vertices drawn earlier. An operation may remove no paths.

A path includes both endpoints, so $a_i=b_i$ describes a one-vertex path. In the sample, completion requires that vertices 1 and 3 both have been drawn; the expectation is $9/2$, represented by the sample answer modulo $998244353$.

## Constraints

- There is one input instance.
- $1\le n,m\le300$.
- For every vertex $i=2,\ldots,n$, its parent $\mathrm{fa}_i$ satisfies $0<\mathrm{fa}_i<i$.
- Every specified path satisfies $1\le a_i\le b_i\le n$.
- Additionally, $a_i$ lies on the route from $b_i$ to root 1; the numerical endpoint inequality alone is not the entire path guarantee.
- Each draw is uniform over $[1,n]$ and later draws may repeat earlier choices.
- Specified paths need not be distinct.
- The reduced expectation has a denominator invertible modulo $998244353$.
- Time limit: 2 seconds.
- Memory limit: 256 MB.

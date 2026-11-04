# 17. Coloring

## Problem Description

There are $n$ elements. Element $i$ has value $w_i$, operation cost $p_i$, and a pointer to element $a_i$. Each element has a binary color $c_i$.

Initially, $c_s=1$ and every other color is 0. An operation on index $i$ replaces its color by the current color of the pointed-to element:

$$c_i\leftarrow c_{a_i},$$

and costs $p_i$. Perform any finite number of operations, possibly zero. Your final score is the sum of $w_i$ over elements whose final color is 1, minus the total cost of all operations. Find the maximum possible score.

## Input Format

Read from standard input.

Read $n,s$, then three lines containing $w_1,\ldots,w_n$, $p_1,\ldots,p_n$, and $a_1,\ldots,a_n$, respectively.

## Output Format

Write to standard output.

Print the maximum score as one integer.

## Examples

### Example

Input:

```text
3 1
-1 -1 2
1 0 0
3 1 2
```

Output:

```text
1
```

## Explanation / Notes

In the example, operate on elements $2,1,3,2$ in that order. The color arrays become $[1,1,0]$, $[0,1,0]$, $[0,1,1]$, and $[0,0,1]$. The only final contribution is $w_3=2$, and the total cost is 1, giving the optimal score 1.

## Constraints

- One test case.
- $1\le s\le n\le5000$.
- $-10^9\le w_i\le10^9$; values may be negative.
- $0\le p_i\le10^9$; zero-cost operations are allowed.
- $1\le a_i\le n$ and $a_i\ne i$ for every index.
- Initially only element $s$ has color 1.
- The same index may be operated on repeatedly; each operation incurs its cost.
- Time limit: 2 seconds; memory limit: 1024 MB.

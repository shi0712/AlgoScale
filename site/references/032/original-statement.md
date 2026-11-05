## Problem Description

Given $n$ positive integers $a_1,\ldots,a_n$, calculate

$$\sum_{S\subseteq\{1,\ldots,n\}}\varphi\left(\prod_{i\in S}a_i\right)\pmod{998244353}.$$

Here $\varphi(x)$ is Euler's totient: the number of integers in $[1,x]$ that are coprime to $x$. The empty product is defined to be 1, and $\varphi(1)=1$.

## Input Format

Read from standard input.

Read $n$, then the $n$ integers $a_1,\ldots,a_n$.

## Output Format

Write to standard output.

Print the requested sum modulo $998244353$.

## Examples

### Example

Input:

```text
3
1 2 3
```

Output:

```text
12
```

## Explanation / Notes

For the example, the eight index subsets produce products $1,1,2,2,3,3,6,6$. Since $\varphi(1)=\varphi(2)=1$ and $\varphi(3)=\varphi(6)=2$, their sum is $4\cdot1+4\cdot2=12$.

Subsets are subsets of indices. Different subsets still contribute separately when their products are equal. The empty subset contributes 1.

## Constraints

- One test case.
- $1\le n\le2000$.
- $1\le a_i\le3000$ for every index.
- Equal input values are allowed.
- Include all $2^n$ subsets, including the empty subset.
- Modulus: the prime $998244353$.
- Time limit: 2 seconds; memory limit: 512 MB.

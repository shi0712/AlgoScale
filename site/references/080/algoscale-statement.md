## Problem Description

This is an interactive problem.

The Department of Supernatural Phenomena at the Oxenfurt Academy has opened the Library of Magic, which contains the works of the greatest sorcerers of Redania — $n$ ($3 \leq n \leq 10^{18}$) types of books, numbered from $1$ to $n$. Each book's type number is indicated on its spine. Moreover, each type of book is stored in the library in exactly two copies! And you have been appointed as the librarian.

One night, you wake up to a strange noise and see a creature leaving the building through a window. Three thick tomes of different colors were sticking out of the mysterious thief's backpack. Before you start searching for them, you decide to compute the numbers $a$, $b$, and $c$ written on the spines of these books. All three numbers are distinct.

So, you have an unordered set of tomes, which includes one tome with each of the pairwise distinct numbers $a$, $b$, and $c$, and two tomes for all numbers from $1$ to $n$, except for $a$, $b$, and $c$. You want to find these values $a$, $b$, and $c$.

Since you are not working in a simple library, but in the Library of Magic, you can only use one spell in the form of a query to check the presence of books in their place:

* "xor l r" — Bitwise XOR query with parameters $l$ and $r$. Let $k$ be the number of such tomes in the library whose numbers are greater than or equal to $l$ and less than or equal to $r$. You will receive the result of the computation $v_1 \oplus v_2 \oplus... \oplus v_k$, where $v_1... v_k$ are the numbers on the spines of these tomes, and $\oplus$ denotes the operation of bitwise exclusive OR.

## Input Format

Read n.

## Output Format

```text
xor l r
ans a b c
```

The reply is the bitwise XOR of the types of all copies remaining in the inclusive interval. An empty XOR is zero. Report the three stolen types in any order. The theft is fixed. Exit immediately on −1.

Fewer counted interactions earn a higher score. The final answer is free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `3` | |
| | `xor 1 3` |
| `0` | |
| | `ans 1 2 3` |

## Constraints

- 3 ≤ n ≤ 10¹⁸; 1 ≤ l ≤ r ≤ n.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

## Problem Description

This is an interactive problem. Reconstruct a hidden rooted tree by operating a ball machine through standard input and standard output.

The tree has $N$ vertices indexed $0,\ldots,N-1$, with root $N-1$. Every other vertex $u$ has parent $P[u]>u$. The leaves are exactly the vertices $0,\ldots,M-1$. You are given only $M$; both $N$ and the parent array are hidden.

Each vertex can hold one ball with a nonnegative integer value. Initially every vertex is empty. You may perform two operations:

- **Insert a ball.** Choose a leaf $U$ and a value $X$. If leaf $U$ is occupied, the insertion fails and the machine does not change. Otherwise put a new ball of value $X$ at $U$ and repeatedly move it to its parent while that parent is free. Stop at the root or immediately below an occupied parent.
- **Collect the balls.** If the root is free, the machine is empty and the result is an empty sequence. Otherwise traverse the occupied vertices from the root in preorder. At each vertex, append its ball value, then recursively visit its occupied children in nondecreasing order of their ball values. Children with equal values may appear in either order. After the traversal, remove every ball and report the collected sequence.

Determine $N$ and a parent array describing the tree, allowing relabeling of internal non-root vertices. Leaf labels and the root label must stay fixed. The machine must be empty when you submit the answer.

## Input Format

Read from standard input.

First read one integer $M$, the number of leaves.

After an insertion query, read one integer: `1` if the insertion succeeded, or `0` if the selected leaf was already occupied.

After a collection query, read one line containing

```text
s v_1 v_2 ... v_s
```

Here $s$ is the number of collected balls, and $v_1,\ldots,v_s$ is their collected sequence. An empty collection is reported as a line containing only `0`.

If a response is `-1`, an interaction error has occurred. Terminate the program immediately without printing anything further.

## Output Format

Write to standard output.

To insert a ball of value $X$ at leaf $U$, print:

```text
? 1 U X
```

To collect all balls, print:

```text
? 2
```

When you have determined the tree and the machine is empty, print:

```text
! N R_0 R_1 ... R_{N-2}
```

The reported $N$ must equal the hidden number of vertices. The $N-1$ values $R_0,\ldots,R_{N-2}$ form a valid parent array if there is a bijective relabeling $L$ of the hidden vertices such that:

- $L[u]=u$ for every leaf $0\le u<M$ and for the root $u=N-1$;
- $L[P[u]]=R_{L[u]}$ for every $0\le u<N-1$.

The reported parents need not satisfy $R_u>u$.

End every output with a newline and flush standard output. Read the response after each query. After printing and flushing the final answer, exit without waiting for another response.

## Examples

### Interaction example

The hidden tree in this example has $N=7$, $M=4$, and $P=[4,6,5,5,6,6]$. Only $M$ is initially revealed to the program.

| Judge sends | Program sends |
| --- | --- |
| `4` | |
| | `? 1 0 0` |
| `1` | |
| | `? 1 3 2` |
| `1` | |
| | `? 1 1 1` |
| `1` | |
| | `? 1 2 3` |
| `1` | |
| | `? 1 1 3` |
| `0` | |
| | `? 1 0 2` |
| `1` | |
| | `? 2` |
| `5 0 1 2 3 2` | |
| | `? 1 2 3` |
| `1` | |
| | `? 2` |
| `1 3` | |
| | `! 7 4 6 5 5 6 6` |

## Explanation / Notes

The first ball rises from leaf 0 through vertex 4 to root 6. The next ball stops at vertex 5 because its parent is occupied. The fifth insertion query fails because leaf 1 already contains a ball.

Before the first collection, root 6 has child ball values 2 at vertex 4, 1 at vertex 1, and 2 at vertex 5. Visiting these children in order $[1,5,4]$ yields the displayed sequence. The order $[1,4,5]$ is also permitted by the equal-value rule. Each collection empties the machine.

There are two collection queries, and the largest value used in an insertion query is 3. Thus $K=2$, $B=3$, and $C=5\le27$. The machine is empty when the final answer is printed.

The hidden tree is fixed before interaction starts. The judge is deterministic: identical sequences of queries produce identical response sequences, including the order used for equal-valued children.

Resource usage counts every insertion query, including failed insertions, and every collection query. Let $K$ be the number of collection queries and $B$ the largest value $X$ used in any insertion query. If no insertion query has been made, define $B=0$. Failed insertions still count when determining $B$, and collecting an empty machine still increases $K$. At every moment, the resource cost must satisfy $C=K+B\le27$.

## Constraints

- $2\le N\le1000$.
- $1\le M\le200$ and $M<N$.
- The leaves are exactly $0,1,\ldots,M-1$.
- Root: $N-1$; hidden parents satisfy $u<P[u]<N$.
- Each insertion query must satisfy $0\le U<M$ and $0\le X\le1000$.
- At most $500000$ insertion queries are allowed, including failed insertions.
- $K$ is the number of collections; $B$ is the largest ball value appearing in any insertion query; $C=K+B$ must never exceed 27.
- The machine must contain no balls when the final answer is submitted.
- Time limit: 1.5 seconds; memory limit: 256 MB.

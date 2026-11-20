## Overview

Recover the column A[i]=p[i] XOR inverse(p)[0]. For even n the XOR of the complete column is the XOR of 0,…,n−1 since a common mask occurs an even number of times. Infer the last row entry without querying. For each candidate shift s, try p_s[i]=A[i] XOR s; reject it if any value leaves the range or A[s] differs from s.

## Strategy and correctness

These tests enumerate precisely the first-column-consistent permutations. Compute each candidate's entire row-zero fingerprint from its inverse. Query a row position only when the surviving fingerprints differ there and eliminate mismatches. The final coordinate is again redundant for even n. Each full-matrix entry is determined by its row-zero and column-zero values and their overlap, so identical complete fingerprints are exactly the indistinguishable correct answers. Output the number of survivors and any one of them.

## Interaction and local complexity

Suppose $n=2^d$, and write $G$ for the group of $d$-bit vectors under XOR. The first column $A$ is a permutation of $G$. Set $D(y)=A^{-1}(y)\mathbin{\mathrm{xor}}y$. For a candidate shift $s$, its predicted answer at $(0,j)$ is $A[0]\mathbin{\mathrm{xor}}j\mathbin{\mathrm{xor}}D(j\mathbin{\mathrm{xor}}s)$. Hence comparing this answer with that of the true shift $s_0$ is equivalent to comparing the two translated values of $D$. Candidate shifts from the first phase satisfy $D(s)=0$, including $s_0$.

Identify $G$ additively with $K=\mathrm{GF}(2^d)$ and form $f=\sum_{y\in G} D(y)e_y$ in the group algebra $K[G]$, where $e_ue_v=e_{u\mathbin{\mathrm{xor}}v}$. For any $g$, its square is $g^2=(\sum_y g_y)^2 e_0$ because the cross terms cancel and $e_y^2=e_0$. The sum of the coefficients of $f$ is zero because $A^{-1}$ permutes all the labels. Thus $f^2=0$, and multiplication by $f$ has image contained in its kernel, hence rank at most $n/2$.

If $f=0$, every fingerprint is identical and there are no second-phase queries. Otherwise any vector in the kernel must have coefficient sum zero. A nonzero coefficient sum would make its square a nonzero scalar times $e_0$, making that vector invertible. Restricting multiplication by $f$ to the codimension-one space $I$ of zero coefficient sum therefore preserves its kernel and reduces its rank by one. The columns of the mismatch matrix

$$M_{j,s}=D(j\mathbin{\mathrm{xor}}s)+D(j\mathbin{\mathrm{xor}}s_0)$$

are the images of $e_s+e_{s_0}$, which span $I$ as $s$ ranges over $G$. Consequently $\operatorname{rank}M\le n/2-1$. Row zero is nonzero on the full set of shifts and already known from the first column. Every new paid query has a surviving candidate that disagrees on the new row and agreed on every old row, including row zero. Its row is linearly independent of those already used. This leaves at most $n/2-2$ paid queries after the initial $n-1$ for $n\ge4$. Sizes one and two are immediate.

When $n$ is not a power of two, XOR differences between valid shifts preserve the interval $[0,n)$. Its translation stabilizer consists of masks smaller than $\operatorname{lowbit}(n)$. Indeed these masks preserve each aligned block of that size. A mask preserving the interval with highest bit $k$ pairs the two halves of every aligned block of length $2^{k+1}$. A partially filled terminal block cannot have equal-size halves, so $2^{k+1}$ must divide $n$. There are thus at most $\operatorname{lowbit}(n)$ candidate shifts, and each paid query eliminates at least one. The total is at most $n+\operatorname{lowbit}(n)-1\le5119$ over these sizes.

The largest power of two allowed is 4096, giving $(4096-1)+(2048-2)=6141$ queries. This implementation attains the bound. In official point 20 the first column is the identity with labels 1 and 2 exchanged. Candidate shifts come in equivalent pairs differing by XOR 3. After the first column there are 2047 pairs. Each additional query can isolate only one pair, since $D$ is zero except at 1 and 2. With the fixed true shift 4092, the reference tests 2046 other pairs first. The archived transcript in `admin/tools/frozen_equality.json` contains all 6141 replies. Run `python admin/tools/check_equality.py` to check them against the fixed permutation. The precomputation of candidate fingerprints uses $O(n^2)$ time and memory, and scanning all surviving candidates over all second-phase coordinates takes $O(n^2)$ time.

## Overview

For divisor two, the quotient buckets are pairs of consecutive values, except for the singleton {1} and, when n is even, singleton {n}. Split the candidate positions in two. Compare defects 2Q−length for the full prefix and suffix at the cut. A split pair contributes one to each side, hence cancels in this comparison. The difference counts singleton positions on one side minus those on the other.

## Strategy and correctness

With odd n this test immediately locates 1. With even n an equal defect places 1 and n on opposite sides; a divisor-n query reveals the side containing n, whose contribution can be removed in all subsequent cuts. Once two adjacent positions remain, the memoized divisor-two prefix or suffix counts identify which contributes the singleton. If the identities of 1 and n are still paired, the final divisor-n test separates them. The program handles empty and singleton intervals without querying; full-array results are known from n. Prefix and suffix counts used on preceding splits are cached.

## Interaction and local complexity

There are at most nine cuts on intervals of size greater than two when $n\le1024$. Each costs at most two divisor-two queries. There is at most one divisor-$n$ disambiguation. If it is still needed when two positions remain, it alone finishes the search. Otherwise write the last interval as $[l,r]$, $r=l+1$. Each outside boundary, at $l-1$ or $r$, came from an earlier cut or from an array endpoint. The prefix and suffix counts at those cuts are cached or known. Thus only the previously unseen cut between $l$ and $r$ can cost another query. This gives $2\cdot9+1+1=20$. Official point 9 attains 20. Local state is $O(\log n)$ cached queries, each with $O(\log\log n)$ map lookup time.

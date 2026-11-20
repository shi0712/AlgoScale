## Overview

Split the array into blocks of a power-of-two width w. Read zero totals for every block except the last. Maintain these totals in a Fenwick tree, decrementing the chosen block after each reported zero is flipped.

## Strategy and correctness

Binary search the block-prefix zero totals to find the first block containing the requested rank; if every queried prefix is too small it lies in the final block by the task guarantee. Subtract the preceding blocks' total. Within that block, a range query returns ones, so interval length minus this answer gives zeros; an ordinary binary search recovers the requested position. No stale count survives an update, and the last block's total is never needed in the prefix search.

## Interaction and local complexity

For width $w=2^d$ the cost is at most $\lceil n/w\rceil-1+td$, summed across all $t$ requests on the same array. Width one needs at most $n-1$ queries. For $n\ge9$, the width search includes 16 and can select a width with budget no larger than $\lceil n/16\rceil-1+4t\le12499+40000=52499$. The initial default estimate $n$ also bounds the cost of width one. Smaller instances use at most $n$ queries. At $n=200000$, $t=10000$, the reference chooses width 16. Every block has exactly 16 entries, so the within-block binary search always performs four paid queries. The 12,499 initial block counts and 40,000 later queries give exactly 52,499 for every legal request sequence at this size. The round answers are free. Fenwick updates and each prefix sum take $O(\log n)$ time, giving $O(n\log n+t\log^2 n)$ local time and $O(n)$ memory.

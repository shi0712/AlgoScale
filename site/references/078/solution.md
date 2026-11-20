## Overview

Open the doors in order. Each experiment made with the first $j$ doors open tells us about door $j$. Whenever it also opens door $j$, its reply tells us about door $j+1$ as well. Save several of these experiments to reduce the candidate set before querying the next door.

## Invariant and strategy

Keep all determined switches at their correct settings. At the start of a door, compare each unused switch against the saved experiments. In the first experiment its setting and the result determine which setting it would require if it controlled this door. Discard a switch if that required setting disagrees with any other saved result. The actual switch always survives. A reply of $-1$ means every door opened.

A new experiment can force any chosen set of candidate switches to fail, by writing their respective incorrect settings. For the other candidates write their correct settings. The reply selects the appropriate subset. With one candidate left we know its index and its required bit.

When several experiments open the current door, group the unused switches by the pattern of changes relative to the first successful experiment. The current controlling switch belongs to group zero. Remove it from this group's effective size. An unchanging switch can still control the next door whether it opened or closed on all saved trials. For every other group at least one of the two switch settings occurred with the next door open if that switch controls the next door. In the next stage the saved replies confine its candidates to the one compatible change pattern, and that successful experiment is available without another query.

## Query bound

The planner tries the 13 split ratios stored in the reference program. For a fixed ratio $p$, after $t\ge1$ successful grouping probes let $e$ be the effective size of group zero and let $o$ bound all other group sizes. Initially $e=k-1,o=0$ for $k$ unused switches. A new split gives $e'=\lfloor pe\rfloor$ and $o'=\max(\lceil o/2\rceil,e-e')$. The failure branch may have at most $e-e'$ candidates, and the success branch at most $1+e'$. The true controlling switch always stays in group zero on success. Fill the remaining positions using noncandidates to realize those split sizes. An ordinary binary split without a further grouping probe is also possible.

Let $F(k,m,s)$ denote the certified remaining budget for $m$ candidates with $k$ switches left. The flag $s$ indicates an available successful experiment. For each ratio compute the capacity $C_t(B)$ for distinguishing candidates using budget $B$ when $t$ grouping probes have succeeded. A singleton may stop if the budget covers the next stage. Its cost is $1+F(k-1,k-1,0)$ for $t=0$ (one probe to open the current door), or $\max(F(k-1,e,0),F(k-1,o,1))$ for $t>0$. Empty groups cost zero. With one query spent, the two successor capacities add:

- Before the first success use $C_0(B-1)+C_1(B-1)$.
- After a success an ordinary split handles $2C_t(B-1)$.
- When another grouping probe is allowed, it handles $\min(C_t(B-1),e-e')+\min(C_{t+1}(B-1),1+e')$.

Take the largest available capacity, bounded by $k$. Invert the capacities to obtain the smallest certified budget for each state, minimize over ratios, and allow an unused known success to be discarded. At $k=1$ a known reply already determines the setting. Induction first on $k$ and then on $B$ proves that the candidate counts fit the chosen action on every branch. The implementation uses at most 13 saved grouping probes.

Including the initial all-zero experiment, the maximum certified value over $1\le N\le5000$ is **38,188 queries**. The final report is free. The independent integer calculation in `admin/tools/query-certificate/certificate.py` reproduces this bound.

## Implementation and resources

Represent a setting by a bit string and flush each query line. The first-closed-door reply supplies the open/closed status for both the current door and any later door known to have been reached. Every door removes one unused index, so the final permutation and settings open every door.

The ratio count and grouping depth are constants. A query processes $O(N+2^{13})$ entries and prints $N$ bits. If $W$ is the largest number of budget entries stored for any stage, preprocessing all stages uses $O(13^2NW)$ arithmetic operations and $O(NW)$ stored capacities. Copies of saved experiments occupy $O(13N)$ additional space.

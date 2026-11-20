## Overview

Recover the two horizontal distances $L,R$ from the initial red cell to the edges of its own red square. Its side length is $M=L+R+1$. A vertical search then determines the centre of this square. The possible positions of this red square within the five-by-five board form a set of only 13 candidates.

## Horizontal search

Alternate six preliminary probes between the left and right sides at distances 164455670, 122512630, 69254700, 18898705, 1204020, and 2135. A positive probe at distance $d$ guarantees $5M>d$, even if it lands in another red square. A negative probe bounds the same-square edge distance by $d-1$. An out-of-meadow point is known to be unflattened and costs no query. Maintain separate upper bounds for $L$ and $R$, a lower bound on $M$, and the upper bound on $M$ given by those edges.

Once a point at distance $a$ is known to be in the initial square, a step of length at most the known lower bound on $M$ cannot cross an entire white square to reach a later red square. Hence every probe in such a step is an ordinary boundary test. After a failure at $q$, binary search in $[a,q-1]$ gives the exact distance. This invariant allows the search to increase its step size as it learns more.

Try both choices of the side to find first. After finding its edge distance, exploit the fact that $L$ and $R$ have the same parity because $M$ is odd. On the second side only every other distance needs testing. If index $j$ is already known positive and $h$ queries remain, advance by at most the smaller of half the known width bound, rounded down, and $2^{h-1}$. The width restriction preserves the boundary-test invariant, and the power-of-two restriction leaves enough binary-search steps after a failure. Iterating this rule computes a sufficient second-side budget. Each proposed first-side probe reserves the binary-search cost of its own failure and the corresponding second-side budget, including the case where the final first-side probe succeeds.

## Centre and query bound

With $M$ known, the downward distance inside the square lies between $0$ and $M-1$, so ordinary binary search needs at most $\lceil\log_2 M\rceil$ additional probes. To distinguish the 13 candidate red squares, probe offsets that are integer multiples of $M$ from the square's centre. For each such offset the colour of the answer is known for every candidate. The reference enumerates the two successor masks and minimizes the maximum remaining depth. Its full mask has depth four. The surviving position fixes the centre of the whole formation.

Running the reference with `--certificate` checks the first-positive branch of each preliminary probe and the all-negative branch. For each it verifies the horizontal plan with a budget of $92$ minus the preliminary cost, vertical budget, and four final probes. The full-range worst case is **exactly 92 examinations**. Private point 20 has $N=2{,}000{,}000{,}000$, $M=281{,}976{,}393$, formation lower left $(563{,}952{,}787,1)$, and initial cell $(1{,}134{,}918{,}530,1{,}252{,}276{,}516)$. Running the supplied reference reaches 92 on this input.

The board search stores at most $2^{13}$ masks and tests at most 81 offsets per mask. The remaining planning consists of bounded integer searches on distances, with no array proportional to the meadow size. Coordinates and products of distances are evaluated in 64-bit arithmetic. Each printed query or answer is flushed.

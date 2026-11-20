## Overview

For four candidates make the four triple-range queries that omit one position each. Let W be their largest reply. Omitting a position lowers the range exactly when that position alone attains the minimum or maximum. A unique zero, when present, is one such position.

## Strategy and correctness

Discard a group without any unique end and retain its single candidate if it has only one. For the first group with two unique ends, keep its extremes a,b and one strict interior point z. To incorporate two new positions x,y, ask the ranges of (a,b,x) and (a,b,y). An unchanged width means the point is inside. If only x lies outside, range(a,z,x) identifies the old endpoint farthest from x. If both lie outside, range(a,x,y) exceeds both preceding answers exactly when x and y are on opposite sides. On the same side it also identifies the old far endpoint, while the larger preceding reply identifies the new outer endpoint. The enlarged interval still contains z strictly inside. If a pass finds no two-end group, repeat on the retained positions and leftover candidates. Pad a final group of three with a known nonzero index.

## Interaction and local complexity

If the first usable group has index j, its 4j probes leave at most s−3j−1 positions, merged at cost ceil(3(s−3j−1)/2). If all floor(s/4) groups compress, pay four per group and recurse on at most floor(s/4)+(s mod 4) positions. With F(1)=F(2)=0, F(3)=4, maximizing this recurrence for s≤1,000 gives 1,498, which is attained. Time and space are O(n).

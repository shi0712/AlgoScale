## Overview

A deletion set preserves a connected cactus exactly when it contains no bridge and at most one edge from any cycle. Scan edges by label and keep 15 versions B[0],…,B[14]. Version B[p] contains the (p+1)-st encountered edge of every cycle already seen at least p+1 times. B[15] is the original graph.

## Strategy and correctness

For the next edge, deleting it from B[p] succeeds precisely beyond a threshold: the threshold is its number of earlier edges in this cycle, capped at 15; a bridge fails even at 15 and has class 16. Find that threshold via the fixed ordered comparison tree and, below 15, retain the corresponding successful version as the new B[p]. Let R[p] be the number of edges assigned to class p. There are R[l−1]−R[l] cycles of length l for 1≤l<15 and R[14] cycles of length at least 15. For an edge in class b<14, a deletion from final B[p] with p>b now succeeds precisely when the cycle length is at most p. Each class contains one edge of every relevant cycle, so the frequency of every possible answer is known. Build an optimal ordered tree for these frequencies, query until one length remains and decrement its frequency. Large cycles and bridges already have their required output labels.

## Interaction and local complexity

For frequency vector w, the minimum weighted external depth F(w) pays for a search: deleting the found leaf occurrence and pruning the old tree shows F(w−e)≤F(w)−depth(e). The rational tree-flow certificate admin/tools/certify.py bounds the combined cost by 395l/56 per length-l cycle and six comparisons per bridge. The resulting bound is floor(395m/56), at most 70,535. The DP has at most 15 leaves, taking O(15³m) local time and O(m+15²) space.

## Overview

Let h be the least number of measurement rows such that the sum of popcounts of 0,…,h−1 is at least n−1. Reserve the final coordinate. Assign up to |S| other coordinates to each nonempty bit mask S<h, indexed by j=0,…,|S|−1. In row x, column (S,j) is true if x∩S is among the first 2^j odd-cardinality subsets of S in numerical order.

## Strategy and correctness

The all-false row gives the total number W of true answers. A score on a set A of true guesses is n−W−|A|+2|A∩hidden|, so each observation gives an intersection count. Process masks in decreasing order. Take the alternating sum of the remaining row counts over all x⊆S. Any column depending on a proper subset of S cancels in pairs, as does a column whose mask fails to contain S. Contributions from strict supersets have already been subtracted. Every active subset of S has odd cardinality, so the column (S,j) contributes the same sign times 2^j. The binary representation of the absolute alternating sum yields all true positions assigned to S. Remove their contributions and update the remaining weight. The reserved position is the only one left. Submit the resulting answer key, stopping earlier if any guess gets a perfect score.

## Interaction and local complexity

The h measurements and final answer total at most 254 submissions for n≤1,000, including the winning submission. The row and column tables require O(h²+nh) memory. Building and subtracting the tables takes O(nh), besides O(h²) subset initialization and signed subset sums, so O(h²+nh) local time suffices.

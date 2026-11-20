## Overview

Tabulate the scaled triangle area for all ordered triples of side lengths 1 through 4 using Heron's formula, with zero for a degenerate triangle. On at most nine unknown positions query every triple and enumerate all assignments compatible with the replies.

## Strategy and correctness

Among nine positions some three have the same value. The four all-equal replies are unique in the table, so examining up to 84 triples supplies two known equal anchors. For an anchor value of 2, 3 or 4, the query using both anchors and a fresh position distinguishes all four possible values. A pair of ones identifies exactly which tested positions also hold one. Stop this scan after four non-ones: among the four some pair is equal, and their triangle with one identifies the repeated value. Use this stronger pair for the entire remaining suffix. If fewer than four non-ones exist, compare each pair with a known one. In the absence of an equal pair, either there are none, or permuting the distinct non-ones (or changing a sole non-one) leaves all replies unchanged. The tiny exhaustive branch then reports ambiguity. Otherwise the compatible assignment from the queries is unique.

## Interaction and local complexity

Initial probing costs at most 84 queries. Only the at most four non-ones handled before switching anchors can require repeated inspection. At most six pair probes and a bounded number of already scanned positions give the bound n+90 for n≥10, hence at most 5,090. Inputs of size at most nine use at most 84 queries. Local work is O(n+4⁹·9³), with O(n) storage apart from a fixed small table.

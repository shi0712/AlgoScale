## Overview

Query the full XOR and embed the legal positions in an aligned binary interval. At each split, query the XOR in the left half; XOR with the total gives the right-half result without another query.

## Strategy and correctness

Two distinct positions cannot XOR to zero and a singleton is positive. If both sides are nonzero while three positions remain, their counts are one and two. All right-half positions have the split bit set, so its presence in that side's XOR identifies odd cardinality. Record the singleton and continue in the other half. If both XORs vanish, all three must be in the left half because three right-half positions would retain the split bit. With only a pair left, nonzero values on both sides identify both positions immediately. At width at most eight, enumerate every legal pair or triple consistent with the known XOR. Try every local interval until its replies separate the remaining possibilities. The finite terminal check covers both a zero and nonzero high prefix, all truncation lengths, and both possible cardinalities.

## Interaction and local complexity

For h=floor(log₂ n)+1 there is one initial query, at most h−3 splitting queries and at most one terminal query, with at most two queries on smaller instances. The global bound is 59. Local work is O(log n) plus a constant-size enumeration and the stored candidate list is bounded independently of n.

## Overview

With vertex 1 as root, a guess for every edge orientation returns a Hamming-match count. Recover the orientation by assigning several unknown coordinates to each nonempty mask S. Use rows x and columns indexed by (S,j); a column is active when x∩S lies among the first 2^j odd-cardinality subsets of S. The all-zero measurement determines the weight. Differences from it give intersection counts. The signed subset sum on S cancels every column depending on a proper subset; after removing decoded supersets, its magnitude encodes the bits for S in binary. Decode in decreasing mask order and derive the final bit from the weight.

## Strategy and correctness

With that orientation, a query at each other vertex gives its depth. Process vertices in increasing depth. For v at depth d exactly one unassigned edge and one incoming edge of a depth-(d−1) vertex are on its root path. Flip any chosen subsets of the former and latter edge sets. After correcting for the known base score and number of flips, the reply gives the sum of the two membership bits. When the previous layer consists of the root the parent is already known. Otherwise split both candidate sets into four groups. The explicit three-query tree in the source distinguishes their 16 ordered group pairs, as one can check by simulating its two bit masks. Alternatively bisect either candidate set alone. Continue until a unique edge and parent remain, and assign the ordered endpoints using the recovered orientation.

## Interaction and local complexity

For a current choice of a edges and b parents, use the lesser of the independent binary-search cost and 3+C(ceil(a/4),ceil(b/4)). The exact audit in admin/tools/tight148.cpp evaluates the chosen tree at each size and maximizes over every possible sequence of layer populations. Together with depth and orientation queries the bound is 6,715 for n≤500. The packaged witness attains it. The implementation uses O(n²) storage and O(n³ log n) local time as a conservative bound.

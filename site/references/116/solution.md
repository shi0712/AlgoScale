## Overview

Query each singleton to record its original root-to-vertex path sum. Find the hidden root by repeatedly taking a centroid of the current connected candidate subtree. Its branches and the possible singleton centroid form the candidate groups.

## Strategy and correctness

Flip the centroid once. A representative in the branch containing the root keeps its old path sum, while every representative on the opposite side has its sum changed by the same ±2. A batched query on one representative per tested branch therefore distinguishes whether the root's group is among them by the absolute difference from the stored sum. Assign a prefix code to each group of size s in a component of size m, with length ceil(log₂(m/s)); Kraft's inequality holds because the group sizes sum to m. Choose the tested side excluding the centroid singleton. Navigate the code, restore the flip, and restrict to the matching group. Once the root is known, subtract the original parent's path sum from each child's sum to recover its value.

## Interaction and local complexity

Each stage on the chosen component costs two flips and at most ceil(log₂(m/s)) queries. A nonterminal branch has size at most m/2. One can tabulate the bound by component size using these splits; the verified maximum at n≤1000 is 1035 including the n singleton queries. A simpler bound n+4floor(log₂ n)≤1036 also certifies the original cap. The component sizes fall geometrically; sorting branch codes and constructing query lists require O(n log n) local work and O(n) storage.

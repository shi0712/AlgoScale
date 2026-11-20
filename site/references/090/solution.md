## Overview

Query the root-to-centre sum c for vertex 1, the centre of the given star. For every leaf the path sum differs from c by exactly +1 or −1. The protocol allows a vertex to occur repeatedly in a sum query. Group up to floor(log₂(n+1)) leaves and use multiplicities 1,2,4,… so the reply encodes the entire group's signs.

## Strategy and correctness

If c is odd then the centre is the root, so c and the signs give every answer directly. Otherwise the root is a leaf. Flip vertex 1 once. Every leaf's path sum changes by the same ±2 except at the root, where the change is zero. Give candidate leaves multiplicity floor(i/w), numbering them from zero. Compare the observed reply with the old weighted sum. The missing change reveals the root's multiplicity, reducing the candidates to one consecutive group of at most w. Choose the smallest w whose total multiplicity is at most n. Once the root is known, recover its value from its original sum and adjust the centre value for the flip.

## Interaction and local complexity

For s candidates define W(s) as the smallest w≥1 with sum_{i=0}^{s−1} floor(i/w)≤n. The root search costs at most one query per iteration s←W(s), stopping at one. Add the centre query, ceil((n−1)/floor(log₂(n+1))) digit queries and at most one flip. Evaluating these integer expressions for n≤1,000 gives at most 117 operations. The lists use O(n) space and O(n²) local time suffices.

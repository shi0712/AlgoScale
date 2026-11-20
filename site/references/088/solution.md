## Overview

Maintain a set A known to belong to a single connected component. Initially A contains just vertex 1. Write f(S) for the number of neighbours outside S. If f(A)=0 while vertices remain outside, report that the graph is disconnected.

## Strategy and correctness

For disjoint A and B, the difference f(A)+f(B)−f(A∪B) counts vertices of A adjacent to B, vertices of B adjacent to A, and common neighbours outside both sets. It is positive exactly when some member of B is at graph distance at most two from A. This property is the union of a per-vertex predicate, so it remains valid under binary search on the outside vertices. Test half the candidates using f(B) and f(A∪B), keeping a half containing a witness. The resulting vertex belongs to A's component even when its connection passes through an unselected vertex. Insert it and repeat. When A covers all vertices the graph is connected.

## Interaction and local complexity

With m outside vertices this iteration costs at most 1+2ceil(log₂ m). Summing for m=1 through n−1 gives at most 2,873 queries when n≤200. Forming bit strings costs O(n² log n) time and O(n) extra storage.

## Overview

Insert vertices one at a time. Store a spanning tree and two colour classes for each connected component of the graph induced by earlier vertices. To find a neighbour of the new vertex v, take the union S of one colour class from each component still under consideration and query S together with v.

## Strategy and correctness

The set S is independent. The reply therefore counts exactly the edges from v into S. After a positive reply, bisect S until one neighbour x is found. In x's component test the opposite colour class. If it also contains a neighbour y, the tree path from x to y has odd length and closing it with the two edges to v gives an odd cycle. Otherwise flip the component's colours if necessary and join it to v by the edge vx. Remove it from the search and continue. When neither union contains a neighbour, all remaining components are separate. If every insertion finishes, the first colour classes form a valid side of a bipartition.

## Interaction and local complexity

A successful merge costs at most two union queries, ten bisections and one opposite-side query. At most n−1 joins or terminal collisions occur, and each insertion after the first uses at most two final unsuccessful queries. The terminal collision can cost ten additional bisections. This gives 15(n−1)+10 ≤ 8,995 queries. Constructing the query lists takes O(n² log n) time and O(n) auxiliary space.

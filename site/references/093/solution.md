## Overview

Treat the footprints as a directed path with one source and one sink. For a rectangle R, compute incoming crossings minus outgoing crossings and add one if the starting cell lies inside R. The result is exactly the indicator that the treasure is in R.

## Strategy and correctness

Internal path edges cancel, and every path vertex other than the source and sink has equal in- and out-degree. Thus previously read outer boundaries together with a new cut suffice to decide which half contains the sink. Alternate vertical and horizontal cuts. For a cut of length h visit both adjacent lanes. Starting on either lane, go to one end and back on the opposite lane, zigzag towards the midpoint, then cover the remaining portion and return to the midpoint. This costs at most 2h moves and positions the explorer at the next cut. Entering the retained rectangle takes at most one extra move. Once one dimension is one, go to the nearer end of the remaining segment and scan to its other end. Stop immediately on entering the treasure cell.

## Interaction and local complexity

For width w in the direction being split, the upper bound is F(w,h)=2h+1+max(F(h,floor(w/2)),F(h,ceil(w/2))). A one-dimensional segment of length s costs at most floor(3(s−1)/2). Evaluating the integer recurrence at (2n+1,2n+1) for 1≤n≤2,000 gives at most 24,046 moves, including the successful move. The stored grid and local processing take O(n²) space and time.

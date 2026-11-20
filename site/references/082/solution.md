## Overview

Keep two growing paths whose ends each record at most one forbidden vertex among those still present. With s vertices left, query degree s−2. The oracle selects a qualifying vertex v of minimum degree. If it has one non-neighbour, the reply identifies that non-neighbour.

## Strategy and correctness

The complement has at most s−2 edges. A vertex of complement degree one can be removed while preserving this inequality. If all qualifying vertices are universal, defer the returned universal vertex and query degree zero next. This takes a vertex of largest complement degree. When any missing edges remain and none has complement-degree one, its complement degree is at least two, paying for both removals. The same inequality holds for the final pair, which must be adjacent. Append each newly processed vertex to a path whose end permits it, retaining distinct forbidden labels on the two ends. If its unique forbidden label equals one already recorded, put it on that side. A deferred universal vertex can follow the low-degree vertex and clears that end's restriction. Assign the last two vertices to opposite ends, swapping them if necessary, then concatenate the two paths in opposite orders.

## Interaction and local complexity

Exactly n−2 queries remove vertices before the final two are joined. The implementation maintains the remaining labels in an ordered set, using O(n log n) local time and O(n) space.

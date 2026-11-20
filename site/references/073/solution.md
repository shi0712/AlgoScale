# Where Am I Now?

Treat a state as an open cell together with a heading. The target is a cell. Several possible headings can therefore already give a valid answer.

## Which states can be localized?

Translate the current cell to the origin and rotate its heading to face north. The resulting set of open cells in its connected component is the relative map of the state. Equal relative maps give identical observations under every legal action sequence. If two maps differ, trace a path to the first square at which their occupancies differ. Looking toward this square distinguishes the maps. Only rotations and translations preserve the meanings of left and right.

The program canonicalizes each component by comparing all four translated rotations as sorted coordinate lists. A second congruent component prevents localization because corresponding positions always lie in different components. A unique component with no rotational symmetry can be localized after its relative map has been recovered. In a rotationally symmetric unique component, any nontrivial symmetry group contains a half-turn. Its center is the only point fixed by that turn. Localization is possible precisely when this center is an open cell. In that case all rotations share the same center.

## Follow the observations

Maintain four bitsets of candidate cells, one per initial heading. Turning changes the relative heading. A forward move shifts each bitset in its actual direction. Intersect the bitsets with the precomputed masks for the current wall distance. If their union is a single cell, report it. If all remaining components are impossible to localize, report impossibility. Exact equivalence classes always survive or disappear as whole classes. Once only one localizable class remains, either its cell is already unique or the component has an open rotation center. Run breadth-first search on cell-heading states from one representative to the center. The same commands work for all representatives by symmetry.

While multiple classes remain, explore the relative component with a depth-first traversal. A wall observation records the visible ray and its ending wall. At a cell, consider the cyclic ordering north, south, east, west, starting just after the parent direction. Skip explored cells and known walls, look in each remaining direction and enter only if the observed distance is positive. Return on each tree edge. After a complete exploration every surviving state has the same relative map, so one of the termination rules applies.

## Round bound

Let the actual component have $N$ cells and an $h$ by $w$ bounding box. Its neighboring wall cells, including virtual exterior cells, number at most $hw-N+2h+2w$. The first observation already records one wall. Hence at most $F=hw-N+2h+2w-1$ later attempts find a previously unknown wall.

The traversal uses at most $2(N-1)$ forward moves. Without failed probes, the rotations at each vertex cost at most two quarter-turns: enumerate the four possible incoming directions in the indicated opposite-pair order, with a returning child facing opposite its outgoing edge. Skipping children cannot increase this cost. An unsuccessful wall probe adds at most two quarter-turns. Thus exploration takes at most $4N+2F-2$ commands. Including the final report gives at most $4hw+4h+4w-3\le40797$ rounds when center navigation is unnecessary.

If navigation is necessary, take any shortest cell path of length $d$ to the center. Its half-turned image and the path have only the center in common: an intersection at a fixed distance from the center must use the same path index, hence a fixed cell. Therefore $2d+1\le N$. Following the path takes at most $2d+1$ commands, and breadth-first search can only improve this. The total is at most $5hw+4h+4w-3$. Here both bounding dimensions are odd and at most 99, yielding **49,794 rounds**, including the final report.

For $A=rc$ and $L=\max(r,c)$, the local running time is $O(A\log A+L^2+Q(L+\lceil A/64\rceil))$ with the implementation's 64-bit bitsets. Storage is $O(A+L^2+LA/64)$ machine words.

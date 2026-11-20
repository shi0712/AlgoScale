## Convert tree queries into forest counts

If the input graph is already a tree, return all its edges. Otherwise remove vertices in minimum-degree order. Let $d$ be the greatest of these minimum degrees. In the remaining graph at that stage a maximal simple path ends at a vertex with at least $d$ neighbors already on the path. Closing it at its earliest such neighbor produces a cycle of length at least $d+1$.

Extend all but one edge of this cycle into a reference spanning tree $S$. Query $S$ once to get its royal count $b$. For a chord $e$ and an edge $t$ on its fundamental cycle, querying $S-t+e$ reveals $[e\text{ royal}]-[t\text{ royal}]$. Query this difference for every unknown tree edge on the cycle. Any nonzero difference identifies the chord's status and all queried edges. If every difference is zero and all the cycle's edges were unknown, they must all be nonroyal because the royal tree cannot contain the cycle. Otherwise one further exchange using an already known tree edge determines the common status.

Use the closing chord first: this classifies at least $d$ tree edges without any further exchange. Each later extra exchange classifies at least one previously unknown tree edge. Thus at most $2(n-1)-d$ exchanges are needed. Stop as soon as either the known royal or the known nonroyal quota within $S$ is full. Any tree edge still unclassified after inspecting every chord is a bridge, hence royal. Now any forest $F$ can be extended to a spanning tree using edges of $S$. Query it and subtract the known statuses of the added edges. This produces the number of royal edges in $F$ with one legal query.

## Partition and decode

Pad to an even number $2h$ of vertices. The paths starting at $i$ and continuing $i-1,i+1,i-2,i+2,\ldots,i-h$ modulo $2h$ for $0\le i<h$ partition all complete-graph edges. Each edge in path $i$ has endpoint sum $2i-1$ or $2i$ modulo $2h$, and the paths have the required total edge count. Deleting dummy, absent, and already known edges leaves at most $h$ forests. The earlier removal ordering gives an alternative partition into $d$ forests: give all forward edges at each vertex distinct colours. A monochromatic cycle would use two forward edges of its earliest vertex. Take the smaller bound, so there are $f\le\min(d,250)$ nonempty forests. Query each forest count except the last, whose count follows from the total.

Pad each forest with known empty slots to a power of two and maintain pools with known size and royal count. Empty and full pools are resolved. For more than seven royal edges, count the left half and recurse. For at most seven, a finite policy compares the cost of splitting one pool with moves that combine several pools in one query. The union is a subset of the same forest and can therefore be counted legally. For two pools with a single royal edge each, a depth-three decision tree identifies two quarter indices among 16 possibilities. For three such pools, let $x,y,z$ indicate right halves. A count of $x+y+z$ resolves the extremes immediately. In the other cases a count of $x+(1-y)$ identifies $x-y$ and thereby all three bits. Each move strictly refines the unresolved pools. The policy indexes them by sorted height and count, so it is independent of the edge labels. The program returns exactly the edges in resolved full singleton or full larger pools.

## Query certificate

The finite calculation in `admin/tools/query-certificate/certificate.py` verifies every response of the two- and three-pool subroutines and enumerates the worst successor of each permitted action. For a forest padded to 512 slots with one through seven royal edges, it gives bounds $9,13,17,22,26,31,35$. For larger counts, add one query and maximize the two child costs over all splits. Let $P(t)$ maximize these bounds over possible forest heights. To cover every distribution of secrets among $f$ forests use

$$B(f,k)=\max_{0\le t\le k}\bigl(B(f-1,k-t)+P(t)\bigr).$$

If $b=0$, no exchange queries are required and there are at most 499 remaining royal edges. Otherwise, among the as-yet-unclassified edges there are at most 498 royals. The exchange budget is then at most $998-d\le998-f$. Add the initial query, at most $f-1$ forest totals, and the applicable maximum of $B(f,k)$. The maximum for $1\le f\le250$ is **4,346 queries**. This is a certified upper bound. The stored policy in the submitted source is a lossless encoding of the audited 22,758 choices.

## Resources

Building the removal ordering takes $O(n^2+m)$ time, and examining all fundamental paths costs $O(mn)$. A simple conservative bound for completing a forest and printing one query is $O(n^2)$, including the disjoint-set operations. The additional decoder work uses fixed finite policy tables for $n\le500$. The graph, edge table, and these policy tables occupy $O(m+n^2+K)$ space with $K=22,758$. Every query and the final report contain exactly $n-1$ edge labels.

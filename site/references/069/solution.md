## Learn positive entries and gaps

A singleton query returns $\max(a_i,0)$. A string of nonpositive entries between positive blocks matters through its total cost when a subarray crosses the gap. At the edge of a subarray these nonpositive entries can be discarded.

Scan from left to right. After a positive singleton, skip the next position if the position following it still exists. A skipped position $j$ has a known positive left neighbor. If the right neighbor is nonpositive, the query over $[j-1,j+1]$ minus the left neighbor gives $\max(a_j,0)$. If the right neighbor is positive, leave $j$ unresolved. With $s$ such positions the scan has used $n-s$ queries.

## Merge positive blocks

Combine adjacent known positive entries into initial blocks. A block has positive total equal to its maximum subarray sum, hence all prefixes and suffixes of the entire block are nonnegative. Within a suffix, any component can be extended rightwards to the block's right endpoint without losing weight. The analogous property holds for prefixes on the left. These endpoint facts allow the same gap analysis for intervals starting or ending inside a block.

Keep active blocks on a stack in strictly decreasing order of total. Between a left block of sum $x$ and a right block of sum $y$ maintain a lower bound $g$ on the crossing cost and an output position that can absorb extra cost. An all-nonpositive gap starts with bound zero and zero entries. An unresolved singleton starts at the upper bound $10^9$, giving cost bound $-10^9$.

If $g<\min(x,y)$, query the complete two-block interval. Its answer is $\max(x,y,x+y-c)$, where $c$ is the true gap cost. An answer exceeding both block sums identifies $c=x+y-q$ exactly. Decrease the designated output entry by $c-g$, merge the blocks and continue. Otherwise raise the bound to at least $\min(x,y)$ and make the same corresponding adjustment to the output. A preexisting bound this high needs no query. If $x>y$, push the smaller right block and stop. Otherwise pop the left block and absorb it between its protecting gaps.

For the last step, a block of mass $x$ protected by costs $c_L,c_R\ge x$ cannot profitably be crossed from only one side. A subarray crossing both sides uses it in full and has net cost $c_L+c_R-x$. Shifting excess cost between its protecting gaps changes no interval maximum while both bounds stay at least $x$. This also holds for endpoints inside earlier blocks by the prefix/suffix property. By induction, decreases to the designated output entries preserve every interval maximum already determined, and each strict crossing fixes the exact net cost needed by all future merges. The computed array may have different negative entries from the hidden array.

## Query and local complexity

For $R\ge2$ initial positive blocks the stack makes at most $2R-3$ comparisons, since each comparison pops a block or stops the processing of the current right block. Of the $R-1$ gaps, the $R-1-s$ gaps that are not unresolved singletons each require one additional position, either a second gap element or an additional element of the preceding positive block. Thus $n\ge3R-2-s$ and

$$3Q\le3(n-s+2R-3)\le5n-5.$$

For $R\le1$ at most $n$ queries are needed. Thus $Q\le\lfloor5(n-1)/3\rfloor$ for $n\ge3$. The original domain also includes $n=1,2$, where the same implementation asks at most $n$ queries. The scan and amortized stack processing take $O(n)$ time and space. All sums and output values use signed 64-bit arithmetic. The final answer is free.

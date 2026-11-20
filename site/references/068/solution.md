## Scan the other dials

Use the starting value of dial 1 as coordinate zero modulo $n$. Advance this dial exactly $n$ times. The reply at coordinate $v$ indicates whether any other dial occupies $v$. The first dial returns to zero. Let $S$ be the resulting support, $k=|S|$, and $e=n-1-k$ the number of further copies beyond one occurrence of each occupied value.

## Use a unique anchor for large instances

For $n\ge8$, choose a vacant coordinate $x$ minimizing

$$H(x)=x+\sum_{v\in S}d(v,x)+e\max_{v\in S}d(v,x),\qquad d(v,x)=(x-v)\bmod n.$$

Move dial 1 there and keep it fixed. Advance every other dial once in index order and remember its individual reply. Make further sweeps in the same order over the active dials. When a dial's reply first changes, a change from zero to one means it reached $x$, so advance twice more and stop it. A change from one to zero means it reached $x+1$, so advance once more and stop it. Every processed dial stops at $x+2$. Finally advance dial 1 twice.

Before reaching the anchor, active dials keep the same relative order and perform the same one-step cyclic shift per sweep. Their collision feedback at the instant each is tested stays the same. A dial reaching $x$ necessarily has a partner there. On next leaving $x$ for $x+1$, formerly aligned smaller-index dials have already been processed and stopped at $x+2$, and a dial that was initially one step ahead has already been processed too. No stationary dial occupies $x+1$. Thus a baseline zero changes at the anchor, and a baseline one changes one step later. Each ordinary dial advances exactly $d(v,x)+2$ times.

The total is $3n+x+\sum_{i=2}^n d(a_i,x)\le 3n+\min H$. For the original hard budget one can directly use the stronger bound $n^2$ certified by this reference. To see the large-$n$ estimate, list the support coordinates as $b_0<\cdots<b_{k-1}$ and put $B=\sum b_i$. Candidate anchors immediately after a support value, together with zero, give the upper bounds $F_i=n(n-1-i)+nb_i-eb_{i+1}-B$ for $i<k-1$, $F_{k-1}=n+nb_{k-1}-eb_0-B$, and $G=n(n-1)-B-eb_0$. Moving across a consecutive run of occupied positions to the next free anchor does not increase the corresponding bound. Put $r=e/n$. The weights $w_G=1/n$ and $w_i=1/(k+1)+r^{i+1}/(n(1-r^k))$ sum to one and cancel every $b_i$. Their average is at most $n(n-1)-\frac{n}{k+1}(k(k-1)/2+e)$. For $n\ge9$ the last subtracted quantity is at least $2n$. At $n=8$ enumeration of its possible support sets gives a maximum total of 63. The running time and space for this branch are $O(n^2)$ and $O(n)$.

## Synthesize the small-instance strategy

For $3\le n\le7$ enumerate all ordered states consistent with the scanned support, normalized by subtracting the first dial's coordinate. Try each next dial at a search node. Simulate a step on every candidate, normalize again, and partition by the two possible responses. Accept an action only when every nonempty child can finish within the remaining budget. At a singleton leaf, choose a target minimizing the sum of forward distances and move the known dials accordingly.

The search uses $n^2-n$ as the remaining budget after the scan. It prunes on the largest known-state minimum number of physical moves and on the binary information bound. A fixed sequence of advances is a bijection on relative states, so two different relative states cannot both end synchronized along an identical transcript. Memoization stores full candidate sets. Across all support sets the resulting worst-case totals for $n=3,4,5,6,7$ are respectively $7,14,24,36,49$. The small branch performs finite decision-tree search and can take exponential local work. Its metric is the number of dial advances, including the initial scan and final adjustments.

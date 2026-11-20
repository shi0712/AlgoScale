## Recover the path lengths in one collection

Process leaves in increasing order. For each leaf, insert until the first failure. Previously occupied vertices form a connected subtree containing the root. Successful insertions at this leaf occupy a previously unseen chain, from its attachment down to the leaf. Record the chain length. These disjoint chains cover the hidden tree, so their lengths sum to the hidden vertex count.

Give the first eight chains distinct decreasing colors. If there are more leaves, use zero on the others and remove zero-colored entries from this first preorder. At each node, later chains with smaller colors appear before the continuation of its own chain. This identifies the side branches recursively. If there are at most eight leaves, the entire tree is already known.

## Place the remaining long chains

An internal vertex of the known skeleton can be refilled exactly by inserting at a representative descendant leaf after its ancestors have been filled. Keep one representative leaf below each terminal internal vertex. For a skeleton with $d$ long chains, reserve $b=\lceil\sqrt d\rceil$ colors. Distinguish internal siblings by the ordered pair consisting of the sibling's color and the minimum color among its skeleton children.

Here is the coloring rule used in the reference. Work upwards. For each child, group its children by their old colors. Allocate its $c$ classes a consecutive segment $[L,L+c-1]$, with the segments disjoint among siblings. Recolor the classes by their positions modulo $b$ and the child by $\lfloor(L+c-1)/b\rfloor$. The total needed across siblings fits in $b^2$. A segment staying within one row has minimum residue equal to its start. A segment crossing a row boundary has minimum residue zero, and two disjoint segments cannot cross the same boundary. Thus the pair identifies the child. Renaming the old color classes preserves lower-level distinctions.

Use the top $b$ colors of $0,\ldots,B$ on the skeleton. Reserve colors below $f$ for observations of singleton leaves. Identify each new long chain with a different pair $f\le x<B+1-b$, $x<y\le B$, and give it colors $x,y,B,\ldots,B$. Assign pairs in decreasing order of $x$. During preorder parsing, a side chain attached later starts at a color at most $x$. The continuation of the current chain has a larger color. Thus the first larger color identifies $y$, and $(x,y)$ determines the chain and its known length. At a skeleton node all new branches start below the skeleton's color band. Ties between equal-colored children may be resolved in any order.

## Identify singleton parents by subset counts

Once all internal nodes are occupied, a new ball at a singleton leaf stays there. Give groups of leaves distinct colors. In one collection the number of observed balls of a given color under a skeleton node counts that group's selected children. Assign each leaf a binary column. Selecting the leaves whose column has a one in each observation row returns the integer sum of the columns belonging to each parent.

The general code has $F(r)=\sum_{a=1}^{r}\operatorname{popcount}(a)$ columns for $r$ rows. For each integer mask $a$, take every nonempty prefix $R$ of its set bits in increasing position order. Its column is one at row $x$ exactly when $a\setminus R\subseteq x$ and $|R\cap x|$ is odd. The multilinear expansion of this column has largest monomial $a$ with coefficient $(-2)^{|R|-1}$. Processing masks in decreasing order and taking a subset Möbius transform leaves a sum of distinct powers of $-2$ with binary digits. Repeatedly recover the least significant digit modulo two, subtract it, then divide by $-2$. Subtract the decoded columns from the observed vector. The nine-row special code in the source instead enumerates all $2^{16}$ subset sums and checks that they are distinct.

## Resource bound

Write $D$ for the total number of long chains and $s$ for the unresolved singleton count. With $b$ skeleton colors, one discovery round adds up to $T(B-f)-T(b-1)$ chains, where $T(z)=z(z+1)/2$. For each candidate $B=8,\ldots,26$, dynamic programming computes the minimum number of ordinary discovery rounds to reach each number of known chains. The last discovery round can also take one singleton observation with each of its $f$ reserved colors. If $q=B+1-\lceil\sqrt D\rceil$ group colors are available afterwards, $r$ further rounds have capacity

$$qF(r)+\min(q,f)\bigl(F(r+1)-F(r)\bigr)+\max(0,f-q).$$

The planner tests the permitted last-round choices and takes a feasible plan minimizing $K+B$. The finite parameter check uses $1\le D\le200$, $1\le d_0\le\min(8,D)$ and $s\le200-D-8+d_0$. Maximum cost over these 1,544 parameter pairs is 27. For $M\le8$, the initial collection alone suffices. Every collection empties the machine. Each round fills at most $N$ vertices, and only the first fill incurs one extra failure per leaf. There are at most 19 rounds for $M>8$, giving at most $19N+M$ insertion attempts. Planning takes $O(27^2D^2)$ local work. Sorting the skeleton takes $O(N\log N)$ per round, and the exceptional nine-row decoder enumerates at most $2^{16}$ subsets per group.

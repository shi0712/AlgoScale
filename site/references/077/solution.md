## Strategy

Encode species A and B by zero and one. The program first learns individual mushrooms to build pools of known species and then counts the rest in batches. Mushroom 0 supplies the first known A.

Between two known zeroes an unknown bit $v$ contributes $2v$ transitions. Between two known ones it contributes $2(1-v)$. Thus two stretches with opposite anchors reveal an integer sum with positive and negative coefficients. The boundary between those stretches contributes a known constant. Appending a fresh mushroom at the end reveals its bit from the parity of the reply. Removing that final contribution and dividing by two yields the signed sum. The program selects anchors from the available pools without repeating any index in a query.

## Decoding and counting

Suppose $q$ integer row sums determine $f$ binary variables. A larger block uses variables $x,y$ of size $f$ and $q$ extra bits $z_i$. From a pair of rows it obtains $u=(Ax)_i+(Ay)_i$ and $v=(Ax)_i-(Ay)_i+z_i$. The parity of $u+v$ gives $z_i$. Subtraction and division by two recover the two original row sums. Recursive decoding down to a single bit recovers the whole block. The reference also implements a one-anchor-species construction during startup, when a signed row would need too many anchors of the other species. Each matrix row separately identifies the fresh mushroom at the end.

Once enough individual mushrooms are known, alternate a larger known-species pool with a batch of fresh mushrooms. Each interior mushroom of the opposite species contributes two transitions, and the last contributes one if it differs from its anchor. The reply therefore determines both the total number in the batch of the opposite species and the species of its last mushroom. Add that last one to its known pool. Every counted bit is determined by the replies, which also proves correctness against the adaptive judge.

## Query bound

The planner uses states $(k,a)$ with $k\le650$ identified mushrooms, $a$ of them A. If it switches to batch counting, before batch $j+1$ the larger pool contains at least $\max(a,k-a,\lceil(k+j)/2\rceil)$ mushrooms. These batch sizes give a guaranteed finishing budget. Alternatively, for a feasible matrix action consuming $q$ queries and learning $g$ mushrooms, charge $q$ plus the maximum remaining budget among $(k+g,a')$ with $a\le a'\le a+g$. Evaluate states in decreasing $k$ and choose the cheaper feasible action. The same recurrence keeps an upper bound on total query length and checks each action's anchor requirements.

Planning for $n=20{,}000$ covers all inputs with $n>650$. Inputs up to 650 start directly in the batch-counting phase (and $n=3$ has a one-query count). The arithmetic certificate gives **195 queries** and total length at most **52,802**. The upper bound on calls is tight. A fixed 20,000-mushroom assignment, private point 20, chooses maximizing successors in the learning phase and puts each tail endpoint in the smaller pool as needed. It costs 195 calls and 42,114 indices.

## Implementation and resources

Each command prints the query length followed by its distinct labels and flushes before reading the transition count. Only the final total is reported. With the fixed prefix limit $L=650$, planning uses $O(L^2)$ states and a constant list of matrix actions. Each state's scans take $O(n+L)$ time as a conservative bound. The live phase stores the identified indices, the fixed matrices and the largest query, using $O(n+L^2)$ space including the planner. The policy and matrix checks are in `admin/tools/query-certificate/`.

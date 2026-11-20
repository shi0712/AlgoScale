## Strategy

Find the hidden depth D using root-centred balls, then locate a vertex on level D by subtree queries. For a vertex u at depth at most D, the ball centred at u with radius D−depth(u) contains a candidate x on level D exactly when u is its ancestor. This follows from the tree distance formula using the lowest common ancestor. Both stages maintain all positions consistent with the transcript, so the procedure also succeeds against an adaptive judge.

## Capacity on one level

Let C₀=1, C₁=2, and Cq=C(q−1)+C(q−2)+1. A set of m≤Cq candidates on the same level can be searched in at most q queries. Write A=C(q−1). If a subtree has between m−A and A candidates, querying it leaves at most A in either branch. Otherwise descend through subtrees with more than A candidates until neither child qualifies. Then each child and the outside region contain at most m−A−1≤C(q−2) candidates. Querying the two child subtrees identifies one of the three regions in at most two queries. The chosen vertex at the junction is above the candidate level, since a candidate-level subtree contains at most one candidate. This proves the recurrence by induction.

## Sharing the budget between the two stages

For level width a let g(a) be the least q with Cq≥a. Assign this level a dyadic interval of width w=2^g(a), aligned to its own width. Lay these intervals in depth order by rounding the previous right endpoint up to a multiple of w. If the final endpoint is P, put B=ceil(log₂ P). Binary-search the occupied depth intervals in [0,2^B) using root-centred ball queries. Skip branches containing no level. Alignment ensures the level interval is reached after at most B−g(a) actual questions. After reserving g(a) for the within-level step this yields at most B.

All legal level populations start at one, at most double on each successive level, and sum to at most 30,000. A finite dynamic program tracks the maximum aligned endpoint F(t,a) for total population t and last width a. From a state add a level b≤2a, replacing its endpoint by w(b)·(ceil(F(t,a)/w(b))+1). The rounding map is monotone, so smaller endpoints at identical states can be discarded. The maximum over these states is 1,593,674, below 2²¹. The reference therefore uses at most **21 signals**, with zero charge for the final answer. Its online implementation uses O(n) space and O(21n) time for tree traversals and candidate filtering.

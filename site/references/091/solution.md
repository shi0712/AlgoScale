## Overview

Build a complete tournament tree with one applicant per leaf and empty padding leaves. Each internal vertex stores the strongest applicant from its two children. A real comparison is necessary only if both subtrees are nonempty.

## Strategy and correctness

Transitivity makes every stored winner stronger than all other applicants in the same subtree. Repeatedly report the root winner, delete that leaf and recompute only the ancestors of the deletion. The resulting list is the required prefix of the skill order. If n−k<k, reverse the comparisons, extract the weakest n−k instead, and output their complement. When n=k return all applicants at once.

## Interaction and local complexity

Let r=min(k,n−k). If r=0 there are no queries. Otherwise construction costs n−1 comparisons. Each of the r−1 subsequent deletions needs at most ceil(log₂ n)−1 comparisons because the bottom match has an empty side. Thus the bound is n−1+(r−1)(ceil(log₂ n)−1), at most 15,376. Local time is O(n+r log n) and space is O(n).

## Strategy

Work modulo n and call the initial value of wheel 1 zero. Use wheel 1 as a probe. Hold the other wheels fixed and call the set of values they display U. When the probe is at z the response is |U|+1−[z∈U]. Its free initial reading and a sweep through positions 1,…,n−2 identify U. If all n−1 readings are equal, all visited positions belong to U (reply n−1) or all lie outside it (reply 2). For n=3 only, inspect the last position if these cases are indistinguishable. The last membership bit then follows from |U|.

Choose a target h in U. Keep the probe at h or at a symbol outside U so that the set S of all occupied symbols has no nonzero translation period. This is always possible. A set A and A∪{z} cannot both have nonzero periods: a period q of the larger set either fixes the smaller set too, which would fix z, or the difference A∖(A+q) is a singleton. A period of A must preserve that singleton, which forces it to be zero.

## Locating one wheel

At a stage let K=|S|. A candidate (x,s) specifies the wheel's original symbol x∈U and whether it alone displays x among all wheels. At nonzero cumulative displacement d its predicted reply is K−s+1−[x+d∈S]. Emit the difference between successive cumulative displacements. If two candidates share s but have different x, a shift distinguishes them by aperiodicity of S. If they differ in s, summing over all nonzero shifts distinguishes them because each candidate has exactly |S|−1 hits on occupied symbols. The reference chooses a displacement minimizing the largest remaining answer class. Once the candidate is unique, move the wheel to h.

With m unprocessed wheels and b=|U|, discard infeasible candidates using the previously fixed wheels and the minimum multiplicities needed for all b symbols. When b=m+1 the non-target wheels must all be singletons, giving at most m candidates. When b<m, a first probe leaves at most one candidate per symbol and b≤m−1. If b=m initially, the m wheels have distinct values. Later, when b=m, the candidate x=h must be non-singleton. For any different symbol u, aperiodicity supplies a displacement with h+d outside S and u+d inside S. These two symbols cannot survive the same reply, leaving at most m−1 candidates after the first probe. Subsequent probes remove at least one candidate each. Thus locating and aligning this wheel takes at most m actions.

## Bound and termination

Only removing a symbol can create a new period. Move the probe between h and an outside position when needed to restore aperiodicity. These repairs and the final alignment require at most n−2 actions as U shrinks. Initialization uses at most n−1 actions for n≥4. The n=3 sweep may need three, but all its nonempty proper subsets are already aperiodic. The uniform bound is n(n−1)/2+2n−3, at most **1,322 actions**. Stop on the first reply of 1, including after an intermediate probe. There is no separate answer command. Local time is O(n⁴), and auxiliary space is O(n).

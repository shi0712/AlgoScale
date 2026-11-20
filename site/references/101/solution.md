## Overview

Write P(s)=s(s−1)/2. The two reversed blocks have lengths A=j−i and B=k−j+1, each at least two. Their total inversion count is T=P(A)+P(B). Query it once and maintain a bracket [L,R] containing both blocks. For n=4 the triple is known immediately.

## Strategy and correctness

At midpoint t, query the prefix inversion count F(t). If it vanishes retain [t,R]; if it equals T retain [L,t]. Otherwise query F(t−1) and set u=F(t)−F(t−1). In the first block, i=t−u and F(t)=P(u+1). Querying the suffix beginning at i+1 now gives T−(A−1), so both lengths follow. In the second block j=t−u and F(t)=P(A)+P(u+1)>P(u+1), directly yielding A and B. For a bracket of at most ten positions, enumerate every legal triple with the known total. For each permissible interval partition the candidates by inversion count. Try intervals recursively until all branches give singleton candidates; all such brackets have a tree of depth at most two.

## Interaction and local complexity

With n≤10⁹, 27 halving rounds reduce the bracket width to at most eight. An interior reply ends the large-bracket branch with at most two more queries, while the finite terminal tree also costs at most two. The full bound is 1+27+2=30. The implementation uses O(log n) time plus bounded exhaustive work on ten positions, with O(1) auxiliary space as a function of n.

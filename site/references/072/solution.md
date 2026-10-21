Track the number `r` of distinct initial ancestors represented by all current elements. Its transition probability is
`p(r,x)=r!(N-r)! 2^(r-2x) m! / ((r-2x)! x! (m-r+x)! N!)`, where the next state is `r-x`.

Processing states downwards gives their expected total residence times. The probability that an `r`-subset has one initial color is `sum_i C(a_i,r)/C(N,r)`, so summing the residence times weighted by the complementary probability gives the answer. The standard runs in `O(m^2+n)` time and `O(m+n)` space. The new limit is `n <= 2m <= 4000` with total `sum m <= 2000`.

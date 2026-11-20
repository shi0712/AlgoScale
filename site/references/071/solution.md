## Strategy

Process the permutation from left to right. Suppose the relative ranks r₁,…,rₘ of the first m values are known. For the next position let t be the number of old values smaller than the newcomer. For an interval inversion parity I(l,r), one query ending at m+1 supplies

    G(l) = I(l,m+1) XOR I(l,m) = XOR over j=l,…,m of [rⱼ>t].

Maintain I(l,m) locally for all l. The virtual suffix G(m+1) is zero. From G(1)=(m−t) modulo two the first query removes half of the possible insertion ranks.

## Separating insertion ranks

For an old rank v at position j, G(j) XOR G(j+1) equals [v>t]. Choose the median candidate rank v. Two queries thus implement an ordinary median comparison and leave at most ceil(k/2) of k candidates. Either of these binary replies might already achieve that reduction on its own. With two candidates at least one of the two queries separates them. With three, a nonconstant reply leaves at most two.

Define f(1)=0, f(2)=1, f(3)=2 and f(k)=2+f(ceil(k/2)) for k≥4. The preceding construction guarantees completion in f(k) more queries. The implementation also tries individual suffixes and selects a balanced single query if its larger predicted branch b satisfies 1+f(b)≤f(k). Otherwise use the median construction. The suffix predictions are exact for the true t, which therefore survives every filter. After finding t, increment the ranks above t and insert the new value at rank t+1. Update every interval parity by taking suffix XORs of the comparisons rⱼ>t. Induction recovers the whole permutation.

## Resources

To evaluate all possible queries, scan suffix starts from right to left. Including one old rank flips a prefix of the sorted candidate list. The implementation uses 64-bit blocks to count resulting parity classes and caches actual replies. The total query bound is the sum of 1+f(ceil(i/2)) for i=2,…,n, at most **34,887 queries** at n=2,000. The final permutation is free. Local time is O(n² log n+n³/64) with O(n) auxiliary space.

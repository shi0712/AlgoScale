## Overview

Sieve the primes and simulate the set that would remain if no number were protected. Send B for primes in batches of sizes 1414, 4095, 2047, …, 1. A 1 after each batch compares the actual size against the locally simulated one.

## Strategy and correctness

The sets agree until a prime divisor of x is processed. Afterwards the actual set contains precisely one extra element, x. If the first successful batch contains a prime d≤313 dividing x, its multiples have all been deleted apart from x. Thus A(d·p^j) is positive exactly while d·p^j divides x. Starting with d and multiplying once for each positive test recovers all prime powers, including repetitions of d. Without a small divisor, x is prime because every composite up to 100000 has a prime divisor at most 313. For a successful batch, test its primes, inferring the last prime after excluding the others. If all batches fail then x=1.

## Interaction and local complexity

There are 9,592 primes through 100,000. At $n=100000,x=1$ the program sends 9,592 deletions, 13 size checks and one final C, totalling 9,606. For batch $j$ (numbered from one) with $d$ members and $b$ earlier primes, a prime branch costs at most $b+2d+j$, including C. It pays $b+d$ deletions, $j$ size checks, at most $d-1$ individual tests, and the final answer. Substitution of the untruncated batch sizes gives 2,829 for the first batch and 9,606 for each later one. Truncating the last batch only reduces the bound. A small divisor is found in the first batch. There are 65 primes through 313 and 5,133 through 50,000. Among pairs $(p,e)$ with $p$ prime and $e\ge2$, exactly 84 satisfy $p^e\le50000$. Even charging every such power test gives at most $1414+1+65+5133+84+1=6698$ operations on this branch. The exact full-domain worst case of this implementation is therefore 9,606 operations. Official point 20 is the stated equality witness. The sieve and all local deletion scans take $O(n\log\log n)$ total time and $O(n)$ memory.

## Overview

Maintain the cumulative amount spent and the known low bits of the remaining number. The initial response is its popcount. With p unresolved unit bits in w positions, p=0 or p=w is immediate. For p≤6 subtract a power of two at the current search position. A borrow through z zeros increases popcount by z−1; the response therefore locates the next unit bit exactly.

## Strategy and correctness

For a dense suffix, resolve six low bits at once. Candidate states record the six-bit residue d and the possible popcount change gamma of borrowing once from the upper suffix. After cumulative probe x the predicted change is wt((d−x) mod 64)−wt(d)+[d<x]gamma. The tree in std.cpp splits states by this response and stops only when their residues agree. Dense suffixes guarantee enough value to pay the probes. After two such blocks, the residual 18-bit problem is solved by an embedded decision tree, grouped by initial popcount. Its decoder regenerates every possible residual value, checks every split, disallows overdrafts, and rejects a non-singleton leaf. Adding back all subtractions recovers the original number.

## Interaction and local complexity

Each six-bit tree has depth at most three. Two blocks plus the exhaustive 18-bit tree cost at most 3+3+6=12 subtractions. If the sparse case is entered earlier, it uses at most six further subtractions. Secret 260128 attains 12. Preprocessing enumerates at most 2¹⁸ residual values per complete level; the interactive phase uses only the decoded tables.

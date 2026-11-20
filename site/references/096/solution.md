## Overview

Compare 21 items with the first item in an anchor group of 22. The larger equality class is genuine, because there are only ten fakes. Split the remaining 978 items into 196 triples and 78 groups of five. Compare each item to its group's first item.

## Strategy and correctness

Orient every mixed group with one comparison to a known genuine item. A monochromatic group then contributes either its full size or zero fakes. If h fakes remain there, 3a+5b=h≤10 has at most one nonnegative solution. Search only the representatives of the pure groups. For one fake, test disjoint pairs: the unequal pair contains the fake, and one anchor comparison resolves it. For two fakes, make triples, read leftovers directly and orient the mixed triples by the remaining count. Two mixed triples must each contain one fake; a single mixed triple has its count known. For three fakes, choose numbers of pairs and triples by minimizing the worst remaining work; 2a+3b≤3 again has unique feasible counts. Transfer every representative's classification back to its group. All deductions hold for any transcript consistent with ten fakes.

## Interaction and local complexity

The two initial partition stages use 21+196·2+78·4=725 queries. When h=0,3,5,6,8,9,10, the maximum further costs including mixed groups are respectively 10,105,44,135,139,139,52. For the three-fake case the bound on t representatives is the minimum of t−a−b+max(3,1+ceil(a/2),ceil(b/2)) over 2a+3b≤t. This gives 864 queries overall. The arithmetic search takes O(n²) local time, with O(n) stored items.

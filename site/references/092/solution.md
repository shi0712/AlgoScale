## Overview

Subtract one from every hidden value. Recover the permutation by splitting its positions into residue classes modulo 2,4,8,… . At a level with modulus d, choose two representatives from each of the d classes. Their total is divisible by d, and an integer-average query on the 2d positions reads one bit about the next binary digits. Replacing a pair in one or more classes exposes the XOR of the changes in their parity disagreements.

## Strategy and correctness

First learn each class's unordered binary partition relative to a fixed pivot. A shared baseline works across all groups; known group sizes force the unused positions once one child is full. For small balanced classes encode several groups together, enumerate their possible partitions and choose the pair replacements that minimize the largest surviving branch. The program keeps the unique state consistent with every answer. Then orient the partitions from the root down. In an odd-sized class the larger child starts at the smaller residue. For an even parent of size 2h, choose another 2h-element class whose residue differs modulo three. Query one h-element child and the entire partner. The two possible average numerators after division by h differ by d and exactly one is divisible by three. At the root the two global orientations are reflections; the promise about the first position selects one.

## Interaction and local complexity

Group sizes at every level depend only on n. The certificate admin/tools/certify_112.py enumerates the local decision trees, the group-size costs and each orientation check. Its maximum is 6,318 partition queries and 478 orientation queries at n=800, hence 6,796 in total. With the fixed small-group tables treated as constants, O(n² log n) local time plus the finite decision-table construction and O(n log n) group storage suffice.

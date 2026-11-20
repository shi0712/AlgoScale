## Overview

Work from the largest values downwards. Before processing a block there are two candidate positions left over from the preceding block, already in increasing value order. Append up to three preceding positions. Their final relative order determines the next largest values and the next two leftovers.

## Strategy and correctness

Value v can finish no earlier than v−2: before step v−2 it cannot have been touched. Accordingly, among the k+2 local labels, the j-th largest surviving value can only be in the old pair or among the first j newly admitted positions. Enumerate the permutations satisfying this restriction and the known old-pair comparison. For every possible comparison split the permutations into the two possible answer sets. A leaf is valid only when it has one order. The recursive builder accepts a comparison only after both answer subtrees are complete. Thus the true order remains present for every legal comparison reply, including adaptive replies. Assign the k highest labels and keep the smallest two, restoring the invariant.

## Interaction and local complexity

At most one initial comparison is needed. The finite search enumerates at most 5! local orders and constructs depth 5 for k=3 and depth at most 2k for k=1,2. With n−2=3h+r, the bound is 1+5h+2r, and n=1 costs zero. This is at most 49998 over n≤30000 and below the instance limit. After the constant-size trees have been built, local work and storage are O(n).

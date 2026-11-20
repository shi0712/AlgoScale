## Overview

Sort the indices by Ford–Johnson merge insertion. Compare pairs, recursively sort the winners, and insert paired losers below their winners in reverse Jacobsthal groups. Use this order to safely sum the board without ever asking the numeric value of an entry.

## Strategy and correctness

For an even-sized sorted group merge its two ends and insert the result in sorted order; their sum is safe whenever the group's total is safe. Indeed, if both ends are positive the sum is at most the group total, if both are negative it is at least the group total, and opposite signs keep it between the ends. For odd size split into alternating positions. The alternating difference of the ordered list lies between its extremes by pairing adjacent differences. If S is the total and D this difference, the two alternating group sums are (S+D)/2 and (S−D)/2. Each lies within the limit because both S and D lie within it. Their final addition returns S. Continue until exactly one value remains.

## Interaction and local complexity

For sorting, the paired winners cost floor(n/2) plus the recursive sort of that many items. A Jacobsthal group ending at last after prev inserts pays at most ceil(log₂(last+prev)) comparisons per loser. For subsequent comparisons the recurrence is R(n)=R(floor(n/2))+R(ceil(n/2)) for odd n and R(n)=ceil(log₂(n−1))+R(n−1) for even n. Add exactly n−1 additions. The certificate in the reference enumerates n≤1000, maximizing at 8641+743+999=10383. All zeros attain it. The vector insertions and copies require O(n²) local time and O(n) working storage along the recursion.

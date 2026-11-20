## Overview

Probe powers of two until the threshold lies in [base,2base−1], clipped to 10¹⁴. The initial balance after the successful doubling prefix is enough for the next test. For very small base, find the answer sequentially, replenishing the balance with a known-safe withdrawal when necessary.

## Strategy and correctness

For larger base, take units of base/8 and use the 11-unit pivot to bound the gain on success and maximum loss on failure. Let C(s,m) be the number of consecutive thresholds distinguishable using s more operations and at least m units of capital. A safe operation can always replenish. With an affordable risk the two child capacities add: success gives at least m+gain, failure leaves at least m−loss. The coded base cases return capacity one at zero steps and 2^s when all remaining worst-case losses are affordable. At each step match the lower interval size to the failure child's capacity and use the rest for success. This preserves both interval coverage and the lower balance invariant. All returned guesses are therefore legal and uniquely determined.

## Interaction and local complexity

The finite capacity recurrence and the initial power-of-two bracket bound the worst case by 99 queries over 0≤M≤10¹⁴; the balance certificate in admin/tools checks its branches. A fixed threshold 48163410256760 attains the bound. Each decision uses O(1) arithmetic plus memoized capacity lookups; preprocessing is bounded by the reachable triples of remaining steps and integer balance for each gain/loss pair.

For a fixed maximum height `x`, the minimum number of horizontal and vertical strips is
`ceil(h/x) + ceil(w/floor(s/x))`. A continuous lower bound confines the relevant integer values to an interval around the continuous optimum. Binary searches find the interval endpoints; only `O(s^(1/4))` values are evaluated.

The standard runs in `O(s^(1/4) + log s)` time and `O(1)` extra space. The original solution scans `x = 1..min(h,s)` and is `O(s^(1/2))`; the new limit is `h,w,s <= 10^18`.

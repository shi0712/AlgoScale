## Overview

For each possible absentee keep its last truth value and whether its current run has length one or two. Every reply uniquely implies truth or dishonesty under a given absentee. Discard candidates that would create a run of length three. Two surviving candidates can be covered by the two free guesses.

## Strategy and correctness

The first phase weights candidates by the number of admissible future reply strings. These counts are Fibonacci numbers for the two possible run lengths. Scan prefix intervals and choose the one minimizing the absolute difference in successor weights. The possible imbalance changes by one candidate's contribution when the cut moves, giving the rounded weight bound implemented in survivors(n,q). The parameters table chooses a phase length that leaves at most eight candidates. In the tail, encode their ordered truth states in base four. Enumerate all contiguous slices and both successors, then fill a state at depth d only when both nonempty children can terminate by depth d−1. Empty gaps between surviving labels do not affect the meaning of a contiguous query. The actual absentee always survives both phases.

## Interaction and local complexity

For every n≤100000, the weighted bound plus the exact small-state tail table guarantees at most 56 interval queries. The exhaustive state-count certificate tightens this program to 55; the packaged fixed truth trace at n=100000 attains it. Guesses and the closing # are free. The prefix phase scans O(n) state per round, and the tail has a fixed precomputed table for at most eight candidates. All contestant outputs are flushed, including # after the successful guess.

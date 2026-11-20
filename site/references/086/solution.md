## Overview

Ask the sum S of the entire array. An active block of length at least four has a unique boundary where its prefix sum is S/2. Locate this boundary by binary search and recurse on the shorter of the two halves, keeping S/2 as the known sum.

## Strategy and correctness

Trace the most recent ancestral concatenation. The two pieces started as equal copies and subsequent operations on a piece split a current maximum into equal parts. Starting from the same multiset, the number of splits determines the resulting multiset, regardless of tie choices. The shorter piece has undergone fewer splits and hence has a maximum at least as large as the longer one's. If there was no concatenation, use the first split as the two-piece ancestor; any globally maximal split within a piece also splits its local maximum. This argument applies again inside each piece. At length one return S. At lengths two or three at least one half is unsplit and has value S/2, which is the maximum.

## Interaction and local complexity

At length s≥4 a binary search over s−1 candidate boundaries costs at most floor(log₂(s−1)) interval-sum queries; an unqueried last candidate is known. The next length is at most floor(s/2). Including the initial sum gives at most 136 for n≤100,000. Local time is O(log² n) and storage is O(1).

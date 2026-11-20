## Overview

Inspect disjoint triples until an edge with gcd d>1 is found. Three pairwise gcds of one exclude zero from that triple, since a zero would force both other values to equal one. Keep the two ends a,b of the successful edge as candidates, together with all unprocessed positions.

## Strategy and correctness

Both anchors are multiples of d. For a fresh i, write g=gcd(p[a],p[i]). If d does not divide g, i cannot be zero. If g is a proper multiple of d, b cannot be zero, since otherwise p[a]=d. Replace b by i. Otherwise g=d, in which case a cannot be zero: this would force p[b]=p[i]=d. Replace (a,b) by (b,i), and query their gcd when another update is needed. The new gcd is always a multiple of the first divisor D. Thus gcd(p[a],p[i]) detects membership of i among the original D-multiples throughout the scan. Stop as soon as all such multiples have been seen, or the current divisor has only one positive multiple in the permutation.

## Interaction and local complexity

Each initially rejected triple costs at most its size. The scan uses at most one ordinary query per position and extra queries only on hits among the at most ceil(n/2) multiples of D, counting zero. The first two are anchors and the last hit needs no refresh. For n≥4 the cost is at most n+max(0,ceil(n/2)−3). The maximum 29,997 is attained at n=20,000. Cases n=2,3 use zero and two queries. Local time is O(n) and extra space is O(1).

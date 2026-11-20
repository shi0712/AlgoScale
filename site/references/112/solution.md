## Overview

Triangles and point sets in strictly convex position determine their area from their hull. Otherwise let L(x) be the total vertical intersection length. Between two adjacent vertex x-coordinates the crossed edges keep the same order and each crossing height is affine, hence L is affine and its integral is exactly the slab width times the midpoint value.

## Strategy and correctness

A column containing only one vertex has matching one-sided limits and can be used as a trapezoid node. At repeated interior columns split the quadrature into separate runs, using interior evaluations for endpoint limits. In one run two neighboring node weights a,b can be merged: an affine function satisfies aL(x)+bL(y)=(a+b)L((ax+by)/(a+b)). If the denominator exceeds the protocol bound, use two interior trisectors with adjusted weights, or the simpler midpoint formula, whichever needs fewer queries. Each formula integrates exactly twice the area. The reference reduces replies modulo 1000000007; denominators are products of edge horizontal differences and query denominators ≤1000 and are invertible, while twice the area is below this modulus. Reporting the residue therefore recovers the actual area.

## Interaction and local complexity

Let d be the number of repeated internal columns, s the number of singleton internal columns and p=d+1. Then s≤n−2d−2. Across runs the query count is at most p+min(s,floor((s+p)/2)) when the column count exceeds 502; in that range the total unused x-span ensures every combined denominator can fit. With at most 502 columns the fallback uses at most 501 midpoints. Maximizing over integer n≤1000 gives at most 666 calls. The alternating single/double-column witness reaches 666. Sort and build the hull in O(n log n) time and store O(n) samples, processing rational replies as decimal strings.

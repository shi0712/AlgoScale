## Overview

Recursively group the left and right halves, always starting and finishing with an empty conveyor. In each half choose one representative per class. Enqueue the left representatives and then the right ones. A right insertion with no count increase identifies an overlapping class. Dequeue the left representatives; the ones whose removal does not decrease the count give the matching left classes.

## Strategy and correctness

The overlapping t left classes and t right classes are in bijection. Give the left classes distinct binary codes, preferring minimum Hamming weight. For each pair of code bits, enqueue the left representatives bearing the first bit, insert the first t−1 right representatives, then insert the left representatives bearing the second bit. Changes on insertion/deletion reveal the two bits of each right representative's mate. After draining the queue the unused mate identifies the last match. Recursion never merges two different classes, and the bijection merges every equality across the split.

## Interaction and local complexity

Let D(n,k) be the maximum cost to classify n minerals in k classes. For halves with l and r classes, t=l+r−k are shared. The merge costs 2(l+r)+2W(t)+(t−1)b, where b is the smallest even bit length with 2^b≥t and W(t) is the sum of the t smallest codeword weights; the last two terms vanish for t≤1. Maximize over feasible l,r,t and add the two child costs. The archived queue_bound.cpp evaluates all n≤500 and yields 13612 at n=500, k=125. Any chosen overlap t is attainable by relabeling the two half partitions. The contestant stores O(n log n) labels across the recursion, and its bookkeeping plus all queried operations is polynomial in n.

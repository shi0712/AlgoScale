## Overview

Root the tree at 1. First identify depths using blocks of w consecutive depth values. Start with w a power of two covering the tree. Select the blocks in one residue class modulo three as light sources of radius w/2. For the following block, the nearer half has a lit ancestor and the farther half is too far from every source. Cycle through the residue classes, treating the root as the preceding virtual boundary, and halve w.

## Strategy and correctness

In the final w=2 experiments each radius is one. For a vertex, the experiment in which its possible children emit light differs from that for its parent. Since a source cannot light itself, that observation also tells whether the vertex has any children. In each depth layer number just the I_d vertices known to be parents using b_d=ceil(log₂ I_d) bits, with zero bits for at most one parent. A child reads a parent's selected bit when those parents emit at radius one, provided the layer two below them is silent. Give alternating layers d and d+2 respectively the earliest and latest slots among max_d(b_d+b_{d+2}) rounds. This decodes the unique parent of every nonroot vertex.

## Interaction and local complexity

For maximum depth D there are E=n−D−1 extra vertices beyond a path. Requiring b parent bits at a layer costs at least 2·2^(b−1) extras across it and its child layer for b>0. Conflicting senders occupy disjoint layer pairs. The depth experiments cost at most min(3,ceil(D/w)) per w>2 and three at w=2. Maximizing their sum with the slot count using admin/tools/certify.py gives 42 experiments for n≤1,000. Local time is O(n log n) and storage O(n).

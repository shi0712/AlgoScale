## Overview

Write H=100000. Scan the slightly tilted segments (s,0)–(s+1,H) for s=199+200i, i=0,…,497. A disk with integer horizontal center from s−99 to s+100 and radius at least 100 yields a detectable chord. If all scans miss, use s=99800 in the remaining strip.

## Strategy and correctness

Query the two vertical lines x=s and x=s+1. Round one quarter of each squared length to integers a,b; the allowed measurement error changes these values by less than 0.051. For a detected intersection the identities a=r²−(c−s)² and b=r²−(c−s−1)² give the horizontal center c=(b−a+2s+1)/2 and then r. A missing adjacent vertical chord in this reconstruction is tangent; in the final strip the only double miss is c=99700,r=100. To recover vertical center y, let u=floor(r/3), v=floor(2r/3), and measure (c+u,0)–(c+v,H). Its distance to the center is (Hu+(v−u)y)/sqrt(H²+(v−u)²). The resulting chord decreases strictly with y; neighboring integer centers change the chord by more than 10^-4, exceeding twice the error. Binary search and choose the nearest predicted chord.

## Interaction and local complexity

All probing segments cross the box from bottom to top, so the entire circle chord is measured. The scan and reconstruction use at most 498+2+1=501 probes; c=99700,y=50000,r=100 takes exactly 501. Local work is O(498+log H) and space is O(1).

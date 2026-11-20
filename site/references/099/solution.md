## Overview

Query all ten triples of the first five positions. For each position total the replies from triples containing it. Sorting these totals sorts the five hidden values. To see this, pair terms involving only one of two positions by keeping the other two members fixed. The function min+max is monotone, and for unequal values at least one pair among the three common companions makes the inequality strict.

## Strategy and correctness

For sorted first values a≤b≤c≤d≤e, the stored replies give a+c, a+e and c+e from triples (a,b,c), (a,b,e) and (c,d,e). Recover a, then c and e, then b and d from two further stored replies. For a new x choose a closest adjacent known pair u≤v. Query (u,v,x). If the reply differs from u+v, the known endpoint at the opposite side gives x immediately. If u=v, the reply also directly determines x. Otherwise x lies in [u,v]. Two known positions on one outside side exist among the five or more known values and their triple with x yields x. Insert it in sorted order.

## Interaction and local complexity

After every extra query the minimum gap becomes at most half its previous value or becomes zero, after which no extra queries are needed. Initially the minimum gap is at most 250,000,000. At most 28 extras are possible, giving 10+(n−5)+28=n+33, at most 1,033. The sorted vector takes O(n²) time and O(n) space overall.

## Find the radius

Cache each unordered pair queried. A farthest leaf $a$ from town 0 is a diameter endpoint of the positively weighted tree. A farthest leaf $b$ from $a$ gives a diameter of length $D$. The two sweeps use at most $2N-3$ different distances.

Project each town $i$ onto the path from $a$ to town 0. Its attachment point is at distance

$$h(i)=\frac{d(a,i)+d(a,0)-d(0,i)}{2}$$

from $a$, using only cached distances. The point at $h(b)$ is where the diameter departs from that path. Because $a$ is farthest from 0, $h(b)\ge D/2$. Along the common part of the two paths, a city at coordinate $h$ has eccentricity $\max(h,D-h)$: the farther of $a,b$ is farthest among all towns. Every interior city on that common part has a side branch containing a town, and hence occurs as an attachment coordinate. A city away from the diameter cannot improve the minimum. Beyond the junction all cities are farther from $a$ than the junction itself. If the common path ends at town 0, all interior diameter cities already occur as attachment coordinates. Scanning the eligible coordinates gives the minimum eccentricity $R$ and its possible hubs.

## Test balance

Process groups of equal $h$ in increasing order. If either the number on the left or that on the right exceeds $T=\lfloor N/2\rfloor$, the candidate hub is unbalanced. If all three counts (left, right, and same-coordinate group) are at most $T$, it is balanced. In the remaining case only the group attached at the candidate can hide an oversized component. There can be at most one such group of size $s>T$.

Two towns $i,j$ in the group belong to the same branch off the hub exactly when

$$d(i,j)<d(a,i)+d(a,j)-2h.$$

Run majority cancellation with this equivalence test. Start a block when the balance is zero, increase the balance for another town in the leader's branch, and decrease it for a town in another branch. Each completed block contains equally many leader-branch and other-branch towns. If the last block leaves balance $r$, no branch can occur more than $(s+r)/2$ times: every cancelled pair contains two different branches. If this already implies balance, stop. Otherwise only the last leader can be a majority. In a completed block whose leader matches, add its leader count directly. In a block whose leader differs, only its nonleader list must be checked individually. Count the last block's leader entries as well. A count over $T$ makes this hub unbalanced. Return $R$ if any candidate balances and $-R$ otherwise.

## Query bound and witness

For $p$ blocks the cancellation phase uses $s-p$ equivalence tests. If a second phase is needed, $s+r>2T$ and $s+r$ is even, so $r\ge2T-s+2$. If the last block has length $\ell\ge r$, verification costs at most $p-1+(s-\ell)/2$ more tests. Together the phases cost at most $2s-T-2$. Without the second phase at most $s-1$ are needed. An interior attachment coordinate excludes both town $a$ and town 0, so $s\le N-2$. Adding the first $2N-3$ calls gives at most **376 distance requests** over $6\le N\le110$, and satisfies the per-size bound $\lceil7N/2\rceil$.

At private point 20, towns 0 and 1 attach with weight 4 to city $H$. A second city $V$ is adjacent to $H$ with weight 1. Towns 2 through 105 attach alternately to $H$ and $V$, and towns 106 through 109 all attach to $V$. The 56-town branch makes the signed answer $-4$. The reference uses $217+55+104=376$ requests, proving tightness for this implementation.

## Implementation

The distance matrix uses $O(N^2)$ space. Grouping projections in an ordered map costs $O(N\log N)$. Including initialization of the cache, the local computation is $O(N^2)$. The program outputs one signed integer after finishing its requests and does not await a further response.

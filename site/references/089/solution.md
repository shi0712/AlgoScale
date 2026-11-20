## Overview

Build a list of ordered menus with the following property: every pair of shops occurs at distance at most k in at least one menu. Reset the tasting history before each menu. Visit the listed shops that are still live. On a positive familiarity reply discard the current shop.

## Strategy and correctness

For any flavour, its last live representative cannot be discarded, since a positive reply requires an earlier shop of that flavour and history is reset between menus. Inside one menu every shop occurs just once; tracing equal-flavour witnesses backwards within that menu ends at a representative that was retained. If two shops of one flavour survive all menus, their covering menu visits both, possibly even closer after skipped shops. The later shop would then have been discarded. The number of survivors is therefore the number of flavours. To construct the menus, partition shops into blocks of size max(1,k/2). A Walecki path decomposition puts every pair of blocks consecutively on some path. For the largest size ratios the source also tries explicit cyclic and packed menus and prunes visits only while preserving complete pair coverage.

## Interaction and local complexity

The number of tastings is bounded by the total menu length, attained when all flavours differ. The certificate admin/tools/exact_coffee.cpp checks every legal power-of-two parameter pair, every shop pair, the reset limit and the aggregate length. The largest menu cost is 6,423. Resets and the final answer are free. For total initial plan length P, the trimming loop has at most P successful deletions, with O(P²(k+n)) as a conservative local work bound and O(n²+P) space.

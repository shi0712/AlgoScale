## Overview

Before playing, compute a finite decision tree over the six fixed deterministic opponents. A state records the current round, surviving opponents, previous player move and length of the current winning streak. For each candidate R/P/S move, simulate every surviving rule to split the mask into the win and nonwin branches.

This bound uses the six programs in the supplied judge. Three always play the same move (rock, paper or scissors). One cycles rock, paper, scissors. The last two start with rock and then either copy the player's previous move or play the move that beats it. The public task withholds these rules. A bound obtained by enumerating this pool applies to exactly these six opponents.

## Strategy and correctness

A streak of ten succeeds immediately. A state fails if even winning every remaining round cannot reach ten. A move is safe exactly when every nonempty child is winning. The recursion decreases the rounds remaining, so backward induction proves the result for the entire transcript. During play follow the stored move and discard opponents inconsistent with the verdict. Ties and losses both follow the nonwin child.

## Interaction and local complexity

Solve from horizons 10 upward. Running the reference executable with `--certificate` reports that the root is losing at 10, 11, 12 and 13, then winning at 14. The search examines all three moves and every nonempty response branch, giving both the lower and upper bounds of 14 rounds for the fixed six-rule pool. At horizon H the table has at most (H+1)·64·3·11 entries with constant branching. In live play each transition and storage lookup takes constant time.

## Overview

Process left endpoints i in increasing order. At the start of a stage every interval starting earlier is off. Cell i can therefore be covered only by the k=n−i+1 intervals that start at i.

## Strategy and correctness

Traverse the reflected binary Gray code on these k switches. In transition j flip the switch whose bit is the least significant set bit of j. The singleton [i,i] is bit zero. A change in the visible-cell count caused by that switch proves that every other interval in its group is off. Leave the singleton off, undoing its last flip if visibility decreased. Every configuration occurs in the Gray traversal; the configurations with all other switches off form one consecutive pair joined by the singleton flip. If the very last state is reached with that pair already visited, the stage has already ended. Stop immediately on receiving n.

## Interaction and local complexity

A stage costs at most 2^k−1 Gray transitions and one restoring flip. The final group costs at most one move because its only active interval must be on if the game is still running. For n=10 the sum 2^10+…+2²+1 equals 2045. The witness enabling every [i,i] and [i,n] attains it. Local work is constant per move and memory is O(1).

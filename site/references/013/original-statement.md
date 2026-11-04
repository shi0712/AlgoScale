# 13. Play It by Ear

## Problem Description

A deck contains $2n$ distinct cards numbered $1$ through $2n$. Initially it is shuffled uniformly over all $(2n)!$ orders. You receive its top $n$ cards as your hand; the order of the remaining $n$ cards is hidden.

On each turn, choose a card from your hand, put it at the bottom of the deck, and then draw the top card. Your hand therefore always contains $n$ cards.

Complete $m$ quests in order. Quest $i$ requires playing card $a_i$. Only the first unfinished quest is active: playing $a_i$ before quest $i$ becomes active gives no credit for that quest. The entire sequence $a_1,\ldots,a_m$ is known in advance, and card labels may repeat.

Find the smallest expected number of turns achievable by a pure strategy. A pure strategy deterministically chooses a card from the observable history: the initial hand, cards previously played and drawn, and quest progress. It cannot depend on the unknown deck order. The expectation is over the random initial shuffle. Return the result modulo $998244353$.

## Input Format

Read from standard input.

Read $T$. Each test case consists of a line containing $n,m$, followed by a line containing $a_1,\ldots,a_m$.

## Output Format

Write to standard output.

For each case, write one integer $r\in[0,998244353)$ representing the minimum expectation. If that expectation is the reduced fraction $p/q$, then

$$rq\equiv p\pmod{998244353}.$$

This modular representation is guaranteed to exist and be unique.

## Examples

### Example

Input:

```text
1
2 1
1
```

Output:

```text
249561090
```

## Explanation / Notes

In the example, play card 1 immediately if it is initially in the hand. Otherwise play other cards until drawing card 1, then play it on the next turn. The optimal expectation is

$$\frac12\cdot1+\frac14\cdot2+\frac14\cdot3=\frac74.$$

This is a standard computation problem: the strategy is part of the mathematical definition; the submitted program does not interact with a card-game judge.

## Constraints

- $1\le T\le1000$.
- $2\le n\le5000$ per case.
- $1\le m\le2\times10^5$ per case.
- $1\le a_i\le2n$; repeated quest labels are allowed.
- Across all cases, $\sum n\le5000$ and $\sum m\le2\times10^5$.
- All $2n$ cards are distinct; all initial deck permutations are equally likely.
- Modulus: $998244353$.
- Time limit: 1 second; memory limit: 1024 MB.

## Problem Description

The training team has $m$ candidate players. You need to select $2n$ players, pair them up, and assign the resulting pairs to $n$ tennis doubles matches.

Player $i$ has an estimated ability $e_i$ and is classified as either a junior or a senior according to their training experience.

Match $i$ requires the ability of each participating player not to exceed $l_i$. For stability, the absolute difference between the abilities of the two players in every pair must not exceed $d$.

Players in a pair do not need to belong to the same category. To make full use of the potential of junior players, the coach wants to know the best possible lineup for every exact number of selected juniors.

For each $t=0,1,\ldots,2n$, determine whether a valid pairing and assignment exists that contains exactly $t$ junior players. If it does not exist, the answer is $-1$. Otherwise, output the maximum possible sum of the abilities of the selected $2n$ players.

## Input Format

Read from standard input.

The first line contains an integer $T$, the number of test cases.

For each test case:

- The first line contains three integers $n$, $m$, and $d$.
- The second line contains $n$ integers $l_1,l_2,\ldots,l_n$.
- Each of the following $m$ lines contains two integers $e_i$ and $t_i$. Here, $e_i$ is the estimated ability of player $i$; $t_i=1$ denotes a junior, and $t_i=2$ denotes a senior.

## Output Format

Write to standard output.

For each test case, output $2n+1$ integers on one line. The $(i+1)$-st integer is the answer when exactly $i$ junior players are selected, for every $i=0,1,\ldots,2n$.

## Examples

Input:

```text
2
4 9 400
800 900 1050 1200
46 1
264 2
295 1
305 1
332 2
678 1
770 2
903 2
1291 2
4 9 400
800 900 1050 1200
46 1
264 2
295 1
305 2
332 2
678 2
770 2
903 2
1291 1
```

Output:

```text
-1 -1 -1 -1 3593 -1 -1 -1 -1
-1 -1 3593 -1 -1 -1 -1 -1 -1
```

## Constraints

- $1\le T\le10^5$.
- $1\le n\le20$.
- $2n\le m\le10^5$.
- $0\le d<10^9$.
- $1\le l_i\le10^9$.
- $1\le e_i\le10^9$.
- $t_i\in\{1,2\}$.
- The sum of $m$ over all test cases does not exceed $2\times10^5$.

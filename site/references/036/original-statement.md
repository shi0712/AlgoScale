## Problem Description

There are $n$ original cities and $m$ damaged undirected roads. Road $i$ joins $u_i$ and $v_i$ and costs $w_i$ to repair. Before the damage, these roads connected all the original cities.

There are also $k$ optional towns. Activating town $j$ costs $c_j$. Once activated, it may be connected to any original city $i$ by building a road of cost $a_{j,i}$. Any number of towns, including none, may be activated.

Choose repairs, town activations, and new roads of minimum total cost so that every pair of **original cities** is connected. A route may pass through activated towns. New roads described by the input connect a town to an original city; no town-to-town road option is given.

## Input Format

Read from standard input.

There is one instance.

1. The first line contains $n,m,k$.
2. Each of the next $m$ lines contains $u_i,v_i,w_i$.
3. Each of the next $k$ lines contains $c_j,a_{j,1},a_{j,2},\ldots,a_{j,n}$ for one town.

## Output Format

Write to standard output.

Print one nonnegative integer: the minimum total cost needed to connect all the original cities.

## Examples

Input:

```text
4 4 2
1 4 6
2 3 7
4 2 5
4 3 4
1 1 8 2 4
100 1 3 2 4
```

Output:

```text
13
```

## Explanation / Notes

In the example, repair roads 3 and 4 for costs 5 and 4. Activate town 1 for cost 1, then connect it to original cities 1 and 3 for costs 1 and 2. The total is $5+4+1+1+2=13$, which is optimal.

## Constraints

- $1\le n\le10^4$.
- $1\le m\le10^6$.
- $0\le k\le10$.
- Every old road satisfies $1\le u_i,v_i\le n$, $u_i\ne v_i$, and $0\le w_i\le10^9$.
- Every town activation cost satisfies $0\le c_j\le10^9$.
- Every possible town-to-city construction cost satisfies $0\le a_{j,i}\le10^9$.
- All input values are integers.
- The original road network is connected. Self-loops are forbidden; multiple roads may join the same pair of cities.
- Zero-cost repairs, activations, and new roads are allowed.
- Time limit: 2,000 ms.
- Memory limit: 524,288 KB.

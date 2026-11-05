## Problem Description

A rectangular sheet has integer height $h$ and width $w$, subdivided into unit cells. A cut is made along an entire horizontal or vertical grid line and cuts every existing piece intersected by that line.

A horizontal cut may use an integer coordinate from $1$ to $h-1$, and a vertical cut may use one from $1$ to $w-1$. Thus $c_h$ distinct horizontal cuts and $c_w$ distinct vertical cuts produce $(c_h+1)(c_w+1)$ rectangles.

Find the minimum total number of cuts needed so that every resulting rectangle has area at most $s$.

## Input Format

Read from standard input.

The first line contains $t$, the number of test cases. Each of the next $t$ lines contains $h,w,s$.

## Output Format

Write to standard output.

Print one integer per test case: the minimum number of cuts.

## Examples

Input:

```text
1
2 2 1
```

Output:

```text
2
```

## Explanation / Notes

One horizontal cut and one vertical cut divide the $2\times 2$ sheet into four unit squares, so two cuts suffice. One cut would leave rectangles of area 2, which exceeds the permitted area 1.

A cut is counted once even when its line passes through several pieces created earlier. Cuts use global horizontal or vertical grid lines, rather than independent cuts on individual rectangles.

## Constraints

- $1\le t\le1000$.
- $1\le h,w,s\le10^9$ in every test case.
- All input values are integers.
- Horizontal coordinates must be integers in $[1,h-1]$; when $h=1$, there is no horizontal cut position.
- Vertical coordinates must be integers in $[1,w-1]$; when $w=1$, there is no vertical cut position.
- Every final rectangle must individually have area at most $s$.
- Zero cuts are allowed.
- Time limit: **3 seconds**. Memory limit: **256 MB**.

## Problem Description

This is an interactive problem.

You're considering moving to another city, where one of your friends already lives. There are $n$ cafés in this city, where $n$ is a power of two. The $i$-th café produces a single variety of coffee $a_i$.

As you're a coffee-lover, before deciding to move or not, you want to know the number $d$ of distinct varieties of coffees produced in this city.

You don't know the values $a_1, \ldots, a_n$. Fortunately, your friend has a memory of size $k$, where $k$ is a power of two.

Once per day, you can ask him to taste a cup of coffee produced by the café $c$, and he will tell you if he tasted a similar coffee during the last $k$ days.

You can also ask him to take a medication that will reset his memory. He will forget all previous cups of coffee tasted. 

More formally, the memory of your friend is a queue $S$. Doing a query on café $c$ will:

* Tell you if $a_c$ is in $S$;
* Add $a_c$ at the back of $S$;
* If $|S| > k$, pop the front element of $S$.

Doing a reset request will pop all elements out of $S$.

Find the diversity $d$ (number of distinct values in the array $a$).

Note that asking your friend to reset his memory does not count towards the number of times you ask your friend to taste a cup of coffee.

In some test cases the behavior of the interactor is adaptive. It means that the array $a$ may be not fixed before the start of the interaction and may depend on your queries. It is guaranteed that at any moment of the interaction, there is at least one array $a$ consistent with all the answers given so far.

## Input Format

Read n and k.

## Output Format

```text
? c
R
! d
```

A tasting returns Y or N. A reset has no reply. Only tastings are charged; the final report gives the number of distinct varieties. Terminate on E. The judge may choose answers adaptively as long as a single assignment of varieties remains consistent with all previous answers.

Fewer tastings earn a higher score. Resets and the final answer are free. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2 1` | |
| | `? 1` |
| `N` | |
| | `? 2` |
| `Y` | |
| | `! 1` |

## Constraints

- n and k are powers of two; 1 ≤ k ≤ n ≤ 1024; n² ≤ 10000k. A café index is between 1 and n. At most 30,000 free resets are allowed.
- Time limit: **10 seconds**. Memory limit: **256 MB**.

While lost on the long walk from the college to the UQ Centre, you have stumbled across the entrance to a secret cave system running deep under the university. The entrance is blocked by a security system consisting of $N$ consecutive doors, each door behind the previous; and N switches, with each switch connected to a different door.

![](https://ioi.contest.codeforces.com/espresso/1f7aed863559e3e0a541816cc2d8c998e32b4473.png)

The doors are numbered $0, 1, \dots, N - 1$ in order, with door $0$ being closest to you. The switches are also numbered $0, 1, \dots, N - 1$, though you do not know which switch is connected to which door.

The switches are all located at the entrance to the cave. Each switch can either be in an up or down position. Only one of these positions is correct for each switch. If a switch is in the correct position then the door it is connected to will be open, and if the switch is in the incorrect position then the door it is connected to will be closed. The correct position may be different for different switches, and you do not know which positions are the correct ones.

You would like to understand this security system. To do this, you can set the switches to any combination, and then walk into the cave to see which is the first closed door. Doors are not transparent: once you encounter the first closed door, you cannot see any of the doors behind it.

You have time to try $70\,000$ combinations of switches, but no more. Your task is to determine the correct position for each switch, and also which door each switch is connected to.

You should submit a file that implements the procedure exploreCave(). This may call the grader function tryCombination() up to $70\,000$ times, and must finish by calling the grader procedure answer(). These functions and procedures are described below.

Grader Function tryCombination():

int tryCombination(int S[]);

The grader will provide this function. It allows you to try a combination of switches, and then enter the cave to determine the first closed door. If all doors are open, the function will return $-­1$. This function runs in $O(N)$ time; that is, the running time is at worst proportional to $N$.

This function may be called at most $70\,000$ times.

Parameters:

* $S$: An array of length $N$, indicating the position of each switch. The element $S[i]$ corresponds to switch $i$. A value of $0$ indicates that the switch is up, and a value of $1$ indicates that the switch is down.
* Returns: The number of the first door that is closed, or ­$-1$ if all doors are open.

Grader Procedure answer():

void answer(int S[], int D[]);

Call this procedure when you have identified the combination of switches to open all doors, and the door to which each switch is connected.

Parameters:

* $S$: An array of length $N$, indicating the correct position of each switch. The format matches that of the function tryCombination() described above.
* $D$: An array of length $N$, indicating the door each switch is connected to. Specifically, element $D[i]$ should contain the door number that switch $i$ is connected to.
* Returns: This procedure does not return, but will cause the program to exit.

Your Procedure exploreCave():

void exploreCave(int N);

Your submission must implement this procedure.

This function should use the grader routine tryCombination() to determine the correct position for each switch and the door each switch is connected to, and must call answer() once it has determined this information.

Parameters:

* $N$: The number of switches and doors in the cave.

Input

The sample grader reads input in the following format:

* line $1$: $N$
* line $2$: $S[0]\ S[1]\dots S[N­ - 1]$
* line $3$: $D[0]\ D[1]\dots D[N­ - 1]$

Here $N$ is the number of doors and switches, $S[i]$ is the correct position for switch $i$, nd $D[i]$ is the door that switch $i$ is connected to.

Scoring

Full constraints: $1 \leq N \leq 5\,000$

|  |  |  |
| --- | --- | --- |
| Subtask | Points | Additional Input Constraints |
| 1 | 12 | For each $i$, switch $i$ is connected to door $i$. Your task is simply to determine the correct combination. |
| 2 | 13 | The correct combination will always be $[0,0,0, \dots ,0]$. Your task is simply to determine which switch connects to which door. |
| 3 | 21 | $N \leq 100$ |
| 4 | 30 | $N \leq 2\,000$ |
| 5 | 24 | (None) |

Example

Input

```
4
1 1 1 0
3 1 0 2
```

Output

Note

Suppose the doors and switches are arranged as in the picture above:

|  |  |  |
| --- | --- | --- |
| Function Call | Returns | Explanation |
| tryCombination([1, 0, 1, 1]) | $1$ | This corresponds to the picture. Switches $0$, $2$ and $3$ are down, while switch $1$ is up. The function returns $1$, indicating that door $1$ is the first door from the left that is closed. |
| tryCombination([0, 1, 1, 0]) | 3 | Doors $0$, $1$ and $2$ are all opened, while door $3$ is closed. |
| tryCombination([1, ­1, 1, 0]) | $-1$ | Moving switch $0$ down causes all doors to be opened, indicated by the return value of ­$-1$. |
| answer([1, 1, 1, 0],[3, 1, 0, 2]) | (Program exits) | We guess that the correct combination is $[1,1, 1,0]$, and that switches $0$, $1$, $2$ and $3$ connect to doors $3$, $1$, $0$ and $2$ respectively. |
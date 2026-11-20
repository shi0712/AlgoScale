# This is an interactive problem.

A mischievous robot is navigating an infinitely large 2D grid. However, its movement is confined to the first quadrant, meaning its coordinates $( x , y )$ must always satisfy $x > 0$ and $y > 0$ . Initially, the robot is located at grid coordinates $( s _ { x } , s _ { y } )$ . All grid cells are initially white.

The game proceeds in discrete time steps, up to a maximum of $T = 1 0 0 0$ steps. In each time step, the following sequence occurs:

1. Your Move: You choose integer coordinates $( x _ { m } , y _ { m } )$ such that $1 \leq x _ { m } \leq T$ and $1 \leq y _ { m } \leq T$ . You then mark this cell black. A cell, once marked black, remains black for the rest of the game. You can mark any cell within the range, including the cell where the robot is currently located.

2. Robot’s Move: The robot, currently at position $( r _ { x } , r _ { y } )$ , observes the grid (including the cell you just marked black). It knows the locations of all currently black cells. It then moves to a new position $( n _ { x } , n _ { y } )$ . The new position $( n _ { x } , n _ { y } )$ must be one of the 8 cells immediately adjacent (horizontally, vertically, or diagonally) to $( r _ { x } , r _ { y } )$ . That is, $| n _ { x } - r _ { x } | \le 1$ and $| n _ { y } - r _ { y } | \le 1$ , and $( n _ { x } , n _ { y } ) \neq ( r _ { x } , r _ { y } )$ . Additionally, the robot must stay within the first quadrant, so $n _ { x } > 0$ and $n _ { y } > 0$ . The interactor (controlling the robot) will choose one valid move for the robot. The robot’s strategy is unknown to you.

3. Outcome Check: After the robot moves to $( n _ { x } , n _ { y } )$ , the system checks if the cell $( n _ { x } , n _ { y } )$ is black.

• If $( n _ { x } , n _ { y } )$ is black, the robot explodes.   
• If $( n _ { x } , n _ { y } )$ is white, the game continues to the next time step, with the robot now at $( n _ { x } , n _ { y } )$ .

Your goal is to make the robot explode within $T$ time steps.

# Input

The first line of input contains two integers $s _ { x }$ and $s _ { y }$ ( $1 \leq s _ { x } , s _ { y } \leq 2 0 _ { , }$ ), the initial coordinates of the robot.

# Interaction Protocol

The interaction proceeds in turns for at most $T$ turns. In each turn $t$ (from $t = 1$ to $T$ ):

1. Your program must print one line containing two space-separated integers $x _ { m }$ and $y _ { m }$ , representing the coordinates of the cell you choose to mark black in this turn. Remember to flush the output stream.

2. The interactor reads your chosen coordinates $\left( x _ { m } , y _ { m } \right)$ .

3. The interactor determines the robot’s next move to $( n _ { x } , n _ { y } )$ based on its current position and the set of all black cells (including the one just marked at $( x _ { m } , y _ { m } ) )$ .

4. The interactor checks if the cell $( n _ { x } , n _ { y } )$ is black.

• If it is black (robot explodes), the interactor will print a single line containing $" 0 \ 0 ^ { \prime }$ and terminate. Your program should then read these values and terminate successfully.

• If it is white, the interactor will print a single line containing two space-separated integers $n _ { x }$ and $n _ { y }$ , the new coordinates of the robot. Your program should read these coordinates to know the robot’s current position for the next turn.

If the robot has not exploded after you have made $T$ moves, your program should terminate. Your solution will be judged as incorrect in this case (or if you exceed the turn limit or make an invalid move).

To flush your output, you can use:

• fflush(stdout) (if you use printf) or cout.flush() (if you use cout) in C and C++.   
• System.out.flush() in Java.   
• stdout.flush() in Python.

Note: If your program does not terminate properly, it may be judged as a Presentation Error (PE).

# Example

<table><tr><td>standard input</td><td> standard output</td></tr><tr><td>55</td><td>61</td></tr><tr><td>54</td><td></td></tr><tr><td>53</td><td>41</td></tr><tr><td>52</td><td>62</td></tr><tr><td></td><td>42</td></tr><tr><td>51</td><td></td></tr><tr><td>00</td><td>52</td></tr></table>
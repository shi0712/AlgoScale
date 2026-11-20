Mirko is a big fan of crop circles, geometrical formations of flattened crops that are supposedly of alien origin.

One summer night he decided to make his own formation on his grandmother's meadow. The great patriot that he is, Mirko decided to make a crop formation that would have the shape of the shield part of the Croatian coat of arms, which is a $5 \times 5$ chessboard with $13$ red squares and $12$ white squares.

![](https://ioi.contest.codeforces.com/espresso/08818a0e55bc586b67034b4c4fa314f49d8f0e57.png)

The chessboard part of the Croatian coat of arms. Grandma's meadow is a square divided into $N \times N$ cells. The cell in the lower left corner of the meadow is represented by the coordinates $(1, 1)$ and the cell in the upper right corner is represented by $(N, N)$.

Mirko decided to flatten only the grass belonging to red squares in the chessboard, leaving the rest of the grass intact. He picked an odd integer $M \ge 3$ and flattened the grass so that each square of the chessboard comprises $M \times M$ cells in the meadow, and the chessboard completely fits inside the meadow.

![](https://ioi.contest.codeforces.com/espresso/6856cf1ac83cbecbb3180c112fd11dedcc5d750e.png)

Example meadow and Mirko's crop formation, with $N = 19$ and $M = 3$. Cells with flattened grass are shown in gray. The center of the formation is at $(12, 9)$ and is marked with a black point.

After Mirko went to sleep, his peculiar creation drew the attention of real aliens! They are floating high above the meadow in their spaceship and examining Mirko's crop formation with a simple device. This device can only determine whether the grass in a particular cell is flattened or not.

The aliens have found one cell with flattened grass and now they want to find the center cell of Mirko's masterpiece, so that they may marvel at its beauty. They do not know the size $M$ of each square in Mirko's formation.

Write a program that, given the size $N$ of the meadow, the coordinates $(X_0, Y_0)$ of one cell with flattened grass, and the ability to interact with the alien device, finds the coordinates of the center cell of Mirko's crop formation.

The device may be used at most $300$ times in one test run.

Interaction

This is an interactive task. Your program sends commands to the alien device using the standard output, and receives feedback from the device by reading from the standard input.

* At the beginning of your program, you should read three integers $N$, $X_0$ and $Y_0$ from the standard input, separated by single spaces. The number $N$ ($15 \le N \le 2\,000\,000\,000$) is the size of the meadow, while $(X_0, Y_0)$ are the coordinates of one cell with flattened grass.
* To examine the grass in the cell $(X, Y)$ using the alien device, you should output a line of the form "examine X Y" to the standard output. If the coordinates $(X, Y)$ are not inside the meadow (the conditions $1 \le X \le N$ and $1 \le Y \le N$ are not satisfied), or if you use this facility more than $300$ times, your program will receive a score of zero on that test run.
* The alien device will respond with a single line containing the word "true" if the grass in cell $(X, Y)$ is flattened and the word "false" otherwise.
* When your program has found the center cell, it should output a line of the form "solution $X_C$ $Y_C$" to the standard output, where $(X_C, Y_C)$ are the coordinates of the center cell.

In order to interact properly with the grader, your program needs to flush the standard output after every write operation.

Scoring

In test cases worth a total of $40$ points, the size $M$ of each of Mirko's squares will be at most $100$. Each test run will have a unique correct answer that will not depend on the questions asked by your program.

Example

Input

```
19 7 4

true

false

false

true
```

Output

```
examine 11 2

examine 2 5

examine 9 14

examine 18 3

solution 12 9
```
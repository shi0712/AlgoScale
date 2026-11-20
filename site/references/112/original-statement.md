# This is an interactive problem.

You are given a simple polygon, but before giving you the $n$ vertices of the polygon, Little Q has shuffled them.

Now you can ask Little Q no more than $( n - 2 )$ queries, each of which consists of two integers $p$ and $q$ $( 0 \leq p / q \leq 1 0 0 0 , 1 \leq q \leq 1 0 0 0$ ). He will tell you the total length of the points on the line $x = p / q$ that lie inside or on the boundary of the polygon, which can be represented as $r / s$ with two integers $r$ and $s$ $r \geq 0 , s \geq 1 , \operatorname* { g c d } ( r , s ) = 1 \quad$ ). Here $\operatorname { g c d } ( r , s )$ is the greatest common divisor of $r$ and $s$ .

You need to find the area of the polygon, which can also be represented as $u / v$ with two integers $u$ and $v$ $( u \geq 0 , v \geq 1 , \operatorname* { g c d } ( u , v ) = 1 $ ).

The polygon and the order of the vertices are determined before the start of the interaction and do not depend on your queries. In other words, the interactor is not adaptive.

# Input

The first line of the input contains an integer $T$ ( $1 \leq T \leq 1 0 0 0$ ), indicating the number of test cases. Fo each test case:

The first line contains an integer $n$ ( $3 \leq n \leq 1 0 0 0$ ), indicating the number of vertices of the polygon.

Then $n$ lines follow, each containing two integers $x$ and $y$ ( $0 \leq x , y \leq 1 0 0 0$ ) that give the coordinate $( x , y )$ of the vertices of the polygon in shuffled order.

The polygon is simple, i.e., its vertices are distinct and no two edges of the polygon intersect or touch, except that consecutive edges touch at their common vertex. In addition, no two consecutive edges are collinear.

It is guaranteed that the sum of $n$ over all test cases does not exceed $1 0 0 0$ .

# Interaction Protocol

If you want to ask a query, output one line. First output ? followed by a space, then output two integers $p$ and $q$ $0 \leq p / q \leq 1 0 0 0 , 1 \leq q \leq 1 0 0 0 )$ . After flushing your output, your program should read two integers $r$ and $s$ $r \geq 0 , s \geq 1 , \operatorname* { g c d } ( r , s ) = 1$ ), indicating the answer to your query.

If you want to guess the area of the polygon, output one line. First output ! followed by a space, then output two integers $u$ and $v$ $( u \geq 0 , v \geq 1 , \operatorname* { g c d } ( u , v ) = 1 )$ ), indicating that the area of the polygon is $u / v$ . After flushing your output, your program should continue processing the next test case, or exit immediately if there are no more test cases. Note that your guess does not count as a query.

To flush your output, you can use:

• fflush(stdout) (if you use printf) or cout.flush() (if you use cout) in C and C++.   
• System.out.flush() in Java and Kotlin.   
• sys.stdout.flush() in Python.

# Example

<table><tr><td>standard input</td><td>standard output</td></tr><tr><td>2</td><td></td></tr><tr><td>4</td><td></td></tr><tr><td>00</td><td></td></tr><tr><td>13</td><td></td></tr><tr><td>11</td><td></td></tr><tr><td>30</td><td></td></tr><tr><td>21</td><td>？11</td></tr><tr><td></td><td>！31</td></tr><tr><td>3</td><td></td></tr><tr><td>00 999 1000</td><td></td></tr><tr><td>1000 999</td><td></td></tr><tr><td></td><td>？11</td></tr><tr><td>1999 999000</td><td></td></tr></table>

# Note

For the first sample case, there are three candidate polygons, shown in the figure below. Only the leftmost one meets the answer 2 to the query $x = 1$ , and thus the area is 3. The other two have the answer 3 to the query $x = 1$ .

![](images/f59aed068ec3581b51f1f5b75d96ef862841625bccda50035f9493a97ab1a609.jpg)

For the second sample case, there is only one candidate polygon, and thus the area is 1999/2.

Note that the order of the vertices shown in the sample case may not be consistent with the actua provided.

The blank lines in the sample case are added for readability. In your output, extra spaces or blank lines will be ignored.
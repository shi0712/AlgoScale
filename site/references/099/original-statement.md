This is an interactive problem.
Petya participates in such contest that the participants only learn results of judging their submissions after the end of the contest.
There are  $n$  problems numbered from 1 to  $n$  in the contest. For each problem he has made a submission. After the contest he was very eager to learn his points and his place. But the results would only be published at the closing ceremony, which is a few hours away.
Petya decided to find out his result before that: to get his points for each problem from the jury, he would ask weird questions. Let Petya's points be  $c_{1}, c_{2}, \ldots, c_{n}$  for problems  $1, 2, \ldots, n$ , respectively. Petya doesn't know these numbers and wants to find them out.
Petya chooses three distinct numbers of the problems and asks the jury to tell him the sum of the minimum and the maximum points he received for these three problems. Fortunately, the jury answers such queries. Formally, if he queries about  $i$ ,  $j$ ,  $k$ , he gets  $\min(c_i, c_j, c_k) + \max(c_i, c_j, c_k)$ .
Petya still doesn't know how to find out all his points. Help him find out  $c_{1}, c_{2}, \ldots, c_{n}$ , by making no more than  $4n$  queries to the jury.
# Interaction Protocol
First, your program must read a single integer  $n -$  the number of the problems in the contest (  $5 \leq n \leq 1000$  ).
Next, your program can make queries about the sum of minimum and maximum points among problems  $i$ ,  $j$ ,  $k$ . In order to do that, output a separate line «? i j k» ( $1 \leq i,j,k \leq n$ ,  $i \neq j$ ,  $j \neq k$ ,  $i \neq k$ ). As the answer to this query, you will get a single integer  $\min(c_i,c_j,c_k) + \max(c_i,c_j,c_k)$  in the input.
If your program makes more than  $4n$  queries or asks an invalid query, you get «Wrong Answer».
After finding Petya's points output  $\ll !c_{1}c_{2}\ldots c_{n}\gg$  - the points for each problem.
It is guaranteed that Petya's points for each problem are integer and satisfy  $0 \leq c_{i} \leq 10^{9}$  for all  $1 \leq i \leq n$ . Also, all numbers  $c_{i}$  are fixed in advance and won't change during the testing of your program.
# Example
<table><tr><td>standard input</td><td>standard output</td></tr><tr><td>5</td><td>? 1 2 3</td></tr><tr><td>2</td><td>? 1 3 5</td></tr><tr><td>4</td><td>? 5 4 3</td></tr><tr><td>4</td><td>? 1 4 2</td></tr><tr><td>1</td><td>! 1 0 2 1 3</td></tr></table>
# Note
Print new line character after each line. After each action your program must flush standard output. If you use "writeln" in Pascal, "cout << ... << endl" in C++, "System.out.println" in Java, "print" in Python, "Console.WriteLine" in C#, standard output is automatically flushed, no additional action is required. If you are using other way to output data, you should flush standard output. Note that even if you flush output, you absolutely must print new line character.
# Explanation
In this sample, there are 5 problems, and Petya got 1, 0, 2, 1, 3 points for the problems. If he queries about problems 1, 2, 3, his minimum points among them is 0 (for the 2-nd problem), and his maximum is 2 (for the 3-rd problem). Thus, the jury will answer  $2 = 0 + 2$  to that query.
Page 1 of 1
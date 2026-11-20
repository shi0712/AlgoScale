
This is an interactive problem.

Bobo had an array $a$ , initially equal to $( 1 , 2 , \ldots , n )$ . He did the following operations with the array.

• For each $i$ from 1 to $n$ in this order, Bobo picked some index $j$ such that $i \leq j \leq \operatorname* { m i n } ( n , i + 2 )$ , and swapped $a _ { i }$ and $a _ { j }$ . Of course, if $i = j$ , then nothing happened after the operation.

Your goal is to determine the final array. You may ask questions of the following type.

• ? i j meaning the question “How do $a _ { i }$ and $a _ { j }$ compare to each other?”. Bobo will respond to this with one symbol $<$ or $>$ , meaning that $a _ { i } < a _ { j }$ or $a _ { i } > a _ { j }$ , respectively.

You may ask no more than $\lfloor 5 n / 3 \rfloor + 5$ questions. After this, you must guess the array.

# Interaction Protocol

First, the interactor prints the number $n$ in a separate line ( $1 \leq n \leq 3 0 0 0 0$ ). Then the solution makes queries, where each query consists of printing $? \ \dot { \textbf { 1 } } \dot { \textbf { ] } }$ on a separate line, where $1 \leq i , j \leq n$ , and $i \neq j$ . After each query the interactor prints one character $<$ or $>$ on a separate line.

After the solution has finished asking questions, it must make a guess. If you think that the array is $( a _ { 1 } , \ldots , a _ { n } )$ , print ! a1 a2 ... $a _ { n }$ on a separate line and terminate.

If your solution makes more than $\lfloor 5 n / 3 \rfloor + 5$ queries, the interactor will finish with the WA verdict. If you do not flush the output after printing a query, you may receive the IL verdict.

Note that the interactor in this task is adaptive, i.e. the array may be generated at the runtime consistently with your questions.

# Example

<table><tr><td>standard input</td><td>standard output</td></tr><tr><td>5</td><td>？54</td></tr><tr><td>&lt;</td><td>？51</td></tr><tr><td>&gt;</td><td>？53</td></tr><tr><td>&gt;</td><td>？31</td></tr><tr><td>&lt;</td><td>？21</td></tr><tr><td>&gt;</td><td>？52</td></tr><tr><td>&gt;</td><td></td></tr><tr><td></td><td>!23154</td></tr></table>
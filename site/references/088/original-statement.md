This is an interactive problem, where your program and the judge interact via standard input and output.

In the kingdom of Duloc, Lord Farquaad is developing a network of watchtowers to monitor every corner of his land. He has a map of towers and the roads that connect them, forming an undirected simple graph $G = ( V , E )$ , where each tower is a vertex and each road is an edge between two towers. However, Farquaad is worried that some parts of Duloc might be isolated, making it impossible to reach every tower from any other.

To ensure full connectivity, he tasks you with verifying whether his network is connected. However, there’s a catch: you’re only allowed limited access to information about the graph.

You can query the network to investigate its connectivity. A query allows you to select a subset of towers $S$ and receive a count of the towers not in $S$ that have direct roads connecting them to at least one tower in $S$ . More precisely, $q u e r y ( S ) = | N ( S ) \setminus S |$ , where $S \subseteq V$ and $N ( S ) = \{ x \vert \exists y \in S$ such that $( x , y ) \in E \}$ .

Your goal is to use these queries efficiently to determine if the network is connected.

Can you help Lord Farquaad confirm the security of his kingdom by verifying that every tower is reachabl from any other in Duloc’s network?

# Input

Interaction starts by reading an integer — the number of vertices.

Then you can make queries of the type "? $\mathtt { s } ^ { \mathfrak { n } }$ (without quotes) where $s$ is a binary string of length $n$ such that character $s _ { i }$ is 1 if node $i \in S$ and 0 otherwise. After the query, read an integer — the answer to your query.

After printing a query do not forget to output end of line and flush the output. The interactor is nonadaptive. The graph does not change during the interaction.

# Constraints

You are allowed to use at most 3500 queries.

# Output

When you find if $G$ is connected or disconnected, print it in the format $" ! ~ \tt ~ x "$ (without quotes), where $x$ is 1 if $G$ is connected and 0 otherwise.

# Note

In the following interaction, $| V | = 4$ , $G = ( V , E )$ , $V = \{ 1 , 2 , 3 , 4 \}$ , $E = \{ ( 1 , 2 ) , ( 2 , 3 ) , ( 3 , 4 ) , ( 2 , 4 ) \} .$ .

<table><tr><td rowspan=1 colspan=1>Input</td><td rowspan=1 colspan=1>Output</td><td rowspan=1 colspan=1>Description</td></tr><tr><td rowspan=1 colspan=1>4</td><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>|V| is given.</td></tr><tr><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>？1100</td><td rowspan=1 colspan=1>Ask a query for subset {1,2}.</td></tr><tr><td rowspan=1 colspan=1>2</td><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>The judge responds with 2.</td></tr><tr><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>？ 0010</td><td rowspan=1 colspan=1>Ask a query for subset {3}.</td></tr><tr><td rowspan=1 colspan=1>2</td><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>The judge responds with 2.</td></tr><tr><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>？1001</td><td rowspan=1 colspan=1>Ask a query for subset {1,4}.</td></tr><tr><td rowspan=1 colspan=1>2</td><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>The judge responds with 2.</td></tr><tr><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>！1</td><td rowspan=1 colspan=1>The algorithm detected that G is connected.</td></tr></table>

Here is another example, $| V | = 2$ , $G = ( V , E )$ , $V = \{ 1 , 2 \}$ , $E = \emptyset$ .

<table><tr><td rowspan=1 colspan=1>Input</td><td rowspan=1 colspan=1>Output</td><td rowspan=1 colspan=1>Description</td></tr><tr><td rowspan=1 colspan=1>2</td><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>|V| is given.</td></tr><tr><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>？10</td><td rowspan=1 colspan=1>Ask a query for subset {1}.</td></tr><tr><td rowspan=1 colspan=1>0</td><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>The judge responds with 0.</td></tr><tr><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>？11</td><td rowspan=1 colspan=1>Ask a query for subset {1,2}.</td></tr><tr><td rowspan=1 colspan=1>0</td><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>The judge responds with 0.</td></tr><tr><td rowspan=1 colspan=1></td><td rowspan=1 colspan=1>！0</td><td rowspan=1 colspan=1>The algorithm detected that G is disconnected.</td></tr></table>
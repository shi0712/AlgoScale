This is an interactive problem. This means that the input to your program will change based on what your program outputs.

Väinö has been selected as the new chairman for the Iconic Character Poetry Contest (ICPC), a televised battle of wits and verse boasting the nation’s best and brightest. Unfortunately, ICPC’s popularity has exploded in recent years, and Väinö is beset with hoards of new applicants wishing to test their mettle.

The program has a limited number of slots to accept applicants, and Väinö is tasked with ensuring that only the best applicants get said slots. He has decided to host a qualifying tournament to decide who advances. In the tournament, he will invite two applicants at a time to compete with each other to assess which has the greater skill at poetry, or runous. He is certain that no two applicants will have the same skill level. The skill levels are transitive, so if $\exists$ defeats b and b defeats $\subset$ , then $\exists$ would also defeat $\subset$ .

However, his time is limited and he wants to limit how many matches he must hold in order to determine the top applicants that will advance. Can you help him choose which matches to hold?

# Interaction

Interaction begins by reading two integers, $N$ and $K$ , representing the total number of applicants and the number that will advance. You are given that $1 \leq N \leq 1 5 0 0 0$ and $1 \le K \le \operatorname* { m i n } ( N , 3 0 )$ . The $N$ applicants are numbered from 1 to $N$ .

You may then make at most $1 2 N + 1 0 0 0$ queries of the form $? \ \mathrm { ~ a ~ } \ \mathrm { ~ b ~ }$ , where $1 \leq a , b \leq N$ and $a \neq b$ . This represents holding a match between applicants $a$ and $b$ , and the interactor will respond with either $a$ or $b$ , representing which applicant won the match and has higher skill.

Remember to flush your output after making a query.

cout.flush(); // $\mathrm { C } + +$ System.out.flush(); // Java print(..., flush ${ . } = { }$ True) # Python

Your submission will receive a Wrong Answer verdict if you make more than $1 2 N + 1 0 0 0$ queries.

Once you have identified the $K$ applicants with highest skill, print them out in the form ! $_ { \textrm { x l } }$ $_ { \textrm { x 2 } }$ x3 ... xK. The top $K$ applicants may be output in any order. Your program should then exit.

The interactor is not adaptive, meaning that the skill level of each applicant is predetermined at the start of the interaction and does not change during the interaction.

Beyond correctly identifying the top $K$ applicants, your queries must also show that your answer is the unique set of $K$ applicants that could be the top $K$ — Väinö’s wrath will befall you if there is any possibility of you missing a budding talent! For example, simply outputting $\ ! 5 3 4$ for the sample will receive Wrong Answer since your queries have not proven that $\{ 5 , 3 , 4 \}$ are the top 3 applicants.

If the interactor receives any invalid input, the interactor will output $- 1$ and then immediately terminate. Your program should cleanly exit in order to receive a Wrong Answer verdict, otherwise the verdict that it receives may be an arbitrary verdict indicating that your submission is incorrect.

You are provided with a command-line tool for local testing. The tool has comments at the top to explain its use.

<table><tr><td colspan="2" rowspan="2">Read                                         Sample Interaction 1                                         Write63</td></tr><tr><td colspan="1" rowspan="1">63</td></tr><tr><td colspan="1" rowspan="1"></td><td colspan="1" rowspan="1">？13</td></tr><tr><td colspan="1" rowspan="1">3</td><td colspan="1" rowspan="1"></td></tr><tr><td colspan="1" rowspan="1"></td><td colspan="1" rowspan="1">？12</td></tr><tr><td colspan="1" rowspan="1">1</td><td colspan="1" rowspan="1"></td></tr><tr><td colspan="1" rowspan="1"></td><td colspan="1" rowspan="1">？35</td></tr><tr><td colspan="1" rowspan="1">5</td><td colspan="1" rowspan="1"></td></tr><tr><td colspan="1" rowspan="1"></td><td colspan="1" rowspan="1">？36</td></tr><tr><td colspan="1" rowspan="1">3</td><td colspan="1" rowspan="1"></td></tr><tr><td colspan="1" rowspan="1"></td><td colspan="1" rowspan="1">？54</td></tr><tr><td colspan="1" rowspan="1">5</td><td colspan="1" rowspan="1"></td></tr><tr><td colspan="1" rowspan="1"></td><td colspan="1" rowspan="1">？34</td></tr><tr><td colspan="1" rowspan="1">4</td><td colspan="1" rowspan="1"></td></tr><tr><td colspan="1" rowspan="1"></td><td colspan="1" rowspan="1">！534</td></tr></table>
You are standing at the very center of a field, which is divided into a grid of cells running north-south and east-west. A great treasure is hidden somewhere within one of these cells.

John Belzoni, a descendant of the famous treasure hunter Giovanni Battista Belzoni, actually discovered that treasure. Unfortunately enough, he died of heatstroke before successfully digging it out; he seems to have spent too long time wandering around the field.

John started his exploration from the central cell of the field where you stand now. All of his footprints leading to the treasure are left on the field, but you cannot identify the footprint on a cell until you reach there. The footprint on a cell indicates which of the four adjacent grid cells he proceeded to. It is known that John did not visit the same grid cell twice or more. You see a footprint on the central cell indicating that John’s first step was northward.

There is exactly one treasure cell in the field, and you will recognize it only when you are on it.

A possible situation of the field is shown in Figure G.1. John’s footprints are depicted as arrows in the cells. The treasure cell is depicted as ${ \bf \epsilon } ^ { \bullet } ( { \bf \vec { \tau } } _ { G } ^ { \bullet }$ . The shaded cell is where you initially stand.

![](images/4b0d167fa585b05654e0a3ebef224e7744cae45171f65111f2408ccac00e544b.jpg)  
Figure G.1. Possible situation

Your task is to find the treasure in a limited number of steps. In a single step, you decide to take one of the four directions north, west, south, or east, and proceed to the adjacent cell in that direction. When you move on to the cell, you may find either the treasure, John’s footprint, or nothing. You do not have to follow John’s footprints. Unlike John’s route, you may visit the same cell more than once. John’s footprints will remain the same regardless of your exploration.

# Interaction

The interaction begins by receiving an integer $n$ $1 \leq n \leq 2 0 0 0 )$ ) from the standard input, followed by a newline. The integer $n$ means that the field is partitioned into $( 2 n + 1 ) \times ( 2 n + 1 )$ grid cells. You are initially on the cell $( n + 1 )$ -th from the west end and $( n + 1 )$ -th from the north end. After receiving the integer $n$ , you can start your exploration steps.

In each step, you send a single character meaning the direction of the cell to move to: ‘ˆ’ (caret) for north, $\cdot _ { < } ,$ (less than symbol) for west, ‘v’ (lowercase V) for south, and $\bullet _ { > } ,$ (greater than symbol) for east. The character should be sent to the standard output followed by a newline.

In its response, you will receive a character indicating what you find on the cell you moved to, followed by a newline. The character ‘G’ means that the treasure is there. The characters ‘ $\because \mathrm { ~  ~ \cdot ~ } < , \mathrm { ~  ~ \cdot ~ } _ { \mathrm { { V } } } ,$ , and $^ \bullet > ^ { : }$ ’ mean John’s footprint directing north, west, south, and east, respectively. The character ‘.’ (dot) means neither the treasure nor a footprint is on the cell.

When you find the treasure, that is, when you have received the character $\mathbf { \epsilon } ^ { \bullet } ( \mathbf { \vec { \jmath } } _ { G } ^ { \ \ \bullet }$ , the interaction stops and your program should terminate. You must reach the treasure cell within 30000 steps. Although following John’s steps will surely lead you to the treasure, that may require more than 30000 steps.

In any of the following cases, your submission will be judged as a wrong answer.

• The output format of your program is invalid.   
• You specify a direction to move out of the grid.   
• Extra outputs are made after reaching the treasure cell.   
• You could not reach the treasure cell within 30000 steps.

The arrangement of the field (the place of the treasure cell and John’s footprints) is fixed before the interaction starts; it does not change during the interaction.

As some environments require flushing the output buffers, make sure that your outputs are actually sent. Otherwise, your outputs will never reach the judge. You are provided with a command-line tool for local testing. For more details, refer to the clarification in the contest system.

![](images/070e0d349e65cf8f76a10a085cf628e77c1f2b42ad7ee4f1f42ad2137bc62696.jpg)

In this interaction, the situation of the field in Figure G.1 is assumed.
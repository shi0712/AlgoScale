## Overview

Build the outer walls x=32 and y=32, each 32 cells long. First paint the seven cells of the vertical wall with y from 26 through 32. Each subsequent move paints a new wall cell. When the robot is near either boundary, choose the mark using the guard recurrence.

## Strategy and correctness

Track the robot's projection on the guarded wall. Once all three positions within distance one are painted, one new mark per move maintains that condition. With h moves left and a bitmask on offsets −h,…,h, enumerate a new white cell to paint, then require a winning state under each possible shift −1,0,+1 of the projection. A containing window is sufficient because farther marks cannot affect the terminal triple. This finite recurrence wins in five moves even from the empty mask. The guard starts when the corresponding coordinate reaches 27, giving five moves before an attempted crossing at 32. When both guards are needed the upper end of the vertical wall is already painted. Thus a surviving robot remains inside [1,31]² until both walls are complete. Next bisect the longer dimension, painting a full dividing line, and keep the side containing the survivor. Finish rectangles of width and height at most two by preferring a white cell other than the robot's current cell.

## Interaction and local complexity

The outer walls contain 63 distinct cells. For width w≥h with w>2 the inside bound is h+max(R(floor((w−1)/2),h),R(floor(w/2),h)); swap axes when necessary. Empty sides cost zero, and a nonempty rectangle with both sides at most two costs max(1,wh−1). R(31,31)=83, hence at most 146 moves including the terminal explosion. The guard table has 6·2¹¹ entries. The independent guard verifier exhausts projection transitions.

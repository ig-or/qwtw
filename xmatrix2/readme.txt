this is a matrix library. 
How to use it:
1. add all the cpp and h files to the project.
2. #include "xmatrix2.h"
3. you may generate doxygen help from the files 
4. look at  xmatrix2.h, there is small help in the beginning
5. define symbol corresponding to your platform; (WIN_PC for windows, see xstdef.h for details);  try to compile.

also have a look at xmcoord.h, xmroundbuf.h   for additional functions

xmatrix2.h: 	vectors, matrix, quaternions, mean and covariance calculation
xmcoord.h: 	earth-related coordinate systems
xmroundbuf.h 	simple round buffer

May be you'll have to define 
void assert_failed(unsigned char* file, unsigned int line, char* str)
function; it may be called if something wrong will happen, in '_DEBUG' mode
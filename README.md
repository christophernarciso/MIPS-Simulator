<snippet>

# MIPS-Simulator
Implementation of a MIPS assembly simulator in C learned in CDA 3103 at the University of Central Florida

## Usage
For your convenience, here is how you could do it in UNIX environment. 
1) First compile:`$ gcc -o spimcore spimcore.c project.c`
2) After compilation, to use MySPIM, you would type the following command in UNIX:`$ ./spimcore <filename>.asc`

Input files are given in ../test_files
  
## Output
Terminal will ask for input.
```
Spimcore console commands:

r = Dump register contents
m = Dump memory contents (in Hexadecimal format)
s[n] = Step n instructions (simulate the next n instruction). If n is not typed, 1 is assumed
c = Continue (carry on the simulation until the program halts(with illegal instruction))
H = Check if the program has halted
d = ads1 ads2 Hexadecimal dump from address ads1 to ads2
I = Inquire memory size
P = Print the input file
g = Display all control signals
X, x, q, Q = Quit


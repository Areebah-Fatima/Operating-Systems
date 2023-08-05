
Document 1: Source File

This document contains all my project code. In summary the program in this file:

- Contains a 2000 entry array to simulate memory
- Implements user and system memory protection
- Interrupts
- Alters the contents of the following registers (PC, IR, SP, X,Y, AC) 
- Contains a switch structure to decode 30+ operations
- Uses a child and parent process to simulate an operating system
- Uses pipes (IPC) to pass data between memory and CPU


Document 2: Extra Sample File

- This file utilizes the stack pointer, jumps, prints etc. to print an elongated image to the screen.
- The 2nd file displays a calculator and displays the subtraction and addition of the values sitting in the AC and register X
- For file 2 ideal timer value is 1000
- File 3 outputs an American flag

Document 3: readme.txt

The current document.

It is responsible for explaining how to compile the source file


Step 1: (to compile)

 gcc [sourceFileName].c 
 
Step 2: (to run)

 a.out [sampleFileName].txt [timerValue]
 
 
 Ex:
 
 gcc project.c
 a.out sample1.txt 20
 
 
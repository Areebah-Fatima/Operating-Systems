#include <stdio.h>

#include <unistd.h>

#include <stdlib.h>

#include <time.h>

//create 2 separate pipes for communication: parent->child, child->parent
// parent to child: "abc", child to parent: "xyz"

// It will consist of 2000 integer entries, 0-999 for the user program, 1000-1999 for system code.
// A timer interrupt should cause execution at address 1000.
// The int instruction should cause execution at address 1500.
//
#define PROG_START_ADR 0
#define PROG_MAX_ADDR 999
#define SYS_START_ADDR 1500
#define SYS_END_ADDR 1999
#define MAX_MEM_SIZE 2000
#define SYS_TMR_PC_START_ADDR 1000
// Control word for write operation
#define CONTROL_WORD 0xFFFF

// CPU Registers
typedef struct {
    // Program Registers
    int pc;
    int sp;
    int ir;
    int ac;
    int x;
    int y;
}
cpu_registered_t;

// Interupt Status
enum int_status_e {
    NO_INT = 1,
        SYS_INT = 2,
        TMR_INT = 3
};

// Operating Mode 
enum cpu_oper_mode_e {
    USER = 0,
        KERNEL = 1
};

void cpu_proc_instruction(cpu_registered_t * reg,
    int * oper_mode, int * int_status,
    int read_from_mem_h, int write_to_mem_h,
    int read_from_cpu_h, int write_to_cpu_h);

void cpu_proc_timer_expiry(cpu_registered_t * reg,
    int * oper_mode, int * int_status,
    int read_from_mem_h, int write_to_mem_h,
    int read_from_cpu_h, int write_to_cpu_h);

void print_memory(int mem[]);
void reset_memory(int mem[]);

int main(int argc, char * argv[]) {
    char raw_data[5000];
    char linedata[100];

    int i = 0;
    cpu_registered_t reg;
    int int_status = NO_INT;
    int oper_mode = USER;
    int timer_value = 0;

    // printf("Memory:  Reading file!\n");
    // Read the text file name 
    FILE * dataFile = fopen(argv[1], "r");

    // A timer will interrupt the processor after every X instructions, 
    // where X is a command-line parameter
    int timer_duration = atoi(argv[2]);
    //printf("Timer duration is %d instructions \n", timer_duration); 

    // Local variables
    int j = 0;
    int mem_data;
    char garbage[100];
    // Handles for the two pipes
    int tochild[2], toparent[2];
    pipe(tochild);
    // printf("fds(tochild): %d, %d\n", tochild[0], tochild[1]);

    pipe(toparent);
    // printf("fds(toparent): %d, %d\n", toparent[0], toparent[1]);

    int read_from_mem = toparent[0];
    int write_to_mem = tochild[1];
    int read_from_cpu = tochild[0];
    int write_to_cpu = toparent[1];
    int instruction = 0;

    fflush(0);
    int fork_rc = fork();
    // printf("fork rc = %d \n", fork_rc);

    // create child process

    if (fork_rc > 0) {
        // printf("CPU started %\nd", reg.pc);
        // Parent Process = CPU 
        //  Readthe first instratution at the starting PC position
        reg.pc = PROG_START_ADR;
        reg.sp = PROG_MAX_ADDR + 1;
        reg.ir = 0;
        // Read first instruction into IR 
        // Send PC to memory
        //printf("Main: CPU reading at PC = %d\n", reg.pc); 
        //write(write_to_mem, &reg.pc, sizeof(reg.pc));

        // Read the instruction from memory
        //read(read_from_mem, &instruction, sizeof(instruction));
        // Load instruction to IR
        //reg.ir = instruction;
        //printf("Main: Read instrcution = %d\n", reg.ir); 
        int test_heck = 0;
        // Read Instructions in a loop 
        //while (1 && test_heck < 30) {
        while (1) {
            // Send PC to memory for read at PC
            //printf("Main: CPU reading instruction (PC:%d, SP:%d)\n", reg.pc, reg.sp); 
            write(write_to_mem, & reg.pc, sizeof(reg.pc));

            // Read the instruction from memory
            read(read_from_mem, & instruction, sizeof(instruction));
            // Load instruction to IR
            reg.ir = instruction;
            //printf("Main: Read instrcution = %d\n", reg.ir); 
            cpu_proc_instruction( & reg, &
                oper_mode, & int_status,
                read_from_cpu, write_to_mem,
                read_from_mem, write_to_cpu);
            // remove tochild[0], tochild[1], toparent[0], toparent[1]);
            test_heck++;
            timer_value++;
            if (timer_value % timer_duration == 0) {
                // Process timer event 
                cpu_proc_timer_expiry( & reg, &
                    oper_mode, & int_status,
                    read_from_cpu, write_to_mem,
                    read_from_mem, write_to_cpu);
            }
            reg.pc++;
        }
        // printf("Parent %d done.\n", getpid());
    } else {
        // Child Process 
        static int mem[2000];
        // bzero memory
        reset_memory( & mem[0]);

        while (fgets(linedata, sizeof(linedata), dataFile) != NULL) {
            // printf("Line read %s \n", linedata);
            mem_data = 0;
            // First character has line encoding information ("." vs. positive integrer) 
            char first_char = linedata[0];
            if ((linedata[0] == ' ' || linedata[0] == '\n')) {
                //printf("skipping empty time or line starts with spaces\n");
            } else if (linedata[0] == '.') {
                //if (linedata[0] == '.') {
                // First character is encoding info 
                // Read the instruction from characters that follows
                // sscanf(linedata, ".%d %s \n", &mem_data, garbage);
                // sscanf(linedata, ".%d %s \n", &mem_data, garbage);
                sscanf(linedata, ".%d %*s\n", & mem_data);
                // printf("Memory data read stats with dot =  .%d\n", mem_data);
                int jump_to_adr = mem_data;
                // Write the memory data to the jump address 
                j = jump_to_adr;
                //printf("Proc dot: read data = .%d to mem[%d] = %d\n", mem_data, jump_to_adr,
                // mem[jump_to_adr]);
                // printf("Memory data read stats with dot =  .%d, garbage = %s\n", mem_data, garbage); 
            } else {
                // First char is an int
                sscanf(linedata, "%d %*s\n", & mem_data);
                // printf("Memory data read =  %d\n", mem_data);
                mem[j] = mem_data;
                // printf("copied mem data = %d to mem[%d] = %d\n", mem_data, j, mem[j]);
                // sscanf(linedata, "%d %s \n", &mem_data, garbage);
                // printf("Memory data read =  %d, garbage = %s\n", mem_data, garbage);
                mem[j] = mem_data;
                j++;
            }

            // Store memory data into memory 
            //mem[j] = mem_data;

            // printf("copied mem data = %d to mem[%d] = %d\n", mem_data, j, mem[j]);
            // Increment memory pointer
            //j++;
        }
        print_memory( & mem[0]);

        // printf("Mem started %d\n", reg.pc);
        // child process - reads first, then writes.
        while (1) {
            // Read PC from CPU
            int readFrom;
            read(read_from_cpu, & readFrom, sizeof(readFrom));
            // printf("Child read from parent PC or control word = %d\n", readFrom);
            if (readFrom == CONTROL_WORD) {
                // Read the memory address where CPU wants to write 
                int addr = 0;
                read(read_from_cpu, & addr, sizeof(addr));
                // Read the value from CPU 
                int value = 0;
                read(read_from_cpu, & value, sizeof(value));
                // Write the value at mem[addr]
                if (addr < MAX_MEM_SIZE) {
                    mem[addr] = value;
                    //printf("Mem: Wrote mem[%d] = %d\n", addr, value);
                } else {
                    //printf("ERR: Mem: Wrote failed at index = %d\n", addr);
                }
            } else {
                write(write_to_cpu, & mem[readFrom], sizeof(mem[readFrom]));
                //printf("Mem: Read mem[%d] = %d\n", readFrom, mem[readFrom]);
            }
        }

        // printf("Child %d done.\n", getpid());
    }

}

void print_memory(int mem[]) {
    int i = 0;
    /*	for (i=0; i<2000; i++) {
    		if (mem[i] != 0) {
    			printf("mem[%d] = %d\n", i, mem[i]);
    		}
    	}
    */
}

void reset_memory(int mem[]) {
    int i = 0;
    for (i = 0; i < 2000; i++) {
        mem[i] = 0;
    }
}

void cpu_proc_timer_expiry(cpu_registered_t * reg,
    int * oper_mode, int * int_status,
    int read_from_cpu_h, int write_to_mem_h,
    int read_from_mem_h, int write_to_cpu_h) {

    int control_word = 0;

    if (!reg) {
        // printf("Null Register passed\n");
    }
    if ((!oper_mode) || (!int_status)) {
        // printf("Null oper mode or int status passed\n");
    }

    //printf("Proc Timer: Opermode = %d, 0x%x, int_status %d, 0x%x\n", 
    //         *oper_mode, oper_mode, *int_status, int_status);

    // We are doing System Timer intrupt Processing
    // Interrupts should be disabled during interrupt processing to avoid nested execution
    // To make it easy, do not allow interrupts during system calls or vice versa
    if (( * int_status == SYS_INT) || ( * int_status == TMR_INT) || ( * oper_mode == KERNEL)) {
        //printf("Skipping timer event handling\n");
        return;
    }

    // Set CPU operating mode to Kernel
    * oper_mode = KERNEL;

    // Set int status to in Timer Call
    * int_status = TMR_INT;

    // Need to save current PC and SP for the User Program to the System Stack 
    int sys_stack_p = MAX_MEM_SIZE - 1;

    // Write user SP at System Stack 
    // Step 1: send the control word that we want to write 
    control_word = CONTROL_WORD;
    write(write_to_mem_h, & control_word, sizeof(control_word));

    // Step 2: Write the address we want to write at (which is sys_stack_p)
    write(write_to_mem_h, & sys_stack_p, sizeof(sys_stack_p));

    // Step 3: Write valuse (which is reg->sp)
    write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));

    //printf("TIMER: Wrote user SP = %d onto System Stack @ [%d]\n", 
    //   reg->sp, sys_stack_p);

    // Now write user SP onto System stack
    // update system stack pointer
    sys_stack_p--;

    // Write user CP at System Stack                                                                            
    // Step 1: send the control word that we want to write                                                      
    control_word = CONTROL_WORD;
    write(write_to_mem_h, & control_word, sizeof(control_word));

    // Step 2: Write the address we want to write at (which is sys_stack_p)                                    
    write(write_to_mem_h, & sys_stack_p, sizeof(sys_stack_p));

    // Step 3: Write valuse (which is reg->cp)
    write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));
    //printf("int (29) Wrote user CP = %d onto System Stack @ [%d]\n",  reg->pc, sys_stack_p);

    // Update the PC to run System Program 
    reg -> pc = SYS_TMR_PC_START_ADDR - 1;

    // Update the SP to point to System Stack 
    reg -> sp = sys_stack_p;

    /*printf("Timer context switched to SYS, Sys CP = %d, Sys SP = %d\n",
	   reg->pc, reg->sp);*/

    return;
}

void cpu_proc_instruction(cpu_registered_t * reg,
    int * oper_mode, int * int_status,
    int read_from_cpu_h, int write_to_mem_h,
    int read_from_mem_h, int write_to_cpu_h) {

    if (!reg) {
        //printf("Null Register passed\n");
    }
    if ((!oper_mode) || (!int_status)) {
        //printf("Null oper mode or int status passed\n");
    }

    //printf("Opermode = %d, 0x%x, int_status %d, 0x%x\n", *oper_mode, oper_mode, *int_status, int_status);

    int addr = 0;
    int value = 0;
    int jump_to_addr = 0;

    switch (reg -> ir) {
    case 1:
        // Load the value into the AC 
        // increament PC to load the value from the memory from the next address
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));
        // printf("Proc Load-Value (1):  PC = %d\n", reg->pc); 
        int value;
        read(read_from_mem_h, & value, sizeof(value));
        reg -> ac = value;
        //printf("Proc Load-Value (1): Value Read = %d\n", reg->ac);
        break;

    case 2:
        // Load the value at the address into the AC
        // Step 1: Increment PC to read the address from memory
        reg -> pc++;
        // Step 2: Read what is written at the new PC value
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));
        // printf("Proc Load addr (2):  PC = %d\n", reg->pc); 
        addr = 0;
        read(read_from_mem_h, & addr, sizeof(addr));
        //printf("Mode(%d), Proc Load addr (2): addr read at PC = %d\n", *oper_mode, addr);

        // User mode should not be allowed to load system memory
        if ( * oper_mode == USER) {
            if (addr > PROG_MAX_ADDR) {
                printf("Memory violation: accessing system address %d in user mode\n",
                    addr);
                exit(0);
                break;
            }
        }

        //
        // Step3: Read memory at the addr
        write(write_to_mem_h, & addr, sizeof(addr));
        // printf("Proc Load addr (2):  write to mem the address to read from = %d\n", addr);
        read(read_from_mem_h, & reg -> ac, sizeof(reg -> ac));
        // printf("Proc Load addr (2): Value read into AC = %d\n", reg->ac);
        break;

    case 3:

        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));

        // Gave the address (ex: 500) now find out what it contains
        addr = 0;
        read(read_from_mem_h, & addr, sizeof(addr));
        // printf("parameter given %d\n", addr);   
        // Want to see whats at address
        write(write_to_mem_h, & addr, sizeof(addr));
        // printf("Proc LoadInd addr (3):  write to mem the address to read from = %d\n", addr);

        int addr2;
        //read in new address now we need to see what sits at new address and load it into ac
        read(read_from_mem_h, & addr2, sizeof(addr2));
        // printf("parameter given", addr2);
        write(write_to_mem_h, & addr2, sizeof(addr2));
        // printf("Proc LoadInd addr (3):  write to mem the address to read from = %d\n", addr2);

        value = 0;
        read(read_from_mem_h, & value, sizeof(value));
        reg -> ac = value;
        //printf("Proc LoadInd Value (3): Value Read AC  = %d\n", reg->ac);

        break;

    case 4:
        // Load the value at (address+X) into the AC
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));

        // Gave the address (ex: 500) now find out what it contains
        addr = 0;
        read(read_from_mem_h, & addr, sizeof(addr));
        // printf("parameter given %d\n", addr);

        // printf("reg x holds %d\n", reg->x);
        addr2 = addr + (reg -> x);
        // printf("new address is %d\n", addr2);

        // want to see whats at address (add+x)
        write(write_to_mem_h, & addr2, sizeof(addr2));
        // printf("Proc LoadIdxX  addr (4):  write to mem the address to read from = %d\n", addr2);

        value = 0;
        read(read_from_mem_h, & value, sizeof(value));
        reg -> ac = value;
        //printf("Proc LoadIdxX  addr (4): Value Read AC  = %d\n", reg->ac);
        break;

    case 5:

        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));

        // gave the address (ex: 500) now find out what it contains
        addr = 0;
        read(read_from_mem_h, & addr, sizeof(addr));
        // printf("parameter given %d\n", addr);

        // printf("reg y holds %d\n", reg->y);
        addr2 = addr + (reg -> y);
        // printf("new address is %d\n", addr2);

        // want to see whats at address (add+y)
        write(write_to_mem_h, & addr2, sizeof(addr2));
        // printf("Proc LoadIdxY  addr (5):  write to mem the address to read from = %d\n", addr2);

        value = 0;
        read(read_from_mem_h, & value, sizeof(value));
        reg -> ac = value;
        //printf("Proc LoadIdxY  addr (5): Value Read AC  = %d\n", reg->ac);
        break;

    case 6:
        // Load the value into the AC 
        //  increament PC to load the value from the memory from the next address

        addr = (reg -> sp) + (reg -> x);
        // printf("reg sp holds %d and x holds %d\n", reg->sp, reg->x);

        write(write_to_mem_h, & addr, sizeof(addr));
        value = 0;
        read(read_from_mem_h, & value, sizeof(value));
        reg -> ac = value;
        //printf("Proc LoadSpX (6): Value Read to AC= %d\n", reg->ac);
        break;

    case 7:
        // Store the value in AC into the address
        // Step1: Read the address where to write from PC++
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));
        // printf("Proc Store_addr (7):  PC = %d\n", reg->pc); 

        addr = 0;
        read(read_from_mem_h, & addr, sizeof(addr));
        // printf("Proc Load addr (7): addr read at PC = %d\n", addr);

        // Step2: Write the control word 
        int control_word = CONTROL_WORD;
        write(write_to_mem_h, & control_word, sizeof(control_word));

        // Step3: Write the address to read from to memory
        write(write_to_mem_h, & addr, sizeof(addr));
        value = reg -> ac;
        write(write_to_mem_h, & value, sizeof(value));
        //printf("Proc Load addr (7): addr read at PC = %d, wrote AC:%d\n", addr, value);
        break;

    case 8:

        srand(time(NULL));
        int random_number = rand() % 100 + 1;
        reg -> ac = random_number;
        break;

    case 9:
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));

        int portV;
        read(read_from_mem_h, & portV, sizeof(portV));
        //printf("Proc Put (9): AC = %d, port = %d\n", reg->ac, portV);

        if (portV == 1) {

            printf("%d", reg -> ac);
        } else if (portV == 2) {

            printf("%c", reg -> ac);

        } else {
            printf("Put port parameter is invalid must be 1 or 2\n");
        }

        break;

    case 10:
        value = 0;
        value = (reg -> ac) + (reg -> x);
        reg -> ac = value;
        //printf("Proc AddX (10): Value Placed in AC= %d\n", reg->ac);
        break;

    case 11:
        value = 0;
        value = (reg -> ac) + (reg -> y);
        reg -> ac = value;
        // printf("Proc AddY (11): Value Placed in AC= %d\n", reg->ac);
        break;

    case 12:
        value = 0;
        value = (reg -> ac) - (reg -> x);
        reg -> ac = value;
        // printf("Proc SubX (10): Value Placed in AC= %d\n", reg->ac);
        break;

    case 13:
        value = 0;
        value = (reg -> ac) - (reg -> y);
        reg -> ac = value;
        // printf("Proc SubY (13): Value Placed in AC= %d\n", reg->ac);
        break;

    case 14:
        value = reg -> ac;
        reg -> x = value;
        //printf("Proc CopyToX (14): Value Placed in Reg X = %d, AC = %d\n", reg->x, reg->ac);
        break;

    case 15:
        value = reg -> x;
        reg -> ac = value;
        //printf("Proc CopyFromX (15): Value Placed in Reg X = %d, AC = %d\n", reg->x, reg->ac);
        break;

    case 16:
        value = reg -> ac;
        reg -> y = value;
        //printf("Proc CopyToY (16): Value Placed in Reg Y = %d, AC = %d\n", reg->y, reg->ac);
        break;

    case 17:
        value = reg -> y;
        reg -> ac = value;
        //printf("Proc CopyFromY (17): Value Placed in Reg Y = %d, AC = %d\n", reg->y, reg->ac);
        break;

    case 18:
        value = reg -> ac;
        reg -> sp = value;
        //printf("Proc CopyToSp (18): Value Placed in Reg Sp = %d, AC = %d\n", reg->sp, reg->ac);
        break;

    case 19:
        value = reg -> sp;
        reg -> ac = value;
        //printf("Proc CopyFromSp (19): Value Placed in Reg Sp = %d, AC = %d\n", reg->sp, reg->ac);
        break;

    case 20:
        // Jump to address (pointed by the PC) 
        // Step 1: Read what is store at PC++
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));

        // printf("Proc Jump addr (20):  Read jump value at PC = %d\n", reg->pc); 
        jump_to_addr = 0;
        read(read_from_mem_h, & jump_to_addr, sizeof(jump_to_addr));
        //printf("Proc Jump addr (20): Jump to value = %d", jump_to_addr);

        // Set the PC to th jump location
        reg -> pc = jump_to_addr - 1;
        break;

    case 21:
        // Jump to address if equal (pointed by the PC) 
        // Step 1: Read what is store at PC++
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));

        // printf("Proc Jump addr (21):  Read jump value at PC = %d\n", reg->pc); 
        jump_to_addr = 0;
        read(read_from_mem_h, & jump_to_addr, sizeof(jump_to_addr));
        //printf("Proc Jump addr (21): Jump to value = %d", jump_to_addr);

        // Set the PC to th jump location
        if ((reg -> ac) == 0) {
            reg -> pc = jump_to_addr - 1;
        }
        break;

    case 22:
        // JumpIfNotEqual addr
        // Jump to the address only if the value in the AC is not zero	
        // Step 1:  Load Address from PC++ 
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));
        //printf("Proc Jump addr (22):  Read jump value at PC = %d\n", reg->pc);  

        jump_to_addr = 0;
        read(read_from_mem_h, & jump_to_addr, sizeof(jump_to_addr));
        //printf("Proc Jump addr (22): Jump to value = %d\n", jump_to_addr); 

        // Set the PC to th jump location 
        if ((reg -> ac) != 0) {
            reg -> pc = jump_to_addr - 1;
            //printf("Proc Jump addr (22): Jump to addr = %d\n", reg->pc); 
        }
        break;

    case 23:
        // 23 = Call addr  
        // Push return address onto stack, jump to the address 
        // Read address at the pc++ 
        reg -> pc++;
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));

        addr = 0;
        read(read_from_mem_h, & addr, sizeof(addr));

        // Push return current PC onto stack before jumping to addr 
        control_word = CONTROL_WORD;
        write(write_to_mem_h, & control_word, sizeof(control_word));
        // Set the SP to write the new addr 
        reg -> sp--;
        // Make sure SP is not writing into user space 
        //if (reg->sp < 999) { // TODO: hash def 
        //	printf("ERR: Stack Overflow new SP = %d\n", reg->sp);;   
        //} 

        // write the address to write at memory; this addr should = sp 
        write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));

        // Write the value = the address we initially read from mem 
        int addr_to_come_back_to = reg -> pc;
        write(write_to_mem_h, & addr_to_come_back_to, sizeof(addr_to_come_back_to));

        //printf("Proc (23): Pushed current PC+1 = %d at stack at addr = %d\n", addr_to_come_back_to, reg->sp); 
        // Jump to address 
        // As we do pc++ after this call, we must set pc to addr-1 
        reg -> pc = addr - 1;
        //printf("Proc (23): Jump to address addr = %d, sp = %d\n", addr, reg->sp); 
        break;

    case 24:
        // Pop return address from the stack, jump to the address                  
        // return addr is at reg->sp                
        write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));
        int ret_addr_at_sp = 0;
        read(read_from_mem_h, & ret_addr_at_sp, sizeof(ret_addr_at_sp));

        //jump to the address;  Update PC  
        reg -> pc = ret_addr_at_sp;
        //printf("Proc (24): jumped to addr = %d from stack, sp = %d\n", ret_addr_at_sp+1, reg->sp);
        // Reset the SP to point to loc we want to write next 
        // Update SP
        reg -> sp++;
        break;

    case 25:
        value = reg -> x;
        reg -> x = value + 1;
        //printf("Proc IncX (25): Value Placed in Reg X = %d\n", reg->x);
        break;

    case 26:
        value = reg -> x;
        reg -> x = value - 1;
        //printf("Proc DecX (26): Value Placed in Reg X = %d\n", reg->x);
        break;

    case 27:
        // Push AC onto stack
        // Update SP
        reg -> sp--;
        // Step 1: send the control word that we want to write 
        control_word = CONTROL_WORD;
        write(write_to_mem_h, & control_word, sizeof(control_word));

        // Step 2: Write the address we want to write at (which is reg->sp) 
        write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));

        // Step 3: Write valuse (which is reg->ac)
        write(write_to_mem_h, & reg -> ac, sizeof(reg -> ac));
        //printf("Proc (27): Push AC = %d to stack @ sp = %d\n", reg->ac, reg->sp);

        break;

    case 28:
        // Pop from stack into AC
        // Step1: Read content stored at the mem[reg->sp]
        write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));

        // Read the mem[reg->sp]
        read(read_from_mem_h, & reg -> ac, sizeof(reg -> ac));
        //printf("Proc (28): POP AC = %d from stack @ sp = %d\n", reg->ac, reg->sp);

        // Update the SP
        reg -> sp++;

        break;

    case 29:
        // int Perform system call
        // The call does NOT have any argugment to read from memory 
        // We are doing System Intrupt Processing 

        // Interrupts should be disabled during interrupt processing to avoid nested execution
        // To make it easy, do not allow interrupts during system calls or vice versa
        if (( * int_status == SYS_INT) || ( * int_status == TMR_INT)) {
            //printf("int (29):  skipping int handler\n");
            // Skip processing the intrupt 
            break;
        }

        // Set CPU operating mode to Kernel
        * oper_mode = KERNEL;

        // Set int status to in System Call
        * int_status = SYS_INT;

        //printf("int (29): Set int_status:%d, oper_mode:%d\n", *int_status, *oper_mode);

        // Need to save current PC and SP for the User Program to the System Stack 
        int sys_stack_p = MAX_MEM_SIZE - 1;

        // Write user SP at System Stack 
        // Step 1: send the control word that we want to write 
        control_word = CONTROL_WORD;
        write(write_to_mem_h, & control_word, sizeof(control_word));

        // Step 2: Write the address we want to write at (which is sys_stack_p)
        write(write_to_mem_h, & sys_stack_p, sizeof(sys_stack_p));

        // Step 3: Write valuse (which is reg->sp)
        write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));

        //printf("int (29) Wrote user SP = %d onto System Stack @ [%d]\n", 
        //       reg->sp, sys_stack_p);

        // Now write user SP onto System stack
        // update system stack pointer
        sys_stack_p--;

        // Write user CP at System Stack                                                                            
        // Step 1: send the control word that we want to write                                                      
        control_word = CONTROL_WORD;
        write(write_to_mem_h, & control_word, sizeof(control_word));

        // Step 2: Write the address we want to write at (which is sys_stack_p)                                    
        write(write_to_mem_h, & sys_stack_p, sizeof(sys_stack_p));

        // Step 3: Write valuse (which is reg->cp)
        write(write_to_mem_h, & reg -> pc, sizeof(reg -> pc));
        //printf("int (29) Wrote user CP = %d onto System Stack @ [%d]\n",
        //       reg->pc, sys_stack_p);

        // Update the PC to run System Program 
        reg -> pc = SYS_START_ADDR - 1;

        // Update the SP to point to System Stack 
        reg -> sp = sys_stack_p;

        //printf("int (29) context switched to SYS, Sys CP = %d, Sys SP = %d\n",
        //reg->pc, reg->sp);

        break;

    case 30:
        //  Interupt Return (Return from system call)
        // This system call does NOT have any argument to read from memory 

        // We are exiting the interupt processing
        // set the mode back to the USER 
        // This is to set future interupt processing 
        *
        int_status = NO_INT;

        // The user data was pushed onto system stack in the following sequence
        // 1. Push user SP 
        // 2. Push user PC
        // Apply the reverse operation for pop

        // 1. Pop user PC
        int recovered_user_pc = 0;
        // Set return address for reading the stack
        // return addr is at reg->sp
        write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));
        // Read user PC
        read(read_from_mem_h, & recovered_user_pc, sizeof(recovered_user_pc));
        //printf("IRet (30): recovered user PC = %d from Sys SP = %d\n", 
        //recovered_user_pc, reg->sp);
        // Update the Sys SP 
        reg -> sp++;

        // 2. Pop user SP
        // Set return address for reading the stack
        // return addr is at reg->sp
        write(write_to_mem_h, & reg -> sp, sizeof(reg -> sp));
        int recovered_user_sp = 0;
        read(read_from_mem_h, & recovered_user_sp, sizeof(recovered_user_sp));
        //printf("IRet (30): recovered user SP = %d from Sys SP = %d\n", 
        //       recovered_user_sp, reg->sp);
        // Update the Sys SP
        reg -> sp++;

        // Now set the context to the user PC and SP
        reg -> pc = recovered_user_pc;
        reg -> sp = recovered_user_sp;

        // We are no longer in kernel mode; set mode to user 
        * oper_mode = USER;
        //printf("int 309): Resetting int_status:%d, oper_mode:%d\n", *int_status, *oper_mode);
        break;

    case 50:
        // printf("Proc End: goodbye!\n");
        exit(0);
        break;

    default:
        printf("Proc instruction: invalid instruction %d\n", reg -> ir);
        break;
    }
}
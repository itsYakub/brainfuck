// + --------------------------------------------------------------- +
// |                                                                 |
// |  ____                   _            __                  _      |
// | | __ )   _ __    __ _  (_)  _ __    / _|  _   _    ___  | | __  |
// | |  _ \  | '__|  / _` | | | | '_ \  | |_  | | | |  / __| | |/ /  |
// | | |_) | | |    | (_| | | | | | | | |  _| | |_| | | (__  |   <   |
// | |____/  |_|     \__,_| |_| |_| |_| |_|    \__,_|  \___| |_|\_\  |
// |                                                                 |
// |                                                                 |
// + --------------------------------------------------------------- +
// | Overcoplicated brainfuck interpreter in C.                      |
// + --------------------------------------------------------------- +
// | Compile command: cc bfi.c -o bfi                                |
// + --------------------------------------------------------------- +
// | Made by: @itsYakub                                              |
// |    https://github.com/itsYakub                                  |
// |                                                                 |
// | Original author: Urban Müller                                   | 
// |	http://aminet.net/dev/lang/brainfuck-2.lha                   |
// + --------------------------------------------------------------- +
// | Rules:                                                          |
// | - '+' - increment the current element;                          |
// | - '-' - decrement the current element;                          |
// | - '>' - move the pointer to the right;                          |
// | - '<' - move the pointer to the left;                           |
// | - '.' - print the current element to output file;               |
// | - ',' - get the value from input file to the current element;   |
// | - '[' - start the loop;                                         |
// | - ']' - end the loop;                                           |
// | Any other character is treated as comment                       |
// + --------------------------------------------------------------- +

// Platform-Dependent inclusions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Platform-Dependent inclusions
#ifdef __linux__

    #include <unistd.h>

#elif _WIN32

    #include <io.h>
    #define isatty(x) _isatty(x)
    #define fileno(x) _fileno(x)

#elif __APPLE__

    #include <unistd.h>

#endif

#define BFI_VERSION "1.0"

#ifdef __linux__

    #define BFI_PLATFORM "Linux"

#elif _WIN32

    #define BFI_PLATFORM "Windows"

#elif __APPLE__
    
    #define BFI_PLATFORM "Apple"

#endif

#define BFI_INPUT_FILE stdin
#define BFI_OUTPUT_FILE stdout
#define BFI_ERROR_FILE stderr

#define BFI_STACKBUF_SIZE 65536

#define BF_STACK_MIN 0
#define BF_STACK_MAX 30000

static char prog_stack[BF_STACK_MAX] = { 0 };
static int  prog_ptr = BF_STACK_MIN;
static int  prog_loop = 0;
static int  prog_piped = 0;

char* bf_fstr(char* fpath);
int bf_flen(FILE* f);
int bf_interpret(char* str);

int main(int ac, char** av) {
    if(ac > 1) { // Execute file
        char* str = bf_fstr(av[1]);
        if(!bf_interpret(str)) {
            fprintf(BFI_ERROR_FILE, "bfi: Interpreter error\n");
            free(str);

            return 1;
        }

        free(str);
    } else { // Execute command-line
        char str[BFI_STACKBUF_SIZE] = { 0 };

        // Check if command-line input is just a program or it's a piped input
        if(!isatty(fileno(BFI_INPUT_FILE))) { // Piped input
            int slen = 0;
            
            for(char c; (c = fgetc(BFI_INPUT_FILE)) != EOF; str[slen++] = c);
            str[slen] = 0;

            prog_piped = 1;

        } else { // simple program execution
            char* sptr = str;
            size_t bufsize = BFI_STACKBUF_SIZE;

            fprintf(BFI_OUTPUT_FILE, "bfi: Brainfuck Interpreter (%s) | Version %s (%s) \n>>> ", __DATE__, BFI_VERSION, BFI_PLATFORM);
            fflush(BFI_OUTPUT_FILE);
            fgets(str, bufsize, BFI_INPUT_FILE);
        }
        
        // Excecute the input from command-line
        if(!bf_interpret(str)) {
            fprintf(BFI_ERROR_FILE, "bfi: Interpreter error\n");
            return 1;
        }
    }

    return 0;
}

char* bf_fstr(char* fpath) {
    FILE* f;
    int flen;
    char* str;

    f = fopen(fpath, "rb");
    if(!f) {
        fprintf(BFI_ERROR_FILE, "bfi: %s\n", strerror(errno));
        return NULL;
    }

    flen = bf_flen(f);
    str = (char*) calloc(flen + 1, sizeof(char));
    if(!str) {
        fprintf(BFI_ERROR_FILE, "bfi: %s\n", strerror(errno));
        fclose(f);
        return NULL;
    }

    if(fread(str, sizeof(char), flen, f) != flen) {
        fprintf(BFI_ERROR_FILE, "bfi: %s\n", strerror(errno));
        fclose(f);
        free(str);
        return NULL;
    }

    str[flen] = 0;
        
    fclose(f);

    return str;
}

int bf_flen(FILE* f) {
    if(!f)
        return 0;

    int flen = 0;

    fseek(f, 0, SEEK_END);
    flen = (int) ftell(f);
    fseek(f, 0, SEEK_SET);

    return flen;
}

int bf_interpret(char* str) {
    if(!str)
        return 0;

    while(*str) {
        switch(*str++) {
            case '+': { prog_stack[prog_ptr]++; } break;
            case '-': { prog_stack[prog_ptr]--; } break;

            case '>': { prog_ptr++; } break;
            case '<': { prog_ptr--; } break;

            case '.': {
                fputc(prog_stack[prog_ptr], BFI_OUTPUT_FILE); 
                fflush(BFI_OUTPUT_FILE); 
            } break;

            case ',': {
                // This problem usually occurs when the code was piped into the interpreter
                // Piped input takes the whole space of the stdin, which means further inputing becomes impossible
                // The same behaviour can be seen in python interpreter, which throws an error when it meets 'input()' function in piped code
                if(prog_piped)
                    return 0;

                prog_stack[prog_ptr] = fgetc(BFI_INPUT_FILE);
                fflush(BFI_OUTPUT_FILE); 
            } break;

            case '[': {
                char* scpy = str;

                prog_loop = 1;
                while(prog_loop && *str) {
                    prog_loop += *str == '[';
                    prog_loop -= *str == ']';
                    str++;
                }

                if(!prog_loop) {
                    str[-1] = 0;
                    
                    while(prog_stack[prog_ptr]) {
                        if(!bf_interpret(scpy))
                            return 0;
                    }

                    str[-1] = ']';

                    break;
                }
            }

            case ']': { return 0; }

            default: { break; }
        }

        if(prog_ptr < BF_STACK_MIN) prog_ptr = BF_STACK_MAX - 1;
        else if(prog_ptr >= BF_STACK_MAX) prog_ptr = BF_STACK_MIN;
    }

    return 1;
}

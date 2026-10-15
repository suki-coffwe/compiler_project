#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.glorp>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    char **messages = NULL;
    int count = 0;
    int capacity = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        // Trim leading whitespace
        char *start = line;
        while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
            start++;
        }
        if (*start == '\0' || *start == '#') continue;

        // Check for 'p' pattern
        if (strncmp(start, "p", 1) == 0) {
            char *ptr = start + 1;
            
            // Skip optional spaces/tabs right after 'p'
            while (*ptr == ' ' || *ptr == '\t') {
                ptr++;
            }
            
            // Find the end of the line (strip trailing \r or \n)
            char *end = ptr;
            while (*end != '\0' && *end != '\r' && *end != '\n') {
                end++;
            }
            
            int len = end - ptr;
            if (len <= 0) continue;

            char *msg = malloc(len + 1);
            strncpy(msg, ptr, len);
            msg[len] = '\0';

            if (count >= capacity) {
                capacity = capacity == 0 ? 4 : capacity * 2;
                messages = realloc(messages, capacity * sizeof(char *));
            }
            messages[count++] = msg;
        }

    }
    fclose(f);

    // Emit NASM output
    printf("default rel\n\n");
    printf("section .data\n");
    for (int i = 0; i < count; i++) {
        printf("    var%d db '%s', 10\n", i + 1, messages[i]);
        printf("    lvar%d equ $ - var%d\n", i + 1, i + 1);
    }

    printf("\nsection .text\n");
    printf("    global _start\n\n");
    printf("_start:\n");

    for (int i = 0; i < count; i++) {
        printf("    mov eax, 1\n");
        printf("    mov edi, 1\n");
        printf("    mov rsi, var%d\n", i + 1);
        printf("    mov edx, lvar%d\n", i + 1);
        printf("    syscall\n\n");
    }

    printf("    mov rax, 60\n");
    printf("    xor rdi, rdi\n");
    printf("    syscall\n");

    // Cleanup
    for (int i = 0; i < count; i++) {
        free(messages[i]);
    }
    free(messages);

    return 0;
}

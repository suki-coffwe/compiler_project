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
/*
        // Check for 'p ' pattern
        if (strncmp(start, "p", 1) == 0) {
            char *q1 = strchr(start, '"');
            if (!q1) continue;
            char *q2 = strchr(q1 + 1, '"');
            if (!q2) continue;

            int len = q2 - (q1 + 1);
            char *msg = malloc(len + 1);
            strncpy(msg, q1 + 1, len);
            msg[len] = '\0';

            if (count >= capacity) {
                capacity = capacity == 0 ? 4 : capacity * 2;
                messages = realloc(messages, capacity * sizeof(char *));
            }
            messages[count++] = msg;
        }
*/

// Check for 'p' pattern
        if (strncmp(start, "p", 1) == 0) {
            // Look for either single or double quote
            char *q1 = strchr(start, '"');
            char *sq1 = strchr(start, '\'');
            
            char quote_char = '\0';
            char *open_q = NULL;
            
            // Pick whichever quote appears first in the line
            if (q1 && sq1) {
                open_q = (q1 < sq1) ? q1 : sq1;
            } else {
                open_q = q1 ? q1 : sq1;
            }
            
            if (!open_q) continue;
            quote_char = *open_q;

            // Find the matching closing quote
            char *close_q = strchr(open_q + 1, quote_char);
            if (!close_q) continue;

            int len = close_q - (open_q + 1);
            char *msg = malloc(len + 1);
            strncpy(msg, open_q + 1, len);
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
        printf("    var%d db \"%s\", 10\n", i + 1, messages[i]);
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

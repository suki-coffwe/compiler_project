#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define VAR_LEN 128

int main(int argc, char *argv[])
{
 if (argc < 2) return 1;

 FILE *f = fopen(argv[1], "r");
 if (!f)
 {
  perror(argv[1]);
  return 1;
 }

 char line[256];
 bool p_found = false;
 bool i_found = false;
 int p_num = 1;
 int line_num = 0;

 printf("default rel\n\n");

 // ==========================================
 // 1. p first section (.data generation)
 // ==========================================
 while (fgets(line, sizeof line, f))
 {
  line_num++;
  line[strcspn(line, "\n")] = '\0';

  if (line[0] == 'p')
  {
   if (!p_found)
   {
 p_found = true;
 printf("section .data\n");
   }
   printf("    ; Line: %d\n", line_num);

   char *p = line + 1;
   char *open, *close;
   int chunk = 1;

   while ((open = strchr(p, '['))) {
 if (open > p) {
  *open = '\0';
  printf("    str_%d_c%d db \"%s\"\n", p_num, chunk, p);
  printf("    len_%d_c%d equ $ - str_%d_c%d\n", p_num, chunk, p_num, chunk);
  *open = '['; // restore
  chunk++;
 }
 open++;
 close = strchr(open, ']');
 if (close) {
  p = close + 1;
 } else {
  break; // unclosed bracket edge case
 }
   }
   // Print remaining literal text + newline
   printf("    str_%d_c%d db \"%s\", 10\n", p_num, chunk, p);
   printf("    len_%d_c%d equ $ - str_%d_c%d\n", p_num, chunk, p_num, chunk);
   p_num++;
  }
 }

 printf("\n");

 // ==========================================
 // 2. i first section (.bss generation)
 // ==========================================
 rewind(f);
 line_num = 0;

 while (fgets(line, sizeof line, f))
 {
  line_num++;
  line[strcspn(line, "\n")] = '\0';

  if (line[0] == 'i')
  {
   if (!i_found)
   {
 i_found = true;
 printf("section .bss\n");
   }
   printf("    ; Line: %d\n", line_num);
   printf("    buf_%s resb %d\n", line + 1, VAR_LEN);
   printf("    len_%s resq 1\n", line + 1);
  }
 }

 printf("\n");

 printf("section .text\n");
 printf("    global _start\n\n");
 printf("_start:\n\n");

 // ==========================================
 // 3. p and i second section (Execution logic)
 // ==========================================
 rewind(f);
 line_num = 0;
 p_num = 1;

 while (fgets(line, sizeof line, f))
 {
  line_num++;
  line[strcspn(line, "\n")] = '\0';

  if (line[0] == 'p')
  {
   printf("    ; Line: %d\n", line_num);
   char *p = line + 1;
   char *open, *close;
   int chunk = 1;

   while ((open = strchr(p, '['))) {
 if (open > p) {
  *open = '\0';
  // Print literal text chunk
  printf("    mov eax, 1\n");
  printf("    mov edi, 1\n");
  printf("    lea rsi, [str_%d_c%d]\n", p_num, chunk);
  printf("    mov rdx, len_%d_c%d\n", p_num, chunk);
  printf("    syscall\n");
  printf("\n");
  *open = '[';
  chunk++;
 }
 open++;
 close = strchr(open, ']');
 if (close) {
  *close = '\0';

  char *var_name = open + 1;

  // Print variable buffer using its captured length (`len_var`)
  printf("    mov eax, 1\n");
  printf("    mov edi, 1\n");
  printf("    lea rsi, [buf_%s]\n", var_name);
  printf("    mov rdx, [len_%s]\n", var_name);
  printf("    syscall\n");
  printf("\n");
  *close = ']';
  p = close + 1;
 } else {
  break;
 }
   }
   // Print final literal chunk + newline
   printf("    mov eax, 1\n");
   printf("    mov edi, 1\n");
   printf("    lea rsi, [str_%d_c%d]\n", p_num, chunk);
   printf("    mov rdx, len_%d_c%d\n", p_num, chunk);
   printf("    syscall\n");
   printf("\n");
   p_num++;
  }

  else if (line[0] == 'i')
  {
   printf("    ; Line: %d\n", line_num);
   printf("    mov eax, 0\n");
   printf("    mov edi, 0\n");
   printf("    lea rsi, [buf_%s]\n", line + 1);
   printf("    mov edx, %d\n", VAR_LEN);
   printf("    syscall\n");
   printf("    mov [len_%s], rax\n", line + 1);
   printf("    dec qword [len_%s]\n", line + 1);
   printf("\n");
  }
 }

 printf("    ; exit:\n");
 printf("    mov eax, 60\n");
 printf("    xor edi, edi\n");
 printf("    syscall\n");

 fclose(f);
 return 0;
}

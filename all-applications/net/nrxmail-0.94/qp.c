#include <stdio.h>

int main() {
  int c;

  while (1) {
    c = fgetc(stdin);
    if (c == EOF) break;
    if (c == '=') {
      int c1, c2;
      c1 = fgetc(stdin);
      if (c1 == '\n') {
        c = c1;
      } else {
        if (c1 >= 'A') c1 -= 'A', c1 += 10; else c1 -= '0';
        c2 = fgetc(stdin);
        if (c2 >= 'A') c2 -= 'A', c2 += 10; else c2 -= '0';
        c = c1 * 16 + c2;
      }
    }
    fputc(c, stdout);
  }

  return 0;
}

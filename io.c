
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct FILE;
typedef struct FILE FILE;

#if !defined(OS_FOPEN)
FILE *fopen(const char *src, const char *name)
{
    return (FILE *)0;
}

int fclose(FILE *stream)
{
    return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return 0;
}
#endif

int putchar(int chr);

void os_putn(size_t n) {
    if (n >= 10) {
        os_putn(n / 10);
    }
    putchar('0' + n % 10);
}

void os_putx1(size_t n) {
    if (n < 10) {
        putchar('0' + n);
    } else {
        putchar('a' + (n-10));
    }
}

void os_putx(size_t n) {
    if (n >= 0x10) {
        os_putn(n / 0x10);
    }
    os_putx1(n % 0x10);
}

void os_put(const char *src) {
    while (*src != '\0') {
        putchar(*(src++));
    }
}


void os_puts(const char *src) {
    os_put(src);
    putchar('\n');
}

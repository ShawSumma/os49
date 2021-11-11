#include <stdint.h>
#include <stddef.h>

void *os_mem_base;
uint8_t os_mem[1 << 26];
size_t os_head = 0;
size_t defcon = 10;

int putchar(int c);
int newline(void);

void os_putn(size_t n);
void os_putx(size_t n);

void *malloc(size_t size) {
    os_mem_base = &os_mem[0];
    uint8_t *ret = &os_mem[os_head]; 
    *(size_t *)ret = size; 
    os_head += size + sizeof(size_t);
    // os_putn(os_head);
    // putchar('\r');
    // if (os_head > (1 << (26 - defcon)))
    // {
    //     putchar('d');
    //     putchar('e');
    //     putchar('f');
    //     putchar('c');
    //     putchar('o');
    //     putchar('n');
    //     putchar(':');
    //     putchar(' ');
    //     os_putn(defcon);
    //     newline();
    //     defcon -= 1;
    //     for (int i = 0; i < 100*1000*1000;i++){}
    // }
    // if (defcon == 0)
    // {
    //     putchar('n');
    //     putchar('u');
    //     putchar('k');
    //     putchar('e');
    //     putchar('!');
    //     while (1) {
    //         defcon = 1 - defcon;
    //     }
    // }
    for (uint8_t *cptr = ret + sizeof(size_t); cptr < ret + size + sizeof(size_t); cptr++)
    {
        *cptr = 0;
    }
    return ret + sizeof(size_t);
}

void *realloc(uint8_t *mem, size_t size) {
    size_t copy = ((size_t *)mem)[-1];
    uint8_t *ret = malloc(size);
    for (size_t i = 0; i < copy; i++)
    {
        ret[i] = mem[i];
    }
    return ret;
}

void* memset(void* dest, int ch, size_t count)
{
    uint8_t *ptr = dest;
    for (size_t i = 0; i < count; i++)
    {
        ptr[i] = ch;
    }
    return dest;
}

void free(void *ptr) {}

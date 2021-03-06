#include <stdint.h>
#include <stddef.h>

#define OS_MEM_MAX (1 << 24)

void *os_mem_base;
uint8_t os_mem[OS_MEM_MAX];
size_t os_head = 0;
size_t os_mem_stat_max = 0;

void os_put(const char *src);
void os_puts(const char *src);
void os_putn(size_t n);
void os_putx(size_t n);

void mreset(void) {
    os_mem_base = &os_mem[0];
    os_head = 0;
}

void *malloc(size_t size) {
    uint8_t *ret = &os_mem[os_head]; 
    *(size_t *)ret = size; 
    os_head += size + sizeof(size_t);
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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stivale2.h>
#include <vm/state.h>

static uint8_t stack[1 << 16];
 
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        .next = 0
    },
    .flags = 0
};
 
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = (uint64_t)&terminal_hdr_tag
    },
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};
 
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
    .tags = (uintptr_t)&framebuffer_hdr_tag
};
 
void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    while (true) {
        if (current_tag == NULL) {
            return NULL;
        }
        if (current_tag->identifier == id) {
            return current_tag;
        }
        current_tag = (void *)current_tag->next;
    }
}

void (*term_write)(const char *string, size_t length);
int putchar(int intc) {
    char c = intc;
    // if (c == '\n') {
    //     c = '\r';
    // }
    term_write(&c, 1);
    for (size_t i = 0; i < 5*1000*1000;i++) {}
    return 0;
}

int newline(void) {
    term_write("\n", 1);
}

#define FILE_PATH "paka/bins/boot.bc"

extern uint8_t os_first_file[]; 

__asm__(
 ".section \".rodata\", \"a\", @progbits\n"
 "os_first_file:\n"
 ".incbin \""FILE_PATH"\"\n"
 ".previous\n"
);

int numv = 0;

void os_fpu_ctrl(const uint16_t cw) {
	asm volatile("fldcw %0" :: "m"(cw));
}

void os_start_fpu(void) {
	size_t cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= 0x200;
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));
	os_fpu_ctrl(0x37F);
}

void _start(struct stivale2_struct *stivale2_struct) {
    os_start_fpu();
    double dubv = numv;

    while (1) {}

    struct stivale2_struct_tag_terminal *term_str_tag;
    term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);
 
    if (term_str_tag == NULL) {
        while (true) {
            asm ("hlt");
        }
    }

    term_write = (void *)term_str_tag->term_write;
 
    void *ops = &os_first_file[1];

    const char *args[] = {"-e" "putchar(10)"};

    vm_state_t *state = vm_state_new(sizeof(args) / sizeof(args[0]), args);
    vm_run(state, ops);
    vm_state_del(state); 

    while (true) {
        asm ("hlt");
    }
}

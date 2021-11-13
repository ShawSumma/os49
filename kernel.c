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

#define FILE_PATH "paka/bin/stage3"

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

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

#define COM1 0x3f8
 
static int init_serial() {
   outb(COM1 + 1, 0x00);    // Disable all interrupts
   outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(COM1 + 1, 0x00);    //                  (hi byte)
   outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(COM1 + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(COM1 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)
 
   // Check if serial is faulty (i.e: not same byte as sent)
   if(inb(COM1 + 0) != 0xAE) {
      return 1;
   }
 
   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   outb(COM1 + 4, 0x0F);
   return 0;
}

static int serial_received() {
   return inb(COM1 + 5) & 1;
}

static int is_transmit_empty() {
   return inb(COM1 + 5) & 0x20;
}
 
void os_serial_write(char a) {
   while (is_transmit_empty() == 0) {}
 
   outb(COM1, a);
}

char os_serial_read() {
   while (serial_received() == 0) {}
 
   return inb(COM1);
}

void (*term_write)(const char *string, size_t length);
int putchar(int intc) {
    char c = intc;
    term_write(&c, 1);
    os_serial_write(c);
    return 0;
}

void os_start_fpu(void) {
	size_t cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= 0x200;
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));
	os_fpu_ctrl(0x37F);
}

void os_put(const char *src);
void mreset(void);

void _start(struct stivale2_struct *stivale2_struct) {
    os_start_fpu();

    struct stivale2_struct_tag_terminal *term_str_tag;
    term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);
 
    if (term_str_tag == NULL) {
        while (true) {
            asm ("hlt");
        }
    }

    term_write = (void *)term_str_tag->term_write;

    os_put("\033[2J");

    os_put("Welcome to os49!\n");

    while(true) {
        mreset();
        os_put(">>> ");
        size_t i = 0;
        char buf[256];
        while (true) {
            char chr = os_serial_read();
            if (chr == '\r') {
                chr = '\n';
            }
            putchar(chr);
            buf[i++] = chr;
            if (chr == '\n') {
                break;
            }
        }
        buf[i] = '\0';
        const char *args[] = {"-e", buf};
        void *ops = &os_first_file[1];

        vm_state_t *state = vm_state_new(sizeof(args) / sizeof(args[0]), args);
        vm_run(state, ops);
        vm_state_del(state); 
    }
 
    while (true) {
        asm ("hlt");
    }
}

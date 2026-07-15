#include <kernel/printf.h>
#include <kernel/mm.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/serial.h>
#include <kernel/string.h>

extern int _hartid[];
void kmain()
{
	printk_set_level(LOG_DEBUG);
	info("entered S-mode\n");
	info("booting on hart %d\n", _hartid[0]);
	info("setting up virtual memory...\n");
	vm_init();

	info("enabling traps...\n");
	trap_setup();
	info("enabling timer...\n");
	timer_irq_enable();
	info("enabling serial...\n");
	serial_init();
	serial_irq_enable();

	hart_irq_enable(); 

    char cmd[256];
    size_t cmd_len = 0;
    char print_buf[64];

    serial_puts("> ");

    while (1) {
        char buf[64];
        size_t n = serial_read(buf);
        
        for (size_t i = 0; i < n; i++) {
            char c = buf[i];
            
            serial_putc(c); 
            
            if (c == '\r') {
                serial_putc('\n'); 
                cmd[cmd_len] = '\0';

                if (strncmp(cmd, "uptime", 6) == 0) {
                    u64 secs = timer_read() / TIMER_FREQ;
                    snprintf(print_buf, sizeof(print_buf), "%lus\n", secs);
                    serial_puts(print_buf);
                    
                } else if (strncmp(cmd, "echo ", 5) == 0) {
                    serial_puts(&cmd[5]);
                    serial_puts("\n");
                    
                } else if (strncmp(cmd, "alarm ", 6) == 0) {
                    u64 secs = strtou64(&cmd[6], 10);
                    timer_set_alarm(secs);
                }
                
                cmd_len = 0;
                serial_puts("> ");
            } else {
                if (cmd_len < sizeof(cmd) - 1) {
                    cmd[cmd_len++] = c;
                }
            }
        }
    }
}

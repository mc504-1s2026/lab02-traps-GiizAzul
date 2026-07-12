#include <kernel/serial.h>
#include <kernel/panic.h>
#include <arch/io.h>
#include <arch/plic.h>
#include <arch/csr.h>
#include <arch/spinlock.h>

#define SERIAL_BUF_SIZE 256 //acho q da?

struct serialdev {
    char buf[SERIAL_BUF_SIZE];
    size_t len;
    struct spinlock lock;
} dev;

void serial_init()
{
	spin_init(&dev.lock);
    dev.len = 0;

    iowrite8(SERIAL_FCR_FIFO_ENABLE | SERIAL_FCR_RX_FIFO_CLEAR | SERIAL_FCR_TX_FIFO_CLEAR, SERIAL_BASE + SERIAL_FCR);

    iowrite8(SERIAL_IER_ERBFI, SERIAL_BASE + SERIAL_IER);

    plic_irq_set_priority(IRQ_SERIAL, 1);
    plic_hart_set_threshold(0, 0);
    plic_hart_enable_irq(0, IRQ_SERIAL);
}

void serial_irq_enable()
{
	csr_set(CSR_SIE, CSR_SIE_SEIE);
}

void serial_irq_disable()
{
	csr_clear(CSR_SIE, CSR_SIE_SEIE);
}

void serial_irq()
{
    u32 irq = plic_hart_claim_irq(0);
    
    if (irq == IRQ_SERIAL) {
        while (ioread8(SERIAL_BASE + SERIAL_LSR) & SERIAL_LSR_DTR) {
            char c = ioread8(SERIAL_BASE + SERIAL_RBR);
            
            u64 flags = spin_lock_irqsave(&dev.lock);
            if (dev.len < SERIAL_BUF_SIZE) {
                dev.buf[dev.len++] = c;
            }
            spin_unlock_irqrestore(&dev.lock, flags);
        }
        plic_hart_complete_irq(0, irq);
    }	
}

size_t serial_read(char *buf)
{
	u64 flags = spin_lock_irqsave(&dev.lock);
    size_t n = dev.len;
    for (size_t i = 0; i < n; i++) {
        buf[i] = dev.buf[i];
    }
    dev.len = 0;
    spin_unlock_irqrestore(&dev.lock, flags);
    return n;
}

void serial_puts(char *str) //todos os caracters
{
	while (*str) {
        serial_putc(*str++);
    }
}

void serial_putc(char c) //um caracter só
{
    while ((ioread8(SERIAL_BASE + SERIAL_LSR) & SERIAL_LSR_THRE) == 0);
    iowrite8(c, SERIAL_BASE + SERIAL_THR);
}

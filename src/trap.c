#include <kernel/trap.h>
#include <kernel/panic.h>
#include <arch/csr.h>
#include <kernel/printf.h>
#include <arch/timer.h>
#include <kernel/serial.h>

/* defined in src/trap_entry.S */
extern void trap_entry();

void handle_irq()
{
	u64 scause = csr_read(CSR_SCAUSE);
    
    if (scause == TRAP_TIMER_IRQ) {
        timer_irq();
    } else if (scause == TRAP_EXTERNAL_IRQ) {
        serial_irq();
    } else {
        warn("Unhandled interrupt! scause: 0x%llx\n", scause);
    }
}

void handle_exception()
{
	u64 scause = csr_read(CSR_SCAUSE);
    u64 sepc = csr_read(CSR_SEPC);
    u64 stval = csr_read(CSR_STVAL);

    panic("Unhandled exception! scause: 0x%llx, sepc: 0x%llx, stval: 0x%llx\n", 
          scause, sepc, stval);
}

void trap_setup()
{
    csr_write(CSR_STVEC, (u64)trap_entry);
    csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

void handle_trap()
{
	u64 scause = csr_read(CSR_SCAUSE);
    
    if (scause & TRAP_IRQ_BIT) {
        handle_irq();
    } else {
        handle_exception();
    }
}

void hart_irq_enable()
{
    csr_set(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

u64 hart_irq_save()
{
    u64 flags = csr_read_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
    return flags & CSR_SSTATUS_SIE;
}

void hart_irq_restore(u64 flags)
{
    if (flags & CSR_SSTATUS_SIE) {
        csr_set(CSR_SSTATUS, CSR_SSTATUS_SIE);
    } else {
        csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
    }
}

void hart_irq_disable()
{
    csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

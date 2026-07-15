#include <arch/timer.h>
#include <kernel/panic.h>
#include <arch/csr.h>
#include <kernel/serial.h>

u64 timer_read()
{
	return csr_read(CSR_TIME);
}

void timer_irq_enable()
{
	csr_set(CSR_SIE, CSR_SIE_STIE);
}

void timer_irq_disable()
{
	csr_clear(CSR_SIE, CSR_SIE_STIE);
}

void timer_set_alarm(u64 secs)
{
	u64 target = timer_read() + secs * TIMER_FREQ; //time ticks = time * TIMER_FREQ
    csr_write(CSR_STIMECMP, target);
}

void timer_irq()
{
	serial_puts("alarm\n> ");
    csr_write(CSR_STIMECMP, -1ULL); //evita outra interrupção
}

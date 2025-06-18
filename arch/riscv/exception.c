#include <kernel.h>
#include "exception.h"
#include "assert.h"
#include "csr.h"
#include "console.h"
#include <stdint.h>
#include <interrupt.h>


//-----------------------------------------------------------------
// Defines:
//-----------------------------------------------------------------
#define SR_MPP_SHIFT    11
#define SR_MPP_MASK     0x3
#define SR_MPP          (SR_MPP_MASK  << SR_MPP_SHIFT)
#define SR_MPP_M        (3 << SR_MPP_SHIFT)

//-----------------------------------------------------------------
// Locals
//-----------------------------------------------------------------
#define CAUSE_MAX_EXC      (CAUSE_PAGE_FAULT_STORE + 1)
static fp_exception        _exception_table[CAUSE_MAX_EXC];

static fp_irq              _irq_handler     = 0;

void exception_set_irq_handler(fp_irq handler)         { _irq_handler = handler; }
void exception_set_syscall_handler(fp_syscall handler) 
{ 
    _exception_table[CAUSE_ECALL_U] = handler;
    _exception_table[CAUSE_ECALL_S] = handler;
    _exception_table[CAUSE_ECALL_M] = handler;
}
void exception_set_handler(int cause, fp_exception handler)
{
    _exception_table[cause] = handler;
}
//-----------------------------------------------------------------
// exception_handler:
//-----------------------------------------------------------------
/*
extern struct irq_context *do_syscall(struct irq_context *ctx);
extern void clkhandler(void);
struct irq_context * exception_handler(struct irq_context *ctx){
 
unsigned int cause_code = ctx->cause & 0xfff;
        if (ctx->cause & CAUSE_INTERRUPT){
        switch (cause_code)
            {
            case 3:
              kprintf("software interruption!\n");
              break;
            case 7:
             
              clear_interrupts();
              // disable_interrupt(MTIM);
            //ctx->pc = (uint32)&os_kernel;
              //enable_interrupt(MTIM);
              ctx->pc =(uint32)&clkhandler;

               

              break;
            case 11:
              kprintf("external interruption!\n");
              break;
            default:
              kprintf("unknown async exception!\n");
              exception_dump_ctx(ctx);
              break;
            }
        }else{
            switch (cause_code){
            case 1:
                {
                    uint32_t val;
                    //asm volatile(".option norvc\ncsrr %0, 0x343":"=r" (val));
                    kprintf("CAUSE_FAULT_FETCH %08x\n",val);
                    exception_dump_ctx(ctx);
                    while(1);
                }
                //ctx->pc += 1;
                break;
                case 2:
              kprintf("Illegal instruction!\n");
              exception_dump_ctx(ctx);
              while(1);
              break;
            case 5:
              kprintf("Fault load!\n");
              exception_dump_ctx(ctx);
              while(1);
              break;
            case 7:
              kprintf("Fault store!\n");
              exception_dump_ctx(ctx);
              while(1);
              break;
            case 8:
                //kprintf("Environment call from U-mode!\n");
                //clear_interrupts();
                  //ctx=do_syscall(ctx);
                  //return_pc += 4;
                  //ctx->pc += 4;
                break;
            case 11:
                //kprintf("Environment call from M-mode!\n");
                //clear_interrupts();
                ctx=do_syscall(ctx);
                //printf("%d %d %d\n"ctx->, );
                 ///return_pc += 4;
                //ctx->pc += 4;
              break;
            default:
         
              kprintf("Sync exceptions! cause code: %d\n", cause_code);
              exception_dump_ctx(ctx);
              while(1);
              break;
      }
    }
return ctx;
}

*/

struct irq_context * exception_handler(struct irq_context *ctx)
{
    //clear_interrupts();
    // External interrupt
    if (ctx->cause & CAUSE_INTERRUPT)
    {
        if (_irq_handler)
            ctx = _irq_handler(ctx);
        else{
            kprintf("Unhandled IRQ!\n");
            ctx->pc += 4;
        }   
    }
    // Exception
    else
    {
        switch (ctx->cause)
        {
            case CAUSE_ECALL_U:
            case CAUSE_ECALL_S:
            case CAUSE_ECALL_M:
                ctx->pc += 4;
                break;
        }

        if (ctx->cause < CAUSE_MAX_EXC && _exception_table[ctx->cause]){
            ctx = _exception_table[ctx->cause](ctx);
        }else if (ctx->cause == CAUSE_FAULT_LOAD || ctx->cause == CAUSE_FAULT_STORE ||
                 ctx->cause == CAUSE_MISALIGNED_LOAD || ctx->cause == CAUSE_MISALIGNED_STORE)
        {
            uint32_t addr = csr_read(0x343); // baddr
            switch (ctx->cause)
            {
                case CAUSE_FAULT_LOAD:
                    kprintf("ERROR: Load fault for access to addr 0x%08x\n", addr);
                    break;
                case CAUSE_FAULT_STORE:
                    kprintf("ERROR: Store fault for access to addr 0x%08x\n", addr);
                    break;
                case CAUSE_MISALIGNED_LOAD:
                    kprintf("ERROR: Misaligned load for access to addr 0x%08x\n", addr);
                    break;
                case CAUSE_MISALIGNED_STORE:
                    kprintf("ERROR: Misaligned store for access to addr 0x%08x\n", addr);
                    break;
                case CAUSE_FAULT_FETCH:
                     kprintf("ERROR: fault fetch access to addr 0x%08x\n", addr);
                    break;
                case  CAUSE_ILLEGAL_INSTRUCTION:
                     kprintf("ERROR: ilegal instruction access to addr 0x%08x\n", addr);
                    break;
                case  CAUSE_MISALIGNED_FETCH:
                     kprintf("ERROR: Misaligned  fetch access to addr 0x%08x\n", addr);
                    break;
            }
            exception_dump_ctx(ctx);
            assert(!"Unhandled exception");
        }else{
            kprintf("Unhandled exception: PC 0x%08x Cause %d!\n", ctx->pc, ctx->cause);
            exception_dump_ctx(ctx);
        }
    }

    return ctx;
}

//-----------------------------------------------------------------
// exception_dump_ctx:
//-----------------------------------------------------------------
void exception_dump_ctx(struct irq_context *ctx)
{
    kprintf("Exception: PC=%08x Cause=%x Status=%08x\n", ctx->pc, ctx->cause, ctx->status);
    for (int i=0;i<NUM_GP_REG;i++)
    {
        kprintf("REG %.2d: %08x\n", i, ctx->reg[i]);
    }
    uint32_t value = 0x5555;  //0x5555 El valor que quieres almacenar
    asm volatile (
        "lui t0, 0x11100       \n"  // Carga los 20 bits superiores (0x11100000) en t0
        //"addi t0, t0, 1      \n"  // Añade los 12 bits inferiores (0x00000001) a t0
        "sw %0, 0(t0)"         // Almacena el valor en la dirección contenida en t0
        :                      // No salida
        : "r" (value)          // Entrada: el valor a almacenar
        : "t0", "memory"       // Clobbers: `t0` y `memory` (ya que estamos modificando memoria)
    );
    for(;;);
}

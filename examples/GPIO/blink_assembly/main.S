.syntax unified
.cpu cortex-m0plus
.thumb

.word __StackTop
.word Reset_Handler
.global Reset_Handler
.type Reset_Handler,%function

.set GPIOA_PWREN,         0x400A0800
.set IOMUX_BASE,          0x40428000 
.set DIO2,                0x3
.set IOMUX_PINCM3,        IOMUX_BASE + (DIO2 * 4)
.set GPIOA_DOESET31_0,    0x400A12D0
.set GPIOA_DOUTTGL31_0,   0x400A12B0
.set PWREN_KEY,           0x26000001

Reset_Handler:
    /* GPIOA peripheral write enable */
    ldr r0, =GPIOA_PWREN
    ldr r1, =PWREN_KEY
    str r1, [r0]     

    /* Configure PINCM3 for GPIOA DIO2. */
    ldr r0, =IOMUX_PINCM3
    movs r1, #0x81                 /* PC_CONNECTED | PF_GPIO */
    str r1, [r0]

    /* Enable output on GPIOA DIO2. */
    ldr r0, =GPIOA_DOESET31_0
    movs r1, #4                    /* GPIOA2 */
    str r1, [r0]

    /* Keep the toggle address and as r1==4, do r2=r1<<20 in blink */
    ldr r0, =GPIOA_DOUTTGL31_0

blink:
    lsls r2, r1, #20

delay:
    subs r2, #1
    bne delay
    str r1, [r0]
    b blink

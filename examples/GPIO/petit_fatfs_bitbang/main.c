#include <ti/driverlib/dl_gpio.h>
#include <ti/driverlib/dl_uart_main.h>
#include <ti/driverlib/m0p/sysctl/dl_sysctl_mspm0c110x.h>

#include "pff.h"

#define BOOT_DELAY (12000000U)
#define POWER_STARTUP_DELAY (16U)
#define BUFFER_SIZE (512U)
#define UART_TX_PIN (DL_GPIO_PIN_27)
#define UART_TX_IOMUX (IOMUX_PINCM28)
#define UART_TX_FUNCTION (IOMUX_PINCM28_PF_UART0_TX)

static void uart_init(void)
{
    static const DL_UART_Main_ClockConfig clock_config = {
        .clockSel = DL_UART_MAIN_CLOCK_BUSCLK,
        .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1,
    };

    static const DL_UART_Main_Config config = {
        .mode = DL_UART_MAIN_MODE_NORMAL,
        .direction = DL_UART_MAIN_DIRECTION_TX,
        .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
        .parity = DL_UART_MAIN_PARITY_NONE,
        .wordLength = DL_UART_MAIN_WORD_LENGTH_8_BITS,
        .stopBits = DL_UART_MAIN_STOP_BITS_ONE,
    };

    DL_UART_Main_reset(UART0);
    DL_UART_Main_enablePower(UART0);
    delay_cycles(POWER_STARTUP_DELAY);

    DL_GPIO_initPeripheralOutputFunction(UART_TX_IOMUX, UART_TX_FUNCTION);
    DL_GPIO_enableOutput(GPIOA, UART_TX_PIN);

    DL_UART_Main_setClockConfig(UART0, (DL_UART_Main_ClockConfig *)&clock_config);
    DL_UART_Main_init(UART0, (DL_UART_Main_Config *)&config);
    DL_UART_Main_setOversampling(UART0, DL_UART_OVERSAMPLING_RATE_3X);
    DL_UART_Main_setBaudRateDivisor(UART0, 70U, 1U);
    DL_UART_Main_enable(UART0);
}

static void gpio_init(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_enablePower(GPIOA);

    delay_cycles(POWER_STARTUP_DELAY);
}

static void uart_putc(int character)
{
    DL_UART_Main_transmitDataBlocking(UART0, (uint8_t)character);
}

static void uart_puts(const char *string)
{
    while (*string) {
        uart_putc(*string++);
    }
}

static void uart_put_uint(unsigned value)
{
    char digits[10];
    unsigned count = 0U;

    do {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) {
        uart_putc(digits[--count]);
    }
}

static void print_readme(const BYTE *buffer, UINT count)
{
    UINT column = 0U;
    UINT index;

    for (index = 0U; index < count; index++) {
        BYTE character = buffer[index];

        if (character == '\r') 
            continue;

        if (character == '\n') {
            uart_puts("\r\n");
            column = 0U;
            continue;
        }

        if (character < ' ' || character > '~')
            character = '.';

        if (column == 80U) {
            uart_puts("\r\n");
            column = 0U;
        }

        uart_putc(character);
        column++;
    }

    if (column != 0U)
        uart_puts("\r\n");
}

static void print_result(const char *operation, FRESULT result)
{
    const char *error_type[] = {
        "FR_OK",            /* 0 */
        "FR_DISK_ERR",      /* 1 */
        "FR_NOT_READY",     /* 2 */
        "FR_NO_FILE",       /* 3 */
        "FR_NOT_OPENED",    /* 4 */
        "FR_NOT_ENABLED",   /* 5 */
        "FR_NO_FILESYSTEM"  /* 6 */
    };

    if (result != FR_OK) {
        uart_puts(operation);
        uart_puts(" error: ");
        uart_puts(error_type[result]);
        uart_puts("\r\n");
    }
}

static void print_line(void)
{
    for(int i=0; i<80; i++)
        uart_puts("-");
    uart_puts("\r\n");
}

/* Petit FatFs cannot create files, grow them, allocate clusters, or update
 * file sizes. WRITEME.TXT must already exist and be at least 512 bytes. */
static void sdcard_example(void)
{
    static FATFS filesystem;
    static BYTE buffer[BUFFER_SIZE];
    UINT transferred;
    UINT index;
    FRESULT result;

    print_line();

    uart_puts("Mounting FAT32 filesystem...\r\n");
    result = pf_mount(&filesystem);
    if (result != FR_OK) {
        print_result("Mount", result);
        return;
    }

    uart_puts("Opening README.TXT...\r\n");
    result = pf_open("README.TXT");
    if (result != FR_OK) {
        print_result("Open README.TXT", result);
        return;
    }

    uart_puts("Reading README.TXT (up to 512 bytes)...\r\n");

    result = pf_read(buffer, BUFFER_SIZE, &transferred);
    if (result != FR_OK) {
        print_result("Read README.TXT", result);
        return;
    }
    uart_puts("README.TXT bytes read: ");
    uart_put_uint((unsigned)transferred);
    uart_puts("\r\n");

    print_line();

    print_readme(buffer, transferred);

    for (index = 0U; index < BUFFER_SIZE; index++) {
        buffer[index] = (BYTE)(' ' + (index % ('~' - ' ' + 1)));
    }

    print_line();

    uart_puts("Opening WRITEME.TXT...\r\n");
    result = pf_open("WRITEME.TXT");
    if (result != FR_OK) {
        uart_puts("WRITEME.TXT must be preallocated to 512 bytes.\r\n");
        print_result("Open WRITEME.TXT", result);
        return;
    }

    uart_puts("Writing 512 visible ASCII characters...\r\n");
    result = pf_write(buffer, BUFFER_SIZE, &transferred);
    if (result == FR_OK) {
        result = pf_write(0, 0U, &transferred);
    }

    if (result != FR_OK) {
        print_result("Write WRITEME.TXT", result);
        return;
    }
    uart_puts("Write complete.\r\n");

    print_line();
}

int main(void)
{
    delay_cycles(BOOT_DELAY);

    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    gpio_init();
    uart_init();
    uart_puts("Petit FatFs SD-card bitbang example\r\n");
    uart_puts("Writing requires a preallocated WRITEME.TXT file.\r\n");
    sdcard_example();

    while (1) {
    }
}

/* 
 * File:   UART functions
 * Author: Wouter
 *
 */

#ifndef UART_DRIVER_H
#define	UART_DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define UART_MODULE_1   0
#define UART_MODULE_2   1
    
#define MESSAGE_TYPE = 0;
#define BLOCK_TYPE = 1;
    
/**
 * Boolean indicating data can be read.
 */    
extern bool UART_flag;

/**
 * Data Struct for reading data.
 */
typedef struct {
    uint8_t sender;
    char command[3];
    char message[10];
} READ_Data;

/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/
/**
 * Initialize the UART module, select which module to use. The module is enabled
 * when it is initialized.
 * @param which: UART_MODULE_1 or UART_MODULE_2
 * @param baud: Baud rate of the UART 
 */
void D_UART_Init(uint16_t which, uint16_t baud);

/**
 * Enable the UART module
 * @param which: UART_MODULE_1 or UART_MODULE_2
 * @param enable Enable or disable UART.
 */
void D_UART_Enable(uint16_t which, bool enable);

/**
 * 
 * @param data
 */
void D_UART_WriteByte(uint8_t data);

/**
 * 
 * @return 
 */
uint8_t D_UART_ReadByte(void);







/**
 * Write data to the TX pin of UART module. 
 * @param command: Command
 * @param data: Data string to write
 */
void C_UART_Write(const char* command, const char* data);

/**
 * Write data to the TX pin of UART module. 
 * @param command: Command
 * @param data: Data integer to write
 */
void C_UART_WriteInt(const char* command, int data);

/**
 * Read data from the RX pin of UART module.
 * @return data: returns the data struct.
 */
READ_Data C_UART_Read();

/**
 * New data available
 * @param data
 */
void C_UART_NewData(uint8_t data);

/**
 * 
 * @return 
 */
READ_Data C_UART_ReadBlock(uint8_t cnt);
uint8_t C_UART_BlockLength();
uint8_t C_UART_MessageType();

void C_UART_Acknowledge(uint16_t val);

/**
 * Get the device name.
 * @return const char* device name
 */
const char* C_UART_GetDeviceName();


#endif
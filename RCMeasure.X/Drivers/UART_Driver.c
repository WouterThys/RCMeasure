#include <xc.h>
#include <stdio.h>
#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "../Settings.h"
#include "SYSTEM_Driver.h"
#include "UART_Driver.h"
/*******************************************************************************
 *          DEFINES
 ******************************************************************************/
// &
#define START_CHAR 0x26 
// $
#define STOP_CHAR  0x24 
// :
#define SEP_CHAR   0x3A 
// A
#define ACK_CHAR   0x41
// M
#define MES_CHAR   0x4D

#define READ_STATE_START   0x00
#define READ_STATE_ID      0x01
#define READ_STATE_LENGTH  0x02
#define READ_STATE_COMMAND 0x03
#define READ_STATE_MESSAGE 0x04
#define READ_STATE_ACK_ID  0x05
#define READ_STATE_CHECK   0x06
#define READ_STATE_END     0x07


/*******************************************************************************
 *          MACRO FUNCTIONS
 ******************************************************************************/

/*******************************************************************************
 *          VARIABLES
 ******************************************************************************/
static uint8_t deviceId;

const char* messageCharacter =     "M";
const char* blockCharacter =       "B";

typedef struct {
    char command[3];      // Command buffer
    uint8_t commandCnt;   // Count of command buffer
    char message[10];  // Message buffer
    uint8_t messageCnt;   // Count of the message buffer
} READ_ComMes;

typedef struct {
    uint8_t id;     // Type of the message buffer  
    uint8_t blockLength;  // One block = <Command>:<Message>:
    READ_ComMes comMes[3];   // Array with commands and messages
    uint8_t ackId;       // Id send from the sender, to acknowledge
    
    uint8_t check;    // Value to check
    uint8_t state;    // State of the read buffer
} READ_Buffer;
READ_Buffer readBuffer;

READ_Data readData;

bool UART_flag;
static bool canWrite;
READ_ComMes comMes;
uint8_t bufferCnt = 0;
uint8_t blockLength = 0;

static uint8_t dummy;


/*******************************************************************************
 *          LOCAL FUNCTIONS
 ******************************************************************************/
static void initUart_1(uint16_t baud, bool interrupts);
static void initUart_2(uint16_t baud);
static void setUartPorts(uint16_t which);

static void fillDataBuffer(uint8_t data);
static void acknowledge(uint8_t ackId);

void initUart_1(uint16_t baud, bool interrupts) {
    // U1MODE register settings
    U1MODEbits.UARTEN = 0; // UARTx is disabled; UARTx pins are controlled by the corresponding PORTx, LATx and TRISx bits
    U1MODEbits.USIDL = 0; // Continues operation in Idle mode 
    U1MODEbits.IREN = 0; // IrDA encoder and decoder are disabled
    U1MODEbits.UEN = 0b00; // UxTX and UxRX pins are enabled and used; UxCTS, UxRTS and BCLKx pins are controlled by port latches        
    U1MODEbits.WAKE = 0; // Wake-up is disabled
    U1MODEbits.LPBACK = 0; // Loop-back mode is disabled
    U1MODEbits.ABAUD = 0; // Baud rate measurement is disabled or complete
    U1MODEbits.URXINV = 1; // UxRX Idle state is ?0?***************************
    U1MODEbits.BRGH = 0; // BRG generates 16 clocks per bit period (16x baud clock, Standard Speed mode)
    U1MODEbits.PDSEL = 0b00; // 8-bit data, no parity
    U1MODEbits.STSEL = 0; // 1 Stop bit
    
    // U1STA register settings
    U1STAbits.UTXISEL0 = 0; // Interrupt is generated when any character is transferred to the Transmit Shift Register and the
    U1STAbits.UTXISEL1 = 0; // transmit buffer is empty (which implies at least one location is empty in the transmit buffer) 
    U1STAbits.UTXINV = 1; // UxTX Idle state is ?0?*********************************
    U1STAbits.UTXEN = 0; // UARTx transmitter is disabled; any pending transmission is aborted and the buffer is reset, UxTX pin is controlled by the port
    U1STAbits.URXISEL = 0b00; // Interrupt flag bit is set when a character is received
    U1STAbits.ADDEN = 0; // Address Detect mode is disabled
    
    // U1BRG register settings
    U1BRG = ((FCY/baud) >> 4) - 1;
    
    // Clear buffers
    while (U1STAbits.URXDA) {
        dummy = U1RXREG;
    }
    U1TXREG = 0x0000;
    
    if (interrupts) {
        _U1RXIF = 0; // Clear flag
        _U1RXIP = U1RX_IP; // Priority
        _U1RXIE = 1; // Enable interrupts
    }
    
    D_UART_Enable(UART_MODULE_1, true);
}

void initUart_2(uint16_t baud) {
    
}

void setUartPorts(uint16_t which){
    if(which == UART_MODULE_1) {
        // Tri-states
        UART1_RX_Dir = 1;
        UART1_TX_Dir = 0;
    
        // Mapping
        RPINR18bits.U1RXR = UART1_RX_Map;
        RPOR2bits.RP39R = UART1_TX_Map;
    } else if(which == UART_MODULE_2) {
        //
    } else {
        return;
    }
}

void fillDataBuffer(uint8_t data){
    switch(readBuffer.state) {
        case READ_STATE_START:
            if(data == START_CHAR) {
                readBuffer.id = 0;
                readBuffer.blockLength = 0;
                comMes.messageCnt = 0;
                comMes.commandCnt = 0;
                blockLength = 0;
                bufferCnt = 0;
                UART_flag = false;
                readBuffer.state = READ_STATE_ID;
            } else {
                readBuffer.state = READ_STATE_START;
                return;
            }
            break;
        
        case READ_STATE_ID:
            if (data == SEP_CHAR) {
                readBuffer.state = READ_STATE_LENGTH;
            } else {
                readBuffer.id = data;
            }
            break;
            
        case READ_STATE_LENGTH:
            if (data == SEP_CHAR) {
                readBuffer.state = READ_STATE_COMMAND;
            } else {
                readBuffer.blockLength = (data - 0x30);
                blockLength = (data - 0x30);
            }
            break;
            
        case READ_STATE_COMMAND:
            if (data == SEP_CHAR) {
                readBuffer.state = READ_STATE_MESSAGE;
                comMes.command[comMes.commandCnt] = '\0';
            } else {
                comMes.command[comMes.commandCnt] = data;
                comMes.commandCnt++;
                if(comMes.commandCnt > 50) {
                    comMes.commandCnt = 0;
                }
            }
            break;
            
        case READ_STATE_MESSAGE:
            if (data == SEP_CHAR) {
                comMes.message[comMes.messageCnt] = '\0';
                readBuffer.comMes[bufferCnt] = comMes;
                register uint8_t i = 0;
                for (i=0; i < comMes.commandCnt; i++) {
                    comMes.command[i] = 0;
                }
                for (i=0; i < comMes.messageCnt; i++) {
                    comMes.message[i] = 0;
                }
                comMes.commandCnt = 0;
                comMes.messageCnt = 0;
                blockLength--;
                bufferCnt++;
                if (blockLength == 0) {
                    readBuffer.state = READ_STATE_ACK_ID;
                } else {
                    readBuffer.state = READ_STATE_COMMAND;
                }
            } else {
                comMes.message[comMes.messageCnt] = data;
                comMes.messageCnt++;
                if(comMes.messageCnt > 50) {
                    comMes.messageCnt = 0;
                }
            }
            break;
            
        case READ_STATE_ACK_ID:
            if (data == SEP_CHAR) {
                readBuffer.state = READ_STATE_END;
            } else {
                readBuffer.ackId = data;
            }
            break;
            
        case READ_STATE_END:
            if (data == STOP_CHAR) {
                acknowledge(readBuffer.ackId);
                readBuffer.state = READ_STATE_START;
                UART_flag = true;
            } else {
                readBuffer.check = data;
            }
            break;
            
        default: 
            readBuffer.state = READ_STATE_START;
            break;
    }
}

void acknowledge(uint8_t ackId) {
    D_UART_WriteByte(START_CHAR);
    D_UART_WriteByte(ACK_CHAR);
    D_UART_WriteByte(ackId);
    D_UART_WriteByte(STOP_CHAR);
    canWrite = true;
}

/*******************************************************************************
 *          DRIVER FUNCTIONS
 ******************************************************************************/
void D_UART_Init(uint16_t which, uint16_t baud) {
    // Register initializes
    if(which == UART_MODULE_1) {
        initUart_1(baud, true);
    } else if (which == UART_MODULE_2) {
        initUart_2(baud);
    } else {
        return;
    }
    
    // Port initializes
    setUartPorts(which);
}

void D_UART_Enable(uint16_t which, bool enable) {
    if (which == UART_MODULE_1) {
        if(enable) {
            U1MODEbits.UARTEN = 1; // UARTx is enabled
            U1STAbits.UTXEN = 1; 
        } else {
            U1MODEbits.UARTEN = 0; // UARTx is disabled
            U1STAbits.UTXEN = 0; 
        }
    } else {
        if(enable) {
            U2MODEbits.UARTEN = 1; // UARTx is enabled
            U2STAbits.UTXEN = 1; 
        } else {
            U2MODEbits.UARTEN = 0; // UARTx is disabled
            U2STAbits.UTXEN = 0; 
        }
    }
}

void D_UART_WriteByte(uint8_t data) {
    U1TXREG = data;
    while(U1STAbits.TRMT == 0);
}

uint8_t D_UART_ReadByte() {
    return U1RXREG;
}

void putch(char data) {
    D_UART_WriteByte(data);
}


void C_UART_Write(const char* command, const char* data) {
//    if (!canWrite) {
//        return;
//    }
    D_UART_WriteByte(START_CHAR); 
    
    // Id
    D_UART_WriteByte(0x30 + deviceId); 
    // Length
    D_UART_WriteByte(SEP_CHAR); D_UART_WriteByte(0x31); // TODO
    // Command
    D_UART_WriteByte(SEP_CHAR); printf(command);
    // Message
    D_UART_WriteByte(SEP_CHAR); printf(data);
    
    // Acknowledge id
    D_UART_WriteByte(SEP_CHAR); D_UART_WriteByte(0x32); // TODO
    // Check
    D_UART_WriteByte(SEP_CHAR); D_UART_WriteByte(0x33); // TODO
    
    D_UART_WriteByte(STOP_CHAR); 
}

void C_UART_WriteInt(const char* command, int data) {
//    if (!canWrite) {
//        return;
//    }
    D_UART_WriteByte(START_CHAR); 
    
    // Id
    D_UART_WriteByte(0x30 + deviceId); 
    // Length
    D_UART_WriteByte(SEP_CHAR); D_UART_WriteByte(0x31); // TODO
    // Command
    D_UART_WriteByte(SEP_CHAR); printf(command);
    // Message
    D_UART_WriteByte(SEP_CHAR); printf("%d", data);
    
    // Acknowledge id
    D_UART_WriteByte(SEP_CHAR); D_UART_WriteByte(0x32); // TODO
    // Check
    D_UART_WriteByte(SEP_CHAR); D_UART_WriteByte(0x33); // TODO
    
    D_UART_WriteByte(STOP_CHAR); 
}

void C_UART_NewData(uint8_t data) {
    fillDataBuffer(data);
}

READ_Data C_UART_Read(){
    READ_ComMes cm = readBuffer.comMes[0];
    readData.sender = readBuffer.id;
    uint16_t i = 0;
    for (i = 0; i < 3; i++) {
        readData.command[i] = cm.command[i];
    }
    for (i = 0; i < 10; i++) {
        readData.message[i] = cm.message[i];
    }
    return readData;
}

READ_Data C_UART_ReadBlock(uint8_t cnt) {
    READ_ComMes cm = readBuffer.comMes[cnt];
    readData.sender = readBuffer.id;
    uint16_t i = 0;
    for (i = 0; i < 3; i++) {
        readData.command[i] = cm.command[i];
    }
    for (i = 0; i < 10; i++) {
        readData.message[i] = cm.message[i];
    }

    return readData;
}

uint8_t C_UART_BlockLength() {
    return readBuffer.blockLength;
}

// UART Received byte
void __attribute__ ( (interrupt, no_auto_psv) ) _U1RXInterrupt(void) {
    if (_U1RXIF) {
        LED3 = !LED3;
        if(U1STAbits.FERR == 1) {
            dummy = U1RXREG;
            LED2 = 1;
            return;
        } 
        if(U1STAbits.OERR == 1) {
            D_UART_Enable(UART_MODULE_1, false);
            DelayUs(10);
            D_UART_Enable(UART_MODULE_1, true);
            LED2 = 1;
            return;
        } 
        
        C_UART_NewData(D_UART_ReadByte());
        _U1RXIF = 0; // Clear interrupt
    }
}
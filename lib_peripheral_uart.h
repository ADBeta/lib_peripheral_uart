/*******************************************************************************
* lib_peripheral_uart is a light and easy to use library for ESP32 / esp-idf 
* devices, it implimets a simple function set to communicate with UART devices 
* and ports
*
* This program is distributed in the hope that it will be useful, but 
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE.
* You should have received a copy of the GNU General Public License along with
* this program; if not, see www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*
* ADBeta (c) 2024    Ver 1.3.0   05 May 2024
*******************************************************************************/
#ifndef LIB_PERIPH_UART
#define LIB_PERIPH_UART

#include <esp_err.h>
#include <driver/uart.h>

/*** Structs ******************************************************************/
//UART Handler struct used for hardware level control
typedef struct {
	uart_port_t port;    // UART Port Enum. For example UART_NUM_1
	int tx;              // TX GPIO Pin
	int rx;              // RX GPIO Pin
	uint32_t baudrate;  
	int tx_buf_size;     // TX Buffer Size (Bytes)
	int rx_buf_size;     // RX Buffer Size (Bytes)
	uint16_t timeout_ms; // Timeout before giving up transmission in millis
	
	/*** WARN: DO NOT CHANGE THESE VARIABLES ***/
	TickType_t timeout_ticks; // Timeout converted to ticks on Init.
	uart_config_t config;
} uart_handler_t;

/*** Interface Control Functions *********************************************/
// Initialises a UART port
// Returns ESP_OK on success
esp_err_t UART_Init(uart_handler_t *uart);

// Sets the UARTs Baudrate to a new value
// Returns ESP_OK on success
esp_err_t UART_SetBaudrate(const uart_handler_t *uart, uint32_t baudrate);

/*** Byte functions **********************************************************/
// Transmits [len] bytes of buffer [buf]
// Returns bytes sent, -1 on error
int UART_Transmit(const uart_handler_t *uart, 
				                         const uint8_t *buf, const size_t len);

// Receives [len] bytes to the buffer [buf]
// Returns bytes received, or -1 on error
int UART_Receive(const uart_handler_t *uart, uint8_t *buf, const size_t len);

/*** String Functions ********************************************************/
// Prints a null terminated string to the UART port.
// Returns total number of bytes sent, -1 on error
int UART_Print(const uart_handler_t *uart, const char *str);
// Same as Print, but sends \n\r after the string.
int UART_PrintNewline(const uart_handler_t *uart, const char *str);

// Reads a string from a UART Port into a buffer string
// Takes a UART Handler, a pointer to a string buffer, and a max length
// Returns length of string, -1 on error
int UART_ReadString(const uart_handler_t *uart, char *str, const size_t len);

// Sends a request string to the UART, and will wait until it responds for
// [waits] number of cycles. Each cycle waits for uart->timeout length.
// Will exit when the buffer is full or a response is given.
// NOTE: DO not use, this is in testing and development
// void UART_WaitForResponse(const uart_handler_t *uart, const char *req, 
//                            char *resp, const size_t len, const size_t waits);

#endif

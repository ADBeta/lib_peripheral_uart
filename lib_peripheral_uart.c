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
* ADBeta (c) 2024
*******************************************************************************/
#include "lib_peripheral_uart.h"

#include <driver/uart.h>
#include <esp_err.h>
#include <string.h>

// Takes a function, if the return is NOT ESP_OK, it will return the same 
// esp_err_t value. NOTE: This is similar to ESP_RETURN_ON_ERROR, but quiet
#define QUIET_RETURN_ON_ERROR(funct) do {                                     \
	esp_err_t f_ret_ = (funct);                                               \
	if(unlikely(f_ret_ != ESP_OK)) return f_ret_;                             \
} while(0)

/*** Interface Control Functions *********************************************/
esp_err_t UART_Init(uart_handler_t *uart)
{
	// Exit if the RX buffer are smaller than UART_HW_FIFO_LEN, OR if 
	// TX buffer is smaller, but NOT Disabled by the user (0 is allowed)
	int min_buf_size = UART_HW_FIFO_LEN(uart->port); 
	if( uart->rx_buf_size < min_buf_size || 
	   (uart->tx_buf_size && uart->tx_buf_size < min_buf_size) )
	{
		return ESP_ERR_NO_MEM;
	}

	// If the interface is already initilised, exit OK
	if(uart_is_driver_installed(uart->port)) return ESP_OK;

	// Set the config struct
	uart->timeout_ticks = uart->timeout_ms / portTICK_PERIOD_MS;
	uart->config = (uart_config_t) {
		.baud_rate = uart->baudrate,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};

	// Install driver then set config
	QUIET_RETURN_ON_ERROR(
		uart_driver_install(uart->port, uart->rx_buf_size, uart->tx_buf_size, 
					                                                0, NULL, 0)
	);
	
	QUIET_RETURN_ON_ERROR(uart_param_config(uart->port, &uart->config));
	
	return uart_set_pin(uart->port, uart->tx, uart->rx,
	                                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

esp_err_t UART_SetBaudrate(const uart_handler_t *uart, uint32_t baudrate)
{
	return uart_set_baudrate(uart->port, baudrate);
}


/*** Byte functions **********************************************************/
int UART_Transmit(const uart_handler_t *uart, 
				                          const uint8_t *buf, const size_t len)
{
	if(buf == NULL) return -1;
	return uart_write_bytes(uart->port, buf, len);
}

int UART_Receive(const uart_handler_t *uart, uint8_t *buf, const size_t len)
{
	if(buf == NULL) return -1;
	return uart_read_bytes(uart->port, buf, len, uart->timeout_ticks);
}

/*** String Functions ********************************************************/
int UART_Print(const uart_handler_t *uart, const char *str)
{
	if(str == NULL) return -1;
	return uart_write_bytes(uart->port, str, strlen(str));
}


int UART_PrintNewline(const uart_handler_t *uart, const char *str)
{
	if(str == NULL) return -1;
	int b_sent = uart_write_bytes(uart->port, str, strlen(str));
	b_sent += uart_write_bytes(uart->port, "\r\n", 2);
	return b_sent;
}


int UART_ReadString(const uart_handler_t *uart, char *str, const size_t len)
{
	int b_read = uart_read_bytes(uart->port, str, len-1, uart->timeout_ticks);
	// Terminate the string
	if(b_read > 0) str[b_read] = '\0';
	return b_read;
}

/*
// NOTE: This is unfinished and shouldn't be used. Testing & development
void UART_WaitForResponse(const uart_handler_t *uart, const char *req, 
						     char *resp, const size_t len, const size_t waits)
{
	// Write the request string out, and wait for the TX Buffer to empty
	UART_PrintNewline(uart, req);
	uart_wait_tx_done(uart->port, uart->timeout);

	// Offset position in the string to copy response to
	size_t offset = 0;
	
	// Pointer to the next position in the string to put chars, and keep track
	// of how many free chars are left in the output string
	char *resp_ptr = resp;
	size_t chars_free = len -1;

	// Keep adding chars to the output string until it is full, or the timeout
	// is reached.
	size_t count = 0;
	while(count < waits && chars_free) {
		// Get a response from the UART, only 
		size_t resp_chars = uart_read_bytes(uart->port, resp_ptr, chars_free, 
									     uart->timeout / portTICK_PERIOD_MS);
		
		// If any chars were read, move the offset position by [n], and remove
		// [n] chars from the amount left free in the output.
		if(resp_chars) {
			offset += resp_chars;
			chars_free -= rest_chars;
			resp_ptr += sizeof(char) * offset;
		}

		printf("Count: %u\t\tBytes: %u\t\tOffset: %u\n", count, resp_bytes, offset);
		
		// Move the offset along by how many byutes were read
		++count;		
	}

	//Terminate the string
	resp[offset] = '\0';
}*/

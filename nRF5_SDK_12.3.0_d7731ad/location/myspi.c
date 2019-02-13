#include "myspi.h"
#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "main.h"

#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

#define TEST_STRING "Nordic"
static uint8_t       m_rx_buf[256];    /**< RX buffer. */
const uint8_t m_length =0;        /**< Transfer length. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    spi_xfer_done = true;
    NRF_LOG_INFO("Transfer completed.\r\n");
    if (m_rx_buf[0] != 0)
    {
        NRF_LOG_INFO(" Received: \r\n");
        NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
    }
}

void spi_init(void)
{

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_PSELSS0;
    spi_config.miso_pin = SPI_PSELMISO0;
    spi_config.mosi_pin = SPI_PSELMOSI0;
    spi_config.sck_pin  = SPI_PSELSCK0;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler));
}
void Spi_WriteByte(uint8_t data)
{
    uint8_t temp[1];
    temp[0]=data;
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, temp, 1, NULL, NULL)); 
}
uint8_t Spi_ReadByte(void){
    uint8_t temp[1];
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, NULL, NULL, temp, 1)); 
    return temp[0];
}


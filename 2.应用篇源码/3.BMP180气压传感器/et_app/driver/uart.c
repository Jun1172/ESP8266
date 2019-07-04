/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: uart.c
 * ��;�����������
 * �޸����ݣ�
 * ���Ӻ���
   void uart0_init(UART_BautRate baud_rate );
   void uart0_tx_SendStr(uint8 *buf);
   void uart0_tx_SendNum(uint32 num);
*******************************************************************************/
#include "esp_common.h"

#include "driver/uart.h"
#include "espressif/esp_libc.h"
#include "espressif/esp_system.h"
#include "et_fifo.h"
#include "factory.h"

LOCAL STATUS
uart_tx_one_char(uint8 uart, uint8 TxChar)
{
    while (true) {
        uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(uart)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);

        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
            break;
        }
    }

    WRITE_PERI_REG(UART_FIFO(uart) , TxChar);
    return OK;
}

LOCAL void
uart1_write_char(char c)
{
    if (c == '\n') {
        uart_tx_one_char(UART1, '\r');
        uart_tx_one_char(UART1, '\n');
    } else if (c == '\r') {
    } else {
        uart_tx_one_char(UART1, c);
    }
}

LOCAL void
uart0_write_char(char c)
{
    if (c == '\n') {
        uart_tx_one_char(UART0, '\r');
        uart_tx_one_char(UART0, '\n');
    } else if (c == '\r') {
    } else {
        uart_tx_one_char(UART0, c);
    }
}

et_int32 recv_data_uart(et_char *buffer,et_int32 len,et_int32 time_out_ms)
{
	et_int32 rc = 0;
	et_int32 count = time_out_ms/10;
	et_int32 num = 0;
	et_int32 size = 0;

	do
	{
		rc = read_data_from_buf(buffer+size,len-size);
		size += rc;
		if(size != len)
		{
			os_delay_us(10*1000);
			num++;
		}
		else
		{
			break;
		}
	}while(size != len && num < count);

	return size;

}

//=================================================================

void ICACHE_FLASH_ATTR
UART_SetWordLength(UART_Port uart_no, UART_WordLength len)
{
    SET_PERI_REG_BITS(UART_CONF0(uart_no), UART_BIT_NUM, len, UART_BIT_NUM_S);
}

void ICACHE_FLASH_ATTR
UART_SetStopBits(UART_Port uart_no, UART_StopBits bit_num)
{
    SET_PERI_REG_BITS(UART_CONF0(uart_no), UART_STOP_BIT_NUM, bit_num, UART_STOP_BIT_NUM_S);
}

void ICACHE_FLASH_ATTR
UART_SetLineInverse(UART_Port uart_no, UART_LineLevelInverse inverse_mask)
{
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_LINE_INV_MASK);
    SET_PERI_REG_MASK(UART_CONF0(uart_no), inverse_mask);
}

void ICACHE_FLASH_ATTR
UART_SetParity(UART_Port uart_no, UART_ParityMode Parity_mode)
{
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_PARITY | UART_PARITY_EN);

    if (Parity_mode == USART_Parity_None) {
    } else {
        SET_PERI_REG_MASK(UART_CONF0(uart_no), Parity_mode | UART_PARITY_EN);
    }
}

void ICACHE_FLASH_ATTR
UART_SetBaudrate(UART_Port uart_no, uint32 baud_rate)
{
    uart_div_modify(uart_no, UART_CLK_FREQ / baud_rate);
}

//only when USART_HardwareFlowControl_RTS is set , will the rx_thresh value be set.
void ICACHE_FLASH_ATTR
UART_SetFlowCtrl(UART_Port uart_no, UART_HwFlowCtrl flow_ctrl, uint8 rx_thresh)
{
    if (flow_ctrl & USART_HardwareFlowControl_RTS) {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_U0RTS);
        SET_PERI_REG_BITS(UART_CONF1(uart_no), UART_RX_FLOW_THRHD, rx_thresh, UART_RX_FLOW_THRHD_S);
        SET_PERI_REG_MASK(UART_CONF1(uart_no), UART_RX_FLOW_EN);
    } else {
        CLEAR_PERI_REG_MASK(UART_CONF1(uart_no), UART_RX_FLOW_EN);
    }

    if (flow_ctrl & USART_HardwareFlowControl_CTS) {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_UART0_CTS);
        SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_TX_FLOW_EN);
    } else {
        CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_TX_FLOW_EN);
    }
}

void ICACHE_FLASH_ATTR
UART_WaitTxFifoEmpty(UART_Port uart_no) //do not use if tx flow control enabled
{
    while (READ_PERI_REG(UART_STATUS(uart_no)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S));
}

void ICACHE_FLASH_ATTR
UART_ResetFifo(UART_Port uart_no)
{
    SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
}

void ICACHE_FLASH_ATTR
UART_ClearIntrStatus(UART_Port uart_no, uint32 clr_mask)
{
    WRITE_PERI_REG(UART_INT_CLR(uart_no), clr_mask);
}

void ICACHE_FLASH_ATTR
UART_SetIntrEna(UART_Port uart_no, uint32 ena_mask)
{
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), ena_mask);
}

void ICACHE_FLASH_ATTR
UART_intr_handler_register(void *fn)
{
    _xt_isr_attach(ETS_UART_INUM, fn, NULL);
}

void ICACHE_FLASH_ATTR
UART_SetPrintPort(UART_Port uart_no)
{
    if (uart_no == 1) {
        os_install_putc1(uart1_write_char);
    } else {
        os_install_putc1(uart0_write_char);
    }
}

void ICACHE_FLASH_ATTR
UART_ParamConfig(UART_Port uart_no,  UART_ConfigTypeDef *pUARTConfig)
{
    if (uart_no == UART1) {
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
    } else {
        PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
    }

    UART_SetFlowCtrl(uart_no, pUARTConfig->flow_ctrl, pUARTConfig->UART_RxFlowThresh);
    UART_SetBaudrate(uart_no, pUARTConfig->baud_rate);

    WRITE_PERI_REG(UART_CONF0(uart_no),
                   ((pUARTConfig->parity == USART_Parity_None) ? 0x0 : (UART_PARITY_EN | pUARTConfig->parity))
                   | (pUARTConfig->stop_bits << UART_STOP_BIT_NUM_S)
                   | (pUARTConfig->data_bits << UART_BIT_NUM_S)
                   | ((pUARTConfig->flow_ctrl & USART_HardwareFlowControl_CTS) ? UART_TX_FLOW_EN : 0x0)
                   | pUARTConfig->UART_InverseMask);

    UART_ResetFifo(uart_no);
}

void ICACHE_FLASH_ATTR
UART_IntrConfig(UART_Port uart_no,  UART_IntrConfTypeDef *pUARTIntrConf)
{

    uint32 reg_val = 0;
    UART_ClearIntrStatus(uart_no, UART_INTR_MASK);
    reg_val = READ_PERI_REG(UART_CONF1(uart_no)) & ~((UART_RX_FLOW_THRHD << UART_RX_FLOW_THRHD_S) | UART_RX_FLOW_EN) ;

    reg_val |= ((pUARTIntrConf->UART_IntrEnMask & UART_RXFIFO_TOUT_INT_ENA) ?
                ((((pUARTIntrConf->UART_RX_TimeOutIntrThresh)&UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S) | UART_RX_TOUT_EN) : 0);

    reg_val |= ((pUARTIntrConf->UART_IntrEnMask & UART_RXFIFO_FULL_INT_ENA) ?
                (((pUARTIntrConf->UART_RX_FifoFullIntrThresh)&UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) : 0);

    reg_val |= ((pUARTIntrConf->UART_IntrEnMask & UART_TXFIFO_EMPTY_INT_ENA) ?
                (((pUARTIntrConf->UART_TX_FifoEmptyIntrThresh)&UART_TXFIFO_EMPTY_THRHD) << UART_TXFIFO_EMPTY_THRHD_S) : 0);

    WRITE_PERI_REG(UART_CONF1(uart_no), reg_val);
    CLEAR_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_INTR_MASK);
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), pUARTIntrConf->UART_IntrEnMask);
}

/********************************************
 * ��������uart0_rx_intr_handler()
 * ������;�����ڽ��մ�����
 * ������para
 * �������ã��洢��������
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
unsigned char uart_RcvString[100],uart_RcvString_Num=0;
unsigned char red_pwn=100,blue_pwn=100,green_pwn=100;

LOCAL void
uart0_rx_intr_handler()
{
	uint8 RcvChar;//����һ������������һ���ֽ�

	//�ж��Ƿ񴥷����ڽ����ж�
	if (UART_RXFIFO_FULL_INT_ST != (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST)) {
		return;
	}

	if(uart_RcvString_Num==0)
	{
		bzero(uart_RcvString,sizeof(uart_RcvString));
	}

	/*****************����һ���ֽ����� ******************/
	//׼��һ�����ݽ���
	WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);

	//����һ���ֽڵ�����
	while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) {
		RcvChar = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
		//ԭ������ֽڣ�����ʹ��
		//uart_tx_one_char(UART0, RcvChar);
	}

	if(RcvChar=='%')
	{
		//���յ��������ַ����������
		uart_RcvString[uart_RcvString_Num]=RcvChar;

		//��ʼ�ж��ǿ���ʲô�ƣ������������
		if(uart_RcvString[1]=='R')
		{
			if(uart_RcvString[7]=='%')
			{
				red_pwn=uart_RcvString[5]-48;
			}else if(uart_RcvString[9]=='%')
			{
				red_pwn=10*(uart_RcvString[5]-48)+uart_RcvString[7]-48;
			}else if(uart_RcvString[11]=='%')
			{
				red_pwn=100;
			}

		}else if(uart_RcvString[1]=='B')
		{
			if(uart_RcvString[7]=='%')
			{
				blue_pwn=uart_RcvString[5]-48;
			}else if(uart_RcvString[9]=='%')
			{
				blue_pwn=10*(uart_RcvString[5]-48)+uart_RcvString[7]-48;
			}else if(uart_RcvString[11]=='%')
			{
				blue_pwn=100;
			}
		}else if(uart_RcvString[1]=='G')
		{
			if(uart_RcvString[7]=='%')
			{
				green_pwn=uart_RcvString[5]-48;
			}else if(uart_RcvString[9]=='%')
			{
				green_pwn=10*(uart_RcvString[5]-48)+uart_RcvString[7]-48;
			}else if(uart_RcvString[11]=='%')
			{
				green_pwn=100;
			}
		}

		uart_RcvString_Num=0;
	}
	else
	{
		uart_RcvString[uart_RcvString_Num]=RcvChar;
		uart_RcvString_Num++;//����û�����꣬��һֱ����ȥ���浽��������
	}

	/****************С�ױ�д*****************/
}

/********************************************
 * ��������uart0_init(UART_BautRate baud_rate )
 * ������;����ʼ������0
 * ������baud_rate
 * �������ã����ò�����
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void ICACHE_FLASH_ATTR
uart0_init(UART_BautRate baud_rate )
{
	//�ȴ�����0��FIFOΪ��
    UART_WaitTxFifoEmpty(UART0);
    //���ڳ�ʼ���ṹ��
    UART_ConfigTypeDef uart_config;

    uart_config.baud_rate    = baud_rate;
    uart_config.data_bits     = UART_WordLength_8b;
    uart_config.parity          = USART_Parity_None;
    uart_config.stop_bits     = USART_StopBits_1;
    uart_config.flow_ctrl      = USART_HardwareFlowControl_None;
    uart_config.UART_RxFlowThresh = 120;
    uart_config.UART_InverseMask = UART_None_Inverse;
    UART_ParamConfig(UART0, &uart_config);//��ʼ������0
    ETS_UART_INTR_ENABLE();

    //�������ڽ����ж�
    UART_intr_handler_register(uart0_rx_intr_handler);
}

/********************************************
 * ��������uart_init_new_uart1(UART_BautRate baud_rate )
 * ������;����ʼ������1
 * ������baud_rate
 * �������ã����ò�����
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void ICACHE_FLASH_ATTR
uart_init_new_uart1(UART_BautRate baud_rate)
{
	//�ȴ�����0��FIFOΪ��
    UART_WaitTxFifoEmpty(UART1);
    //���ڳ�ʼ���ṹ��
    UART_ConfigTypeDef uart_config;

    uart_config.baud_rate    = baud_rate;
    uart_config.data_bits     = UART_WordLength_8b;
    uart_config.parity          = USART_Parity_None;
    uart_config.stop_bits     = USART_StopBits_1;
    uart_config.flow_ctrl      = USART_HardwareFlowControl_None;
    uart_config.UART_RxFlowThresh = 120;
    uart_config.UART_InverseMask = UART_None_Inverse;
    UART_ParamConfig(UART1, &uart_config);
    return;
}

void uart_send(const et_char *str,et_int32 size)
{
	et_int32 i = 0;
    for(i = 0; i < size;i++)
	{
        uart_tx_one_char(UART0, *str++);
    }
}
/********************************************
 * ��������uart0_tx_buffer(uint8 *buf, uint16 len)
 * ������;����������
 * ����1��buf
 * ����1���ã�����ָ��
 * ����1��len
 * ����1���ã����鳤��
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void ICACHE_FLASH_ATTR
uart0_tx_buffer(uint8 *buf, uint16 len)
{
  uint16 i;

  for (i = 0; i < len; i++)
  {
    uart_tx_one_char(UART0, buf[i]);
  }
}

/********************************************
 * ��������uart0_tx_SendStr(uint8 *buf)
 * ������;�������ַ���
 * ������buf
 * �������ã�����ָ��
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void ICACHE_FLASH_ATTR
uart0_tx_SendStr(uint8 *buf)
{
   while(*buf!='\0')
   {
	  uart0_tx_buffer(buf++,1);
   }
}

/********************************************
 * ��������uart0_tx_SendNum(uint32 num)
 * ������;������������
 * ������num
 * �������ã�Ҫ���͵���
 * �޸�ʱ�䣺2017-1-14
 * �޸��ˣ�С��
 ********************************************/
void ICACHE_FLASH_ATTR
uart0_tx_SendNum(uint32 num)
{
	uint8   buf[10];
	uint32  numTmp = num;
	int8   i=0;
	while(numTmp)
	{
	  numTmp=numTmp/10;
	  i++;
	}
	buf[i--]='\0';
	for(;i>=0;i--)
	{
	  buf[i]=num%10 + '0';
	  num/=10;
	}
	uart0_tx_SendStr(buf);
}


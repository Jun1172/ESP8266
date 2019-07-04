#include "driver/ir.h"
#include "espressif/esp_common.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "driver/hw_timer.h"

/*Generally speaking , this is a ir tx demo of IR tx function( NEC CODE ,32 BIT LENGTH)*/

typedef enum{
	IR_TX_ADDR_P,
	IR_TX_ADDR_N,
	IR_TX_DATA_P,
	IR_TX_DATA_N,
}IR_TX_SUB_STATE;

typedef enum{
     TX_BIT_CARRIER,
     TX_BIT_LOW,
}TX_BIT_STA;

//tx parameter
IR_TX_STATE ir_tx_state=IR_TX_IDLE;
ir_tx_msg_t tx_msg;

//rx parameter
LOCAL et_uint32 t0=0;
LOCAL et_int32 ir_state=IR_NEC_STATE_IDLE;
LOCAL et_uint8 ir_rdy=0;
LOCAL et_int32 ir_repeat=0;
LOCAL et_uint32 ir_cmd=0;
LOCAL et_int32 cnt=0;
ring_buf_t ir_rx_ring_buf;
et_uint8 ir_rx_buf[RX_RCV_LEN];

set_tx_data(et_uint8 tx_addr, et_uint8 tx_cmd,et_uint16 tx_rep)
{
	tx_msg.ir_tx_addr = tx_addr;
	tx_msg.ir_tx_data = tx_cmd;
	tx_msg.ir_tx_rep = tx_rep;
}


IR_TX_STATE get_ir_tx_status()
{
	return ir_tx_state;
}

void ir_tx_func(ir_tx_msg_t *ir_msg)
{
	if(ir_msg == NULL)
		return;
	
	set_tx_data(ir_msg->ir_tx_addr, ir_msg->ir_tx_data, ir_msg->ir_tx_rep);
	os_printf("addr:%02x;cmd:%02x;repeat:%d\n", ir_msg->ir_tx_addr, ir_msg->ir_tx_data, ir_msg->ir_tx_rep);

	if(ir_tx_state == IR_TX_IDLE)
	{
		ir_tx_handler();
	}
	else 
	{
		os_printf("ir is busy!wait\n");
	}
}

/******************************************************************************
 * FunctionName : gen_carrier_clk
 * Description  : gen 38khz carrier clk
 * Parameters   : NONE
 * Returns      :   NONE
 * TX IO:GPIO14, GPIO14:I2S:WS. if want to get I2S:WS 38k output,the divide parameter 
 * should be 160MHZ/38K/(2 * (16 + 1)) =  123.8 = 2  * 62
*******************************************************************************/
void ICACHE_FLASH_ATTR
gen_carrier_clk()
{
	//ENABLE I2S CLK SOURCE
	rom_i2c_writeReg_Mask(i2c_bbpll, i2c_bbpll_hostid, i2c_bbpll_en_audio_clock_out, i2c_bbpll_en_audio_clock_out_msb, i2c_bbpll_en_audio_clock_out_lsb, 1);
	//CONFIG AS I2S 

	WRITE_PERI_REG(I2SCONF,  READ_PERI_REG(I2SCONF) & 0xf0000fff|
                    (((62&I2S_BCK_DIV_NUM)<<I2S_BCK_DIV_NUM_S)|  
                    ((2&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S)|   
                    ((1&I2S_BITS_MOD) << I2S_BITS_MOD_S)));

	WRITE_PERI_REG(IR_GPIO_OUT_MUX, (READ_PERI_REG(IR_GPIO_OUT_MUX)&0xfffffe0f)| (0x1<<4) );
	WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x2<<8)) ;//i2s rx  start
}

/******************************************************************************
 * FunctionName : ir_tx_carrier_clr
 * Description  : stop 38khz carrier clk and output low
 * Parameters   : NONE
 * Returns      :   NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
ir_tx_carrier_clr()
{
	CLEAR_PERI_REG_MASK(I2SCONF,I2S_I2S_RX_START); //i2s clk stop
	PIN_FUNC_SELECT(IR_GPIO_OUT_MUX, IR_GPIO_OUT_FUNC); 
	GPIO_OUTPUT_SET(IR_GPIO_OUT_NUM, 0);
}

/*
The standard NEC ir code is:
addr + ~addr + cmd + ~cmd
*/

void ir_tx_handler(void)  
{
	et_uint32 t_expire=0;
	static et_uint32 rep_expire_us=110000;//for nec 32bit mode
	static et_uint16 data_tmp=0;
	static et_uint8 ir_tx_bit_num=0;
	static  et_uint8 ir_bit_state=TX_BIT_CARRIER;

	TM1_EDGE_INT_DISABLE();
	switch(ir_tx_state)
	{
		case IR_TX_IDLE:
		{
			gen_carrier_clk();
			ir_tx_state = IR_TX_HEADER;
			hw_timer_arm(IR_NEC_HEADER_HIGH_US);
			TM1_EDGE_INT_ENABLE();
		}
			break;

		case IR_TX_HEADER:
		{
			ir_tx_carrier_clr();
			hw_timer_arm(IR_NEC_HEADER_LOW_US);
			ir_tx_state = IR_TX_DATA;
			ir_bit_state = TX_BIT_CARRIER;
			data_tmp = tx_msg.ir_tx_addr;
			rep_expire_us -= 13500;
			TM1_EDGE_INT_ENABLE();
		}
			break;

	        case IR_TX_DATA:
	        {
			if(ir_bit_state == TX_BIT_CARRIER)
			{
				t_expire = IR_NEC_DATA_HIGH_US;
				ir_bit_state = TX_BIT_LOW;
				gen_carrier_clk();
			}
			else if(ir_bit_state == TX_BIT_LOW)
			{
				ir_tx_carrier_clr();
				if((data_tmp >> (ir_tx_bit_num % IR_NEC_BIT_NUM)) & 0x1)
				{
					t_expire = IR_NEC_DATA_LOW_1_US;
				} else
				{
					t_expire = IR_NEC_DATA_LOW_0_US; 
				}
				
				ir_tx_bit_num++;
				if(ir_tx_bit_num == IR_NEC_BIT_NUM )
				{
					data_tmp	 = (~(tx_msg.ir_tx_addr));
				}
				else if(ir_tx_bit_num == IR_NEC_BIT_NUM * 2 )
				{
					data_tmp = tx_msg.ir_tx_data;
				}
				else if(ir_tx_bit_num == IR_NEC_BIT_NUM  * 3)
				{
					data_tmp = (~(tx_msg.ir_tx_data));	
				}
				else if((ir_tx_bit_num == (IR_NEC_BIT_NUM * 4 + 1)))
				{
					//clean up state for next or for repeat
					ir_tx_state = IR_TX_IDLE;
					ir_tx_bit_num = 0;
					ir_bit_state = TX_BIT_CARRIER;
					TM1_EDGE_INT_DISABLE();
					if(tx_msg.ir_tx_rep > 0)
					{   
						t_expire = (rep_expire_us - 5); 
						hw_timer_arm(t_expire);
						TM1_EDGE_INT_ENABLE();
						rep_expire_us = IR_NEC_REP_CYCLE;
						 ir_tx_state = IR_TX_REP;
					}
					else
					{
						rep_expire_us = IR_NEC_REP_CYCLE;
						ir_tx_state = IR_TX_IDLE;
					}
					break;
				}
				ir_bit_state = TX_BIT_CARRIER;
			}
				rep_expire_us -= t_expire;
				hw_timer_arm(t_expire);
				TM1_EDGE_INT_ENABLE();
			}
				 break;

			case IR_TX_REP:
			{
				if(tx_msg.ir_tx_rep > 0)
				{   
					if(ir_tx_bit_num == 0)
					{
						gen_carrier_clk();
						t_expire = IR_NEC_HEADER_HIGH_US ; 
					}
					else if(ir_tx_bit_num == 1)
					{
						ir_tx_carrier_clr();
						t_expire = IR_NEC_D1_TM_US ; 
					}
					else if(ir_tx_bit_num == 2)
					{
						gen_carrier_clk();
						t_expire = IR_NEC_DATA_HIGH_US; 
					}
					else if(ir_tx_bit_num == 3) 
					{
						ir_tx_carrier_clr();
						tx_msg.ir_tx_rep--;
						if(tx_msg.ir_tx_rep > 0)
						{
						    t_expire = rep_expire_us ; 
						    rep_expire_us = IR_NEC_REP_CYCLE;
						}
						else
						{
						    ir_tx_bit_num = 0;
						    rep_expire_us = IR_NEC_REP_CYCLE;
						    ir_tx_state = IR_TX_IDLE;
						    ir_bit_state = TX_BIT_CARRIER;
							TM1_EDGE_INT_DISABLE();
						    break;
						}
					}
					
					ir_tx_bit_num++;//bit num reuse for repeat wave form
					if(ir_tx_bit_num == 4) 
					{
						ir_tx_bit_num = 0;
						rep_expire_us = IR_NEC_REP_CYCLE;
					}
					else
					{
						rep_expire_us -= t_expire;
					}
					hw_timer_arm(t_expire);//143
					TM1_EDGE_INT_ENABLE();
				}
			}
				break;
				
			default: 
				break;
	}
}

/******************************************************************************
 * FunctionName : nec_code_check
 * Description  : check whether the ir cmd and addr obey the protocol
 * Parameters   : u32 nec_code : nec ir code that received
 * Returns      :   true : cmd check ok  ;  false : cmd check fail
*******************************************************************************/
//check if is a legal nec code 
et_int32 nec_code_check(et_uint32 nec_code)
{   
	et_uint8 addr1, addr2, cmd1, cmd2;

	cmd2 = (nec_code >> 24) & 0xff;     
	cmd1 = (nec_code >> 16) & 0xff;   
	addr2 = (nec_code >> 8) & 0xff;   
	addr1 = (nec_code >> 0) & 0xff;   
	if((cmd1 == ((~cmd2)&0xff)) && (addr1 == ((~addr2) & 0xff)))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/******************************************************************************
 * FunctionName : ir_intr_handler
 * Description     : ir rx state machine via gpio intr
 * Parameters    : NONE
 * Returns         :  NONE
*******************************************************************************/
LOCAL void ir_intr_handler()
{
	et_uint32 status, t_h, d, t_st;
	static et_uint8 rep_flg;

	d = GPIO_INPUT_GET(GPIO_ID_PIN(IR_GPIO_IN_NUM)) ;
	status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	t_h = system_get_time() - t0;
	t0 = system_get_time();
	//clear interrupt status 
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);

	switch(ir_state)
	{
		case IR_NEC_STATE_IDLE:  
		{
			if(t_h < IR_NEC_TM_PRE_US + IR_NEC_MAGIN_US && t_h > IR_NEC_TM_PRE_US -IR_NEC_MAGIN_US)
			{
		  		ir_state = IR_NEC_STATE_CMD;
			}
		}
			break;
	    
		case IR_NEC_STATE_CMD:
		{
			if(t_h < IR_NEC_D1_TM_US + IR_NEC_MAGIN_US && t_h > IR_NEC_D1_TM_US -IR_NEC_MAGIN_US)
			{
				ir_cmd = (ir_cmd >> 1) |(0x1 << (IR_NEC_BIT_NUM * 4 - 1));
				cnt++;
			}
			else if(t_h < IR_NEC_D0_TM_US + IR_NEC_MAGIN_US && t_h > IR_NEC_D0_TM_US -IR_NEC_MAGIN_US)
			{
				ir_cmd =  (ir_cmd >> 1) |(0x0 << (IR_NEC_BIT_NUM * 4 - 1));
				cnt++;
			}
			else
			{
				goto RESET_STATUS;
			}

			if(cnt == IR_NEC_BIT_NUM * 4)
			{
				if(!nec_code_check(ir_cmd))
				{
					ring_buf_put(&ir_rx_ring_buf, (ir_cmd >> 16) & 0xff);
					ir_state = IR_NEC_STATE_REPEAT;
					rep_flg = 0;
				}
				else
				{
					goto RESET_STATUS;    
				}
			}
		}
	       	break;
			
		case IR_NEC_STATE_REPEAT:
		{
			if(rep_flg == 0)
			{
				//when repeat code,according to NEC protocol, within 110ms, header, addr, cmd are send, 
				//after them being send,  send "1" until total time is 110ms ,then send repeat code,
				//from cmd to repeat code, time is from (110 -9 - 4.5 -1.12 * 32 = 60.66) to (110 -9 - 4.5 -2.25 * 32 = 24.5)
				if(t_h > IR_NEC_REP_TM1_US && t_h < IR_NEC_REP_TM1_US * 8)          //110 - 9 - 4.5 - 1.12 * 32 <  t_h 
				{
					rep_flg = 1;
				}
				else
				{
					goto RESET_STATUS;
				}
			}
			else if(rep_flg == 1)
			{
				if(t_h < IR_NEC_REP_TM2_US + IR_NEC_MAGIN_US && t_h > IR_NEC_REP_TM2_US -IR_NEC_MAGIN_US)
				{
					ring_buf_put(&ir_rx_ring_buf, (ir_cmd >> 16) & 0xff);
					ir_repeat++;
					rep_flg = 0;
				}
				else
				{
					goto RESET_STATUS;
				}
			}
		}
			break;
			
		default: 
			break;

		RESET_STATUS:
		ir_state = IR_NEC_STATE_IDLE;
		cnt=0;
		ir_cmd=0;
		ir_repeat=0;
		rep_flg=0;
	    
	}
}



/******************************************************************************
 * FunctionName : ir_rx_intr_handler
 * Description     : ir rx state machine via gpio intr
 * Parameters    : NONE
 * Returns         :  NONE
******************************************************************************/
LOCAL void ir_rx_intr_handler(void *para)
{
	et_int32  status;

	status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	if((status >> IR_GPIO_IN_NUM) & BIT0)
	{
		ir_intr_handler();
	}
	else
	{
		os_printf("gpio num mismached\n");
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status);
	}
}

void ir_rx_task(void *pvParameters)
{
	et_uint8 ir_data;

	while(1)
	{
		if(ir_rx_ring_buf.fill_cnt > 0)
		{
			ring_buf_get(&ir_rx_ring_buf, &ir_data, 1);
			os_printf("ir_data = %02xh\n", ir_data);
		}
		delay_ms(500);
	}
}

LOCAL void ir_rx_gpio_init()
{
	//Select pin as gpio
	PIN_FUNC_SELECT(IR_GPIO_IN_MUX, IR_GPIO_IN_FUNC);
	//set GPIO as input
	GPIO_DIS_OUTPUT(IR_GPIO_IN_NUM);
	PIN_PULLUP_DIS(IR_GPIO_IN_MUX);
	//clear interrupt status
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(IR_GPIO_IN_NUM));
	//set negitive edge interrupt
	gpio_pin_intr_state_set(IR_GPIO_IN_NUM, GPIO_PIN_INTR_NEGEDGE);
	//register interrupt handler
	ETS_GPIO_INTR_ATTACH(ir_rx_intr_handler, NULL);
	//enable gpio interrupt
	ETS_GPIO_INTR_ENABLE();
}

void ir_tx_init(et_uint8 req)
{
	hw_timer_init(req);
       hw_timer_set_func(ir_tx_handler);
}

/******************************************************************************
 * FunctionName : ir_rx_init
 * Description  : init gpio function, attach interrupt serv function
 * Parameters   : NONE
 * Returns      :   NONE
*******************************************************************************/
void ir_rx_init()
{
	ring_buf_init(&ir_rx_ring_buf, ir_rx_buf, sizeof(ir_rx_buf));
	ir_rx_gpio_init();
	xTaskCreate(ir_rx_task, "ir_rx_task", 256, NULL, 2, NULL);
}

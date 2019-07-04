#ifndef _IR_H
#define _IR_H
#include "et_types.h"
#include "driver/ring_buf.h"

#define RX_RCV_LEN 128

#define IR_GPIO_IN_MUX PERIPHS_IO_MUX_GPIO5_U
#define IR_GPIO_IN_NUM 5
#define IR_GPIO_IN_FUNC  FUNC_GPIO5

#define IR_GPIO_OUT_NUM 14
#define IR_GPIO_OUT_MUX PERIPHS_IO_MUX_MTMS_U
#define IR_GPIO_OUT_FUNC  FUNC_GPIO14

#define IR_POTOCOL_NEC 0
#define IR_POTOCOL_RC5 1  //not support yet 

#define IR_NEC_STATE_IDLE 0
#define IR_NEC_STATE_PRE 1
#define IR_NEC_STATE_CMD 2
#define IR_NEC_STATE_REPEAT 3

typedef enum{
	IR_TX_IDLE=0,
	IR_TX_HEADER,
	IR_TX_DATA,
	IR_TX_REP,
}IR_TX_STATE;

  #define IR_NEC_BIT_NUM 		8
  #define IR_NEC_MAGIN_US 		200
  #define IR_NEC_TM_PRE_US 		13500
  #define IR_NEC_D1_TM_US 		2250
  #define IR_NEC_D0_TM_US 		1120
  #define IR_NEC_REP_TM1_US 	20000
  #define IR_NEC_REP_TM2_US 	11250
  #define IR_NEC_REP_LOW_US 	2250
  #define IR_NEC_REP_CYCLE 	110000

  #define IR_NEC_HEADER_HIGH_US 9000
  #define IR_NEC_HEADER_LOW_US 	4500
  #define IR_NEC_DATA_HIGH_US 	560
  #define IR_NEC_DATA_LOW_1_US 	1690
  #define IR_NEC_DATA_LOW_0_US 	560

  typedef struct ir_tx_msg{
	et_uint8 ir_tx_addr;
	et_uint8 ir_tx_data;
	et_uint16 ir_tx_rep;
  }ir_tx_msg_t;

void  ir_rx_init();
void ir_rx_disable();
void ir_rx_enable();

void  ir_tx_init(et_uint8 req);
void ir_tx_handler();
void gen_carrier_clk();
#endif


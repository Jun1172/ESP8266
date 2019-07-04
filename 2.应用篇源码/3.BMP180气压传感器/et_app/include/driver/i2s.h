#include "driver/i2s_reg.h"
#include "et_types.h"
#include "driver/sdio_slv.h"

#define i2c_bbpll                                 0x67
#define i2c_bbpll_en_audio_clock_out            4
#define i2c_bbpll_en_audio_clock_out_msb        7
#define i2c_bbpll_en_audio_clock_out_lsb        7
#define i2c_bbpll_hostid                           4

#define i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)  rom_i2c_writeReg_Mask(block, host_id, reg_add, Msb, Lsb, indata)
#define i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb)  rom_i2c_readReg_Mask(block, host_id, reg_add, Msb, Lsb)
#define i2c_writeReg_Mask_def(block, reg_add, indata) \
      i2c_writeReg_Mask(block, block##_hostid,  reg_add,  reg_add##_msb,  reg_add##_lsb,  indata)
#define i2c_readReg_Mask_def(block, reg_add) \
      i2c_readReg_Mask(block, block##_hostid,  reg_add,  reg_add##_msb,  reg_add##_lsb)

#define CONF_RXLINK_ADDR(addr)		CLEAR_PERI_REG_MASK(SLC_RX_LINK,SLC_RXLINK_DESCADDR_MASK);\
	SET_PERI_REG_MASK(SLC_RX_LINK, ((et_uint32)(addr)) & SLC_RXLINK_DESCADDR_MASK)
#define CONF_TXLINK_ADDR(addr) 		CLEAR_PERI_REG_MASK(SLC_TX_LINK,SLC_TXLINK_DESCADDR_MASK);\
	SET_PERI_REG_MASK(SLC_TX_LINK, ((et_uint32)(addr)) & SLC_TXLINK_DESCADDR_MASK)

#define START_RXLINK() 	SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START)
#define START_TXLINK() 	SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START)

#define IIS_RX_BUF_LEN  512  //unit Byte
#define IIS_TX_BUF_LEN  512  //unit Byte

#ifdef I2S_RECEIVE
#define n  2
#endif

#define SAMPLE_NUM   6
#define RX_NUM 128 //unit word

//Initialize the I2S module
void i2s_init();

//create DMA buffer descriptors
void creat_one_link(et_uchar own, et_uchar eof,et_uchar sub_sof, et_uint16 size, et_uint16 length,
                       et_uint32* buf_ptr, struct sdio_queue* nxt_ptr, struct sdio_queue* i2s_queue);

//Initialize the SLC module for DMA function
void slc_init();

void slc_isr(void *para);

 void i2s_audio_init();


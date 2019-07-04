/******************************************************************************
 * 作者：长沙-嵌入式-小白
 * 修改时间：2017-1-14
 * 文件名: user_main.c
 * 用途：主函数入口
*******************************************************************************/
#include "driver/user_main.h"
#include "spi_flash.h"

//定义按键
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//定义一个时间变量
LOCAL os_timer_t timer;

char key_num=0;//按键相关变量

#define PUTC	fcmd_putc
fcmd_putc(uint8_t c)
{
	os_printf("%c", c);
}
/*****************************************************************************
 * spi flash的两个命令操作
 */
/*
 * esp8266的spi flash内存显示, 用于串口调试查看flash内容
 * @readaddr flash地址，4字节对齐
 * @elem_cnt  要读取多少元素, 1024
 * @elem_size 一个元素多宽(1,2,4字节)
 *
 * note  sfmd(0, 1024, 1)
 */
int ICACHE_FLASH_ATTR
sfmd(u32 addr, int elem_cnt, int elem_size)
{
	u32 size = (elem_cnt * elem_size) & 0xFFFFFFFC;	// 4的倍数
	uint8 *pbuf;
	uint8 status, ret;

	pbuf = (uint8 *)zalloc(size + 4);
	if (pbuf == NULL)
	{
		os_printf("memory err\n");
		return SPI_FLASH_RESULT_TIMEOUT + 1;
	}

	//read
	status = spi_flash_read(addr, (uint32 *)pbuf, size);
	if (status != SPI_FLASH_RESULT_OK)
	{
		os_printf("spi flash read err!\n");
		return status;
	}

	//display spi flash memory
	int i, j;
	const uint8_t *pcbuf = (uint8_t *)pbuf;	//读取到的flash数据
	const uint8_t *bp;
	const uint16_t *sp;
	const uint32_t *lp;

	int line_elem;//一行多少个元素
	int line_count;//有多少行
	int line_remain;//最后一行剩下多少个元素

	if ((elem_size == 1) || (elem_size == 2) || (elem_size == 4))
		;
	else
	{
		return SPI_FLASH_RESULT_TIMEOUT + 2;
	}

	line_elem = 16 / elem_size;
	line_count = (elem_cnt + line_elem - 1) / line_elem;//向上取整，不够一行算一行
	line_remain = elem_cnt % line_elem;

	PUTC('\n');

	for (j = 0; j < line_count; j++)
	{
		if (j == (line_count - 1))      /* last line */
		{
			if (line_remain != 0)
			{
				line_elem = line_remain;
			}
		}

		//dump a line, 16 bytes a line
		os_printf("%08X:", addr);		/* address */

		switch (elem_size)
		{
		case 1://1
			bp = pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				os_printf(" %02X", bp[i]);
			}
			break;
		case 2://2
			sp = (uint16_t *)pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				os_printf(" %04X", *sp++);
			}
			break;
		case 4://4
			lp = (uint32_t *)pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				os_printf(" %08X", lp[i]);
			}
			break;
		}

		PUTC(' ');
		PUTC(' ');
		bp = pcbuf;
		for (i = 0; i < line_elem * elem_size; i++) /* ASCII dump */
		{
			PUTC((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.');
		}

		PUTC('\n');

		addr += line_elem * elem_size;
		pcbuf += line_elem * elem_size;
	}

	free(pbuf);

	return 0;
}

/*
 * spi flash内存写入
 * @writeaddr  写入flash的地址, 地址随意，无需四字节对齐
 * @*pbuf   写入的内容
 * @num  写入大小, 大小随意无需四字节对齐

 * bug:pbuf如果没有4字节对齐的话，也会触发异常, 全部采用4096整扇区读写
 *     不再判断要不要擦除
 * 比如spi_flash_write(0x00300000, pbuf, 512)的pbuf没有4字节对齐将会出错
 */
int ICACHE_FLASH_ATTR
sfmw(u32 writeaddr, u8 *pbuf, u32 num)
{
	u32 secpos;
	u16 secoff;
	u16 secremain;
	u16 i;
	uint8 *pmen, *pmenalign;
	int ret = 0;
	int status;

	secpos = writeaddr / 4096; //扇区地址 0~511 for w25x16
	secoff = writeaddr % 4096; //偏移地址
	secremain = 4096 - secoff; //扇区剩余空间大小

	if (num <= secremain)
	{
		secremain = num;    //不大于4096个字节
	}
	pmen = (uint8 *)zalloc(4096 + 8);
	if (pmen == NULL)
	{
		ret = SPI_FLASH_RESULT_TIMEOUT + 1;
		return ret;			//内存错误
	}
	pmenalign = (uint8 *)((((u32)pmen) + 3) & (~3)); //四字节对齐的内存缓冲

	while (1)
	{
		status = spi_flash_read(secpos * 4096, (uint32 *)pmenalign, 4096); //读出整个扇区的内容
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}

		status = spi_flash_erase_sector(secpos);//擦除这个扇区
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}
		for (i = 0; i < secremain; i++)	 //复制用户内容
		{
			pmenalign[i + secoff] = pbuf[i];
		}
		//写入整个扇区
		status = spi_flash_write(secpos * 4096, (uint32 *)pmenalign, 4096);
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}

		if (num == secremain)
		{
			break;    //写入结束
		}
		else                        //写入未结束
		{
			secpos++;               //扇区地址增1
			secoff = 0;             //偏移位置为0
			pbuf += secremain;      //pbuf指针偏移
			writeaddr += secremain; //写地址偏移
			num -= secremain;		//字节数递减
			if (num > 4096)
			{
				secremain = 4096;    //下一个扇区还是写不完
			}
			else
			{
				secremain = num;    //下一个扇区可以写完了
			}
		}
	}

	free(pmen);
	return ret;
}

/********************************************
 * 函数名：timer_callback
 * 函数用途：定时器回调函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void timer_callback()
{
	if(KEY==0)//判断按键按下
	{
		while(KEY==0);//松手检测
		key_num++;//自加一次
		sfmw(0x40000, "JUN ", 4);
		sfmd(0x40000, 4, 1);
		uart0_tx_SendStr("\r\n读取一次\r\n");
	}
}

/********************************************
 * 函数名：user_init
 * 函数用途：主函数
 * 修改时间：2017-1-14
 * 编写：小白
 ********************************************/
void user_init(void)
{
	uart0_init(74880);  //初始化串口，设置波特率
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//初始化按键1
	uart0_tx_SendStr("\r\n初始化完成\r\n");

	// 取消（停止）定时器timer
	os_timer_disarm(&timer);
	// 将时间定时器变量timer关联到一个函数timer_callback（），这个函数会在到达定时时间时执行一次
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// 设置时间定时器变量timer的时间周期为10毫秒，第三个参数为1时，timer_callback（）只会执行一次
	os_timer_arm(&timer,5,1);//定时器回调函数
}

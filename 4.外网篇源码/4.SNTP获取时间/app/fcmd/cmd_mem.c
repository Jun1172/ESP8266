#include "fcmd.h"

#include "cmd_mem.h"
#include "c_types.h"
#include "osapi.h"
#include "spi_flash.h"
#include "flash_api.h"
#include "mem.h"

/*
 * 显示内存内容
 * addr:被dump的内存地址
 * elem_cnt:被dump多少个元素
 * elem_size:一个元素有几个字节宽
 */
void ICACHE_FLASH_ATTR
md(int addr, int elem_cnt, int elem_size)
{
	int i, j;
	const uint8_t *pcbuf = (uint8_t *)addr;
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
		return ;
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
		PRINTF("%08X:", addr);		/* address */

		switch (elem_size)
		{
		case 1://1
			bp = pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				PRINTF(" %02X", bp[i]);
			}
			break;
		case 2://2
			sp = (uint16_t *)pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				PRINTF(" %04X", *sp++);
			}
			break;
		case 4://4
			lp = (uint32_t *)pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				PRINTF(" %08X", lp[i]);
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
}

/*
 * 比较俩块内存是否相等
 * mem1:第一块内存地址
 * mem2:第二块
 * elem_cnt:比较多少个元素
 * elem_size:一个元素是几个字节
 */
int ICACHE_FLASH_ATTR
cmp(void *mem1, void *mem2, int elem_cnt, int elem_size)
{
	uint32_t	ngood = 0;
	int     	rcode = 0;
	uint8_t		*addr1 = (uint8_t *)mem1, *addr2 = (uint8_t *)mem2;

	while (elem_cnt-- > 0)
	{
		if (elem_size == 4)
		{
			uint32_t word1 = *(uint32_t *)addr1;
			uint32_t word2 = *(uint32_t *)addr2;

			if (word1 != word2)
			{
				PRINTF("word at 0x%08x (0x%08x) "
				       "!= word at 0x%08x (0x%08x)\n",
				       (int)addr1, word1, (int)addr2, word2);
				rcode = 1;
				break;
			}
		}
		else if (elem_size == 2)
		{
			uint16_t hword1 = *(uint16_t *)addr1;
			uint16_t hword2 = *(uint16_t *)addr2;

			if (hword1 != hword2)
			{
				PRINTF("halfword at 0x%08x (0x%04x) "
				       "!= halfword at 0x%08x (0x%04x)\n",
				       (int)addr1, hword1, (int)addr2, hword2);
				rcode = 1;
				break;
			}
		}
		else if (elem_size == 1)
		{
			uint8_t byte1 = *(uint8_t *)addr1;
			uint8_t byte2 = *(uint8_t *)addr2;

			if (byte1 != byte2)
			{
				PRINTF("byte at 0x%08x (0x%02x) "
				       "!= byte at 0x%08x (0x%02x)\n",
				       (int)addr1, byte1, (int)addr2, byte2);
				rcode = 1;
				break;
			}
		}

		ngood++;

		addr1 += elem_size;
		addr2 += elem_size;
	}
	PRINTF("Total of %d %s%s were the same\n",
	       ngood,
	       (elem_size == 4) ? ("word") : ((elem_size == 2) ? "halfword" : "byte"),
	       ngood == 1 ? "" : "s");

	return rcode;
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

	pbuf = (uint8 *)os_zalloc(size + 4);
	if (pbuf == NULL)
	{
		os_printf("memory err\n");
		return SPI_FLASH_RESULT_TIMEOUT + 1;
	}

	//read
	status = flash_safe_read(addr, (uint32 *)pbuf, size);
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
		PRINTF("%08X:", addr);		/* address */

		switch (elem_size)
		{
		case 1://1
			bp = pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				PRINTF(" %02X", bp[i]);
			}
			break;
		case 2://2
			sp = (uint16_t *)pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				PRINTF(" %04X", *sp++);
			}
			break;
		case 4://4
			lp = (uint32_t *)pcbuf;
			for (i = 0; i < line_elem; i++)		/* Hexdecimal dump */
			{
				PRINTF(" %08X", lp[i]);
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

	os_free(pbuf);

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
	pmen = (uint8 *)os_zalloc(4096 + 8);
	if (pmen == NULL)
	{
		ret = SPI_FLASH_RESULT_TIMEOUT + 1;
		return ret;			//内存错误
	}
	pmenalign = (uint8 *)((((u32)pmen) + 3) & (~3)); //四字节对齐的内存缓冲

	while (1)
	{
		status = flash_safe_read(secpos * 4096, (uint32 *)pmenalign, 4096); //读出整个扇区的内容
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}
#if 1
		status = flash_safe_erase_sector(secpos);//擦除这个扇区
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
		status = flash_safe_write(secpos * 4096, (uint32 *)pmenalign, 4096);
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}
#else
		for (i = 0; i < secremain; i++) //检查数据是否需要擦除
		{
			if (pmenalign[secoff + i] != 0XFF)
			{
				break;
			}
		}
		if (i < secremain)
		{
			//需要擦除
			status = flash_safe_erase_sector(secpos);//擦除这个扇区
			if (status != SPI_FLASH_RESULT_OK)
			{
				ret = status;
				break;
			}

			for (i = 0; i < secremain; i++)	 //复制用户内容
			{
				pmenalign[i + secoff] = pbuf[i];
			}

			status = flash_safe_write(secpos * 4096, (uint32 *)pmenalign, 4096); //写入整个扇区
			if (status != SPI_FLASH_RESULT_OK)
			{
				ret = status;
				break;
			}
		}
		else
		{
			//无需擦除，直接写入扇区剩余区间. pbuf没有4字节对齐将会异常
			os_printf("flash_safe_write:%08x, %s(%08x), %d\n", writeaddr, pbuf, pbuf, secremain);

			status = flash_safe_write(writeaddr, (uint32 *)pbuf, secremain);
			if (status != SPI_FLASH_RESULT_OK)
			{
				ret = status;
				break;
			}
		}
#endif
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

	os_free(pmen);
	return ret;
}



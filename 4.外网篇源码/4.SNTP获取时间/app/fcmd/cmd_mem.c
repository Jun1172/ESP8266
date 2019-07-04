#include "fcmd.h"

#include "cmd_mem.h"
#include "c_types.h"
#include "osapi.h"
#include "spi_flash.h"
#include "flash_api.h"
#include "mem.h"

/*
 * ��ʾ�ڴ�����
 * addr:��dump���ڴ��ַ
 * elem_cnt:��dump���ٸ�Ԫ��
 * elem_size:һ��Ԫ���м����ֽڿ�
 */
void ICACHE_FLASH_ATTR
md(int addr, int elem_cnt, int elem_size)
{
	int i, j;
	const uint8_t *pcbuf = (uint8_t *)addr;
	const uint8_t *bp;
	const uint16_t *sp;
	const uint32_t *lp;

	int line_elem;//һ�ж��ٸ�Ԫ��
	int line_count;//�ж�����
	int line_remain;//���һ��ʣ�¶��ٸ�Ԫ��

	if ((elem_size == 1) || (elem_size == 2) || (elem_size == 4))
		;
	else
	{
		return ;
	}

	line_elem = 16 / elem_size;
	line_count = (elem_cnt + line_elem - 1) / line_elem;//����ȡ��������һ����һ��
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
 * �Ƚ������ڴ��Ƿ����
 * mem1:��һ���ڴ��ַ
 * mem2:�ڶ���
 * elem_cnt:�Ƚ϶��ٸ�Ԫ��
 * elem_size:һ��Ԫ���Ǽ����ֽ�
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
 * spi flash�������������
 */
/*
 * esp8266��spi flash�ڴ���ʾ, ���ڴ��ڵ��Բ鿴flash����
 * @readaddr flash��ַ��4�ֽڶ���
 * @elem_cnt  Ҫ��ȡ����Ԫ��, 1024
 * @elem_size һ��Ԫ�ض��(1,2,4�ֽ�)
 *
 * note  sfmd(0, 1024, 1)
 */
int ICACHE_FLASH_ATTR
sfmd(u32 addr, int elem_cnt, int elem_size)
{
	u32 size = (elem_cnt * elem_size) & 0xFFFFFFFC;	// 4�ı���
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
	const uint8_t *pcbuf = (uint8_t *)pbuf;	//��ȡ����flash����
	const uint8_t *bp;
	const uint16_t *sp;
	const uint32_t *lp;

	int line_elem;//һ�ж��ٸ�Ԫ��
	int line_count;//�ж�����
	int line_remain;//���һ��ʣ�¶��ٸ�Ԫ��

	if ((elem_size == 1) || (elem_size == 2) || (elem_size == 4))
		;
	else
	{
		return SPI_FLASH_RESULT_TIMEOUT + 2;
	}

	line_elem = 16 / elem_size;
	line_count = (elem_cnt + line_elem - 1) / line_elem;//����ȡ��������һ����һ��
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
 * spi flash�ڴ�д��
 * @writeaddr  д��flash�ĵ�ַ, ��ַ���⣬�������ֽڶ���
 * @*pbuf   д�������
 * @num  д���С, ��С�����������ֽڶ���

 * bug:pbuf���û��4�ֽڶ���Ļ���Ҳ�ᴥ���쳣, ȫ������4096��������д
 *     �����ж�Ҫ��Ҫ����
 * ����spi_flash_write(0x00300000, pbuf, 512)��pbufû��4�ֽڶ��뽫�����
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

	secpos = writeaddr / 4096; //������ַ 0~511 for w25x16
	secoff = writeaddr % 4096; //ƫ�Ƶ�ַ
	secremain = 4096 - secoff; //����ʣ��ռ��С

	if (num <= secremain)
	{
		secremain = num;    //������4096���ֽ�
	}
	pmen = (uint8 *)os_zalloc(4096 + 8);
	if (pmen == NULL)
	{
		ret = SPI_FLASH_RESULT_TIMEOUT + 1;
		return ret;			//�ڴ����
	}
	pmenalign = (uint8 *)((((u32)pmen) + 3) & (~3)); //���ֽڶ�����ڴ滺��

	while (1)
	{
		status = flash_safe_read(secpos * 4096, (uint32 *)pmenalign, 4096); //������������������
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}
#if 1
		status = flash_safe_erase_sector(secpos);//�����������
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}
		for (i = 0; i < secremain; i++)	 //�����û�����
		{
			pmenalign[i + secoff] = pbuf[i];
		}
		//д����������
		status = flash_safe_write(secpos * 4096, (uint32 *)pmenalign, 4096);
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}
#else
		for (i = 0; i < secremain; i++) //��������Ƿ���Ҫ����
		{
			if (pmenalign[secoff + i] != 0XFF)
			{
				break;
			}
		}
		if (i < secremain)
		{
			//��Ҫ����
			status = flash_safe_erase_sector(secpos);//�����������
			if (status != SPI_FLASH_RESULT_OK)
			{
				ret = status;
				break;
			}

			for (i = 0; i < secremain; i++)	 //�����û�����
			{
				pmenalign[i + secoff] = pbuf[i];
			}

			status = flash_safe_write(secpos * 4096, (uint32 *)pmenalign, 4096); //д����������
			if (status != SPI_FLASH_RESULT_OK)
			{
				ret = status;
				break;
			}
		}
		else
		{
			//���������ֱ��д������ʣ������. pbufû��4�ֽڶ��뽫���쳣
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
			break;    //д�����
		}
		else                        //д��δ����
		{
			secpos++;               //������ַ��1
			secoff = 0;             //ƫ��λ��Ϊ0
			pbuf += secremain;      //pbufָ��ƫ��
			writeaddr += secremain; //д��ַƫ��
			num -= secremain;		//�ֽ����ݼ�
			if (num > 4096)
			{
				secremain = 4096;    //��һ����������д����
			}
			else
			{
				secremain = num;    //��һ����������д����
			}
		}
	}

	os_free(pmen);
	return ret;
}



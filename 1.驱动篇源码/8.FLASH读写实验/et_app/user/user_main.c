/******************************************************************************
 * ���ߣ���ɳ-Ƕ��ʽ-С��
 * �޸�ʱ�䣺2017-1-14
 * �ļ���: user_main.c
 * ��;�����������
*******************************************************************************/
#include "driver/user_main.h"
#include "spi_flash.h"

//���尴��
#define KEY  GPIO_INPUT_GET(GPIO_ID_PIN(0))

//����һ��ʱ�����
LOCAL os_timer_t timer;

char key_num=0;//������ر���

#define PUTC	fcmd_putc
fcmd_putc(uint8_t c)
{
	os_printf("%c", c);
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
	pmen = (uint8 *)zalloc(4096 + 8);
	if (pmen == NULL)
	{
		ret = SPI_FLASH_RESULT_TIMEOUT + 1;
		return ret;			//�ڴ����
	}
	pmenalign = (uint8 *)((((u32)pmen) + 3) & (~3)); //���ֽڶ�����ڴ滺��

	while (1)
	{
		status = spi_flash_read(secpos * 4096, (uint32 *)pmenalign, 4096); //������������������
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}

		status = spi_flash_erase_sector(secpos);//�����������
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
		status = spi_flash_write(secpos * 4096, (uint32 *)pmenalign, 4096);
		if (status != SPI_FLASH_RESULT_OK)
		{
			ret = status;
			break;
		}

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

	free(pmen);
	return ret;
}

/********************************************
 * ��������timer_callback
 * ������;����ʱ���ص�����
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void timer_callback()
{
	if(KEY==0)//�жϰ�������
	{
		while(KEY==0);//���ּ��
		key_num++;//�Լ�һ��
		sfmw(0x40000, "JUN ", 4);
		sfmd(0x40000, 4, 1);
		uart0_tx_SendStr("\r\n��ȡһ��\r\n");
	}
}

/********************************************
 * ��������user_init
 * ������;��������
 * �޸�ʱ�䣺2017-1-14
 * ��д��С��
 ********************************************/
void user_init(void)
{
	uart0_init(74880);  //��ʼ�����ڣ����ò�����
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);//��ʼ������1
	uart0_tx_SendStr("\r\n��ʼ�����\r\n");

	// ȡ����ֹͣ����ʱ��timer
	os_timer_disarm(&timer);
	// ��ʱ�䶨ʱ������timer������һ������timer_callback����������������ڵ��ﶨʱʱ��ʱִ��һ��
	os_timer_setfn(&timer,(os_timer_func_t *)timer_callback,NULL);
	// ����ʱ�䶨ʱ������timer��ʱ������Ϊ10���룬����������Ϊ1ʱ��timer_callback����ֻ��ִ��һ��
	os_timer_arm(&timer,5,1);//��ʱ���ص�����
}

/*******************************************************************************
--º¯ÊıÃüÁîĞĞV0.03
--fcmd.c
--zrpeng
--2015-3-5
--
--note:
	¸ÃÄ£¿éÖ÷ÒªÓÃÓÚµ÷ÊÔÓÃÍ¾£¬¿ÉÒÔ·½±ãµÄ½øĞĞÊÖ¶¯²âÊÔ£¬Ïòµ÷ÓÃCÓïÑÔº¯ÊıÒ»ÑùÔÚÔËĞĞ½×¶ÎÖ´ĞĞ
	ÏëÒªÖ´ĞĞµÄCº¯Êı¡£
	±ÈÈç:µ÷ÊÔµ¥Æ¬»úµÄPWMÊ±£¬Ö´ĞĞint timer_pwm_set(int pwm)
          ÊäÈëtimer_pwm_set(88)£¬¾Í¿ÉÒÔ¸Ä±äÕ¼¿Õ±ÈÁË
	¸ÃÄ£¿é´æÔÚÒ»¶¨µÄÎ£ÏÕ£ ĞèÒª¶ÔCºÍ¼Ä´æÆ÷²ÎÊı´«µİÓĞÒ»¶¨µÄÁË½â.

--history:
	0.01ÔËĞĞÕı³££¬Ö»ÄÜÖ§³Öint32_tÀàĞÍ²ÎÊı
	0.02ĞÂÔö¼Ó×Ö·û´®ÀàĞÍµÄÖ§³Ö,eg:disp(50,-60, "ADC:  mV", 10, "89"),
		µ÷»»ÁËCmdTblÄÚ²¿³ÉÔ±µÄË³Ğò£¬×Ö·û´®ÔÚÇ°£¬º¯ÊıÖ¸ÕëÔÚºó
		Ôö¼ÓÁËÄÚ´æÏÔÊ¾ÃüÁî,c¿âº¯Êı¶¼¿ÉÒÔÕı³£µ÷ÓÃ²âÊÔ
		Ä¿Ç°ÔËĞĞÕı³£
	0.03Ôö¼Ó¶Ô¿É±ä²ÎÊıµÄÖ§³Ö,eg:int printf(const char *fmt, ...)
*******************************************************************************/
#include "fcmd.h"


#define PARAMS_NUM  10 //º¯ÊıÖ§³Ö10¸ö²ÎÊı
#define SYSTEM_CMD  3  //ÏµÍ³ÃüÁîµÄ¸öÊı, CmdTblÔö¼ÓÏµÍ³ÃüÁîµÄÍ¬Ê±Ò²ÒªÔö¼ÓÕâ¸ö±äÁ¿

#define _args_t  int	//²ÎÊıÀàĞÍ£¬¶ÔÓÚ16bitºÍ8bitµ¥Æ¬»úÒª×¢Òâ

/*******************************************************************************
 * Ò»¸öº¯ÊıÃüÁîĞĞ½á¹¹
 */
typedef struct
{
	char *fname;
	void (*pfunc)(void);
} CmdTbl_t;

#include "fcmd_cfg.h"


/*******************************************************************************
 * ¾²Ì¬º¯ÊıÉùÃ÷
 */
LOCAL void system_cmd_exe(uint8_t n);
LOCAL int8_t get_args_num(uint8_t *str, uint8_t *key);
LOCAL void get_args(uint8_t *str, uint8_t *key, _args_t *args);
LOCAL uint8_t *get_fname(uint8_t *str, uint8_t *len);

void ICACHE_FLASH_ATTR
fcmd_putc(uint8_t c)
{
	PRINTF("%c", c);
}
/*
 * Ö´ĞĞÄ£¿é¶¨ÒåµÄÖ¸Áî
 * @n Ö¸ÁîĞòºÅ£¬Ö¸ÁîCmdTblÖĞµÄĞòºÅ
 */
LOCAL void ICACHE_FLASH_ATTR
system_cmd_exe(uint8_t n)
{
	uint8_t i;

	switch (n)
	{
	case 0://ls:ÁĞ³öËùÓĞµÄÃüÁî
		PRINTF("------------- function list --------------\n");
		for (i = 0; i < CmdTblSize; i++)
		{
			PRINTF("%s\n", CmdTbl[i].fname);
		}
		PRINTF("-------------------------------------------\n");
		break;

	case 1://addr:ÁĞ³öËùÓĞº¯ÊıÖ¸ÕëµÄµØÖ·
		PRINTF("------------------------------------------\n");
		for (i = 0; i < CmdTblSize; i++)
		{
			PRINTF("0x%08x: %s\n", (int)CmdTbl[i].pfunc, CmdTbl[i].fname);
		}
		PRINTF("------------------------------------------\n");
		break;

	case 2:
		PRINTF(
		    "---------------------------------------------\n"
		    "fcmd V0.03  zrpeng\n"
		    "Used:\n"
		    "  md(0x00141280, 512, 1)\n"
		    "  memset(0x00141280, 65, 512)\n"
		    "  malloc(1024)\n"
		    "  free(0x00141280)\n"
		    "---------------------------------------------\n"
		);
		break;
	}
}

/*
 * »ñÈ¡º¯ÊıµÄ²ÎÊı¸öÊı
 * @str º¯ÊıÃüÁî×Ö·û´®
 * @key ²ÎÊı·Ö¸ô·û£¬eg: (,) ×Ö·û´®ÄÚ²¿µÄ,²»Ëã eg:"auther:zrp,2015"
 *
 * return ²ÎÊıµÄ¸öÊı
 *        -2   ¿É±ä²ÎÊı(²ÎÊı¸öÊı²»¶¨)
 *        -1   Ò»¸ö·Ö¸ô·ûÒ²Ã»ÓĞ£¬eg£ºlsÃüÁî¾ÍÃ»ÓĞ()·Ö¸ô·û
 *        >=0  ²ÎÊı¸öÊı£¬eg:timer_pwm_set(5,67)£¬·Ö¸ô·ûÊÇ(,)ºÜÃ÷ÏÔÓĞ2¸ö²ÎÊı
 */
LOCAL int8_t ICACHE_FLASH_ATTR
get_args_num(uint8_t *str, uint8_t *key)
{
	uint8_t *pch;
	uint8_t *pbrk[PARAMS_NUM * 2]; //Ö¸Ïò¶Ïµã(,)
	int8_t brk_cnt;

	brk_cnt = 0;
	pch = (uint8_t *)strpbrk ((char *)str, (char *)key);
	while (pch != NULL)
	{
		pbrk[brk_cnt] = pch;
		brk_cnt++;
		pch = (uint8_t *)strpbrk ((char *)(pch + 1), (char *)key);
	}

	if (brk_cnt == 2)//"(void)" or "(int a)" or "(  )" or "(void *p)"
	{
		uint8_t *t = pbrk[0] + 1;

		if (strstr((char *)(pbrk[0] + 1), "void") != NULL)
		{
			if (strchr((char *)(pbrk[0] + 1), '*') != NULL)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		while (t != pbrk[1])//(  )
		{
			if (*t == ' ')
			{
				t++;
			}
			else
			{
				return 1;
			}
		}

		return 0;
	}
	else if (brk_cnt == 3)
	{
		//ÅĞ¶Ï¿É±ä²ÎÊıµÄÇé¿ö,eg:int printf(const char *fmt, ...)
		if (strstr((char *)pbrk[0], ".." ) != NULL)
		{
			return -2;
		}
	}

	return brk_cnt - 1;
}

/*
 * ÅĞ¶Ï×Ö·û´®Çø¼äÊÇ·ñÎª¿Õ, \r \n \f \v \t ' '
 * @head  Çø¼äÍ·Ö¸Õë
 * @tail  Çø¼äÎ²²¿
 * return Îª¿Õ·µ»ØTRUE, ²»Îª¿Õ·µ»ØFALSE
 * note   [head, tail), ×ó±ÕÓÒ¿ª
 */
LOCAL int8_t ICACHE_FLASH_ATTR
span_isspace(uint8_t *head, uint8_t *tail)
{
	while (head != tail)
	{
		if (isspace(*head))
		{
			head++;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

/*
 * ·ÖÀë²ÎÊı£¬Ö§³Öint32_tºÍ×Ö·û´®, eg:disp(10,-60, "string", 10, "sub:(a,b)")
 * @str º¯Êı×Ö·û´®
 * @key ·Ö¸ô·û×Ö·û´®£¬eg: (,)"
 *
 * note  ×Ö·û´®²ÎÊı²»Ö§³Ö×ªÒå×Ö·û, Ö±½ÓÊäÈë"'\¼´¿ÉÊ¶±ğ, eg:"'\rÖ±½ÓÊ¶±ğ³É4¸ö×Ö·û.
 *       ºÏ·¨×Ö·û´®Ê¾·¶:disp("value:"56V,""), disp("value:\"56\""),
 *       Òª±£Ö¤"³É¶Ô³öÏÖ,·ñÔò±¨´í
 */
LOCAL void ICACHE_FLASH_ATTR
get_args(uint8_t *str, uint8_t *key, _args_t *args)
{
	uint8_t *pch;
	uint8_t *pbrk[PARAMS_NUM * 3]; //Ö¸Ïò·Ö¸ô·û(,)"
	int8_t brk_cnt;

	brk_cnt = 0;
	pch = (uint8_t *)strpbrk ((char *)str, (char *)key);
	while ((pch != NULL) && (brk_cnt < PARAMS_NUM * 3))	//·Ö¸ô·û²»ÄÜÌ«¶à
	{
		pbrk[brk_cnt] = pch;
		brk_cnt++;
		pch = (uint8_t *)strpbrk ((char *)(pch + 1), (char *)key);
	}

	if (brk_cnt == 0)//Ã»ÓĞ(,)¾ÍÊÇÏµÍ³ÃüÁî
	{
		args[0] = -1;
	}
	else if (brk_cnt == 1)//Ö»ÓĞÓĞÒ»¸ö·Ö¸ô·ûËµÃ÷ÊäÈë´íÎó
	{
		args[0] = -2;//º¯Êı´íÎó
	}
	else//2¸ö¼°ÒÔÉÏ
	{
		if (*pbrk[0] == '(' && *pbrk[brk_cnt - 1] == ')')
		{
			args[0] = 0;

			if (brk_cnt == 2)//2¸ö£¬Á½ÖÖ¿ÉÄÜ£ºÒ»¸ö²ÎÊı»òÃ»ÓĞ²ÎÊı
			{
				uint8_t *t = pbrk[0] + 1;

				while (t != pbrk[1])
				{
					if (*t == ' ')
					{
						t++;
						args[0] = 0;
					}
					else
					{
						args[0] = 1;//ÓĞÒ»¸ö²ÎÊı
						args[1] = strtol((char *)(pbrk[0] + 1), NULL, 0);
						break;
					}
				}
			}
			else//3¸ö¼°ÒÔÉÏ, ´¦Àí×î¸´ÔÓµÄÇé¿ö
			{
				uint8_t quotation_cnt;
				uint8_t i;
				uint8_t args_cnt;
				uint8_t state;
				uint8_t finding;

				//Ë«ÒıºÅÊıÁ¿ÊÇ·ñ³É¶Ô
				quotation_cnt = 0;
				for (i = 1; i < brk_cnt; i++)
				{
					if (*pbrk[i] == '\"')
					{
						quotation_cnt++;
					}
				}

				if (quotation_cnt == 0)//Ã»ÓĞ×Ö·û´®²ÎÊı
				{
					for (i = 0; i < brk_cnt; i++)
					{
						*pbrk[i] = '\0';    //Ìæ»»³É×Ö·û´®½áÊø·û
					}

					for (i = 0; i < brk_cnt; i++)//×ª»»³ÉÕûĞÍÊı
					{
						args[i + 1] = strtol((char *)(pbrk[i] + 1), NULL, 0);
					}

					args[0] = brk_cnt - 1;

					return ;
				}
				else if ((quotation_cnt % 2) != 0) //Ã»ÓĞ³É¶Ô³öÏÖ"
				{
					args[0] = -2;//º¯Êı´íÎó
					return ;
				}

				//²ÎÊı×ª»», ´ËÊ±·Ö¸ô·ûÊıÁ¿>=3
				args_cnt = 1;
				state = 0;
				i = 0;
				while (i < brk_cnt - 1)	//ÉÙÒ»¸ö, ½áÎ²µÄ')'²»¿¼ÂÇ
				{
					finding = 1;
					while (finding)
					{
						switch (state)
						{
						case 0://Ê¶±ğ",
							{
								if (i == 0)
								{
									if (*pbrk[i + 1] == '\"')
									{
										i++;
										state = 2;
									}
									else if (*pbrk[i + 1] == ',')
									{
										state = 1;
									}
								}
								else if (*pbrk[i] == ',')
								{
									if (*pbrk[i + 1] == ',' || *pbrk[i + 1] == ')')
									{
										state = 1;
									}
									else if (*pbrk[i + 1] == '\"')
									{
										i++;
										state = 2;
									}
								}
							}
							break;

						case 1://ÕûĞÍ²ÎÊı
							{
								args[args_cnt] = strtol((char *)(pbrk[i] + 1), NULL, 0);
								args_cnt++;

								i++;			//ÏÂÒ»¸ö,
								finding = 0;	//Ìø³öÑ­»·£¬ÅĞ¶ÏÒ»ÏÂ´ËÊ±iµÄ·¶Î§
								state = 0;
							}
							break;

						case 2://×Ö·û´®²ÎÊı
							{
								uint8_t is_string_end;

								args[args_cnt] = (_args_t)(pbrk[i] + 1);
								args_cnt++;

								//ÕÒ³ö",¼´¿ÉÅĞ¶Ï×Ö·û´®½áÊøÁË
								is_string_end = 0;
								while (!is_string_end)	//ÕÒ³ö×Ö·û´®½áÎ²
								{
									i++;
									while (*pbrk[i] != '\"')//ÕÒ³öÅä¶ÔµÄ"
									{
										i++;
									}

									i++;		//ÏÂÒ»¸ö,
									if (i >= brk_cnt - 1)
									{
										is_string_end = 1;
										*pbrk[i - 1] = 0;	//×Ö·û´®Î²²¿"Ìæ»»³É½áÊø·û
										break;				//$! µ½ÃüÁîĞĞ½áÎ²ÁË,ÍË³öÕû¸öº¯Êı
									}

									if (*pbrk[i] == ',')	//µ½×Ö·û´®Î²²¿ÁË?
									{
										//½øÒ»²½ÅĞ¶Ï"ºÍ,Ö®¼äÊÇ²»ÊÇ¿Õ×Ö·û
										if (span_isspace(pbrk[i - 1] + 1, pbrk[i]))
										{
											is_string_end = 1;	//$! È·ÊµÊÇ×Ö·û´®Î²²¿
											*pbrk[i - 1] = 0;	// ×Ö·û´®Î²²¿"Ìæ»»³É½áÊø·û
											break;
										}
										else
										{
											//"  """" "ºÍ,Ö®¼äÓĞ×Ö·û
											i++;
											while (*pbrk[i] != '\"')//ÕÒ³öµÚÒ»¸ö"
											{
												i++;
											}
										}
									}
									else if (*pbrk[i] == '\"')	//ÓÖÊÇµÚÒ»¸ö"
									{}
								}

								finding = 0;	//Ìø³öÑ­»·£¬ÅĞ¶ÏÒ»ÏÂ´ËÊ±iµÄ·¶Î§
								state = 0;
							}
							break;
						}
					}
				}

				args[0] = args_cnt - 1;
			}

			*pbrk[0] = 0;	//'('Çåµô£¬±£Ö¤º¯ÊıÃû½áÎ²Îª0
		}
		else
		{
			args[0] = -2;//¸ñÊ½´íÎó
		}
	}
}

/*
 * ´ÓCmdTblÖĞ»ñÈ¡º¯ÊıÃû
 * @str CmdTbl[i].fname
 * @len ·µ»Øº¯ÊıÃû³¤¶È
 *
 * return ·µ»ØÓÃ»§º¯ÊıÃûÖ¸Õë
 */
LOCAL uint8_t ICACHE_FLASH_ATTR
*get_fname(uint8_t *str, uint8_t *len)
{
	uint8_t *phead;
	uint8_t *pend;

	phead = (uint8_t *)strchr((char *)str, '(');

	//Ìø¹ıÎ²²¿¿Õ¸ñ,\t
	phead--;
	while (*phead == ' ' || *phead == '\t')
	{
		phead--;
	}
	pend = phead;

	//ÕÒµ½Í·²¿µÄ ' ' »òÕß '*'
	while (*phead != ' ' && *phead != '*' && *phead != '\t')
	{
		phead--;
	}
	phead++;

	*len = pend - phead + 1;
	return phead;
}
/*
 * º¯ÊıÃüÁîĞĞÖ´ĞĞ
 * @cmd º¯ÊıÃüÁîĞĞ, eg: timer_pwm_set(0, 567)
 */
void ICACHE_FLASH_ATTR
fcmd_exec(uint8_t *cmd)
{
	uint8_t *pcmd = cmd;
	_args_t args[PARAMS_NUM + 1]; 	//²ÎÊıÊı×é
	_args_t ret = 0;				//º¯Êı·µ»ØÖµ
	uint8_t i;
	int8_t cmdtbl_param_num;

	//Ìø¹ı¿ªÍ·¿Õ¸ñ
	while (*pcmd == ' ')
	{
		pcmd++;
	}

	//·ÖÀë²ÎÊı
	memset(args, 0, PARAMS_NUM + 1);
	get_args(pcmd, (uint8_t *)"(,)\"", args);

	if (args[0] == -2)
	{
		PRINTF("err:fmt err\n");
		return ;
	}

	//Æ¥ÅäÃüÁî
	if (args[0] == -1)
	{
		//ÏµÍ³ÃüÁî
		for (i = 0; i < SYSTEM_CMD; i++)
		{
			if (strncmp((char *)pcmd, CmdTbl[i].fname, strlen((char *)pcmd)) == 0)
			{
				break;
			}
		}

		if (i >= SYSTEM_CMD)
		{
			PRINTF("err:system cmd err\n");
			return ;
		}
	}
	else
	{
		//ÆÕÍ¨ÃüÁî
		for (i = SYSTEM_CMD; i < CmdTblSize; i++)
		{
			uint8_t *pcmd_end;
			uint8_t pfname_len;
			uint8_t pcmd_len;
			uint8_t *pfname = get_fname((uint8_t *)CmdTbl[i].fname, &pfname_len);

			//malloc (int size), pcmd_endÖ¸Ïò'c'
			pcmd_end = pcmd + strlen((char *)pcmd) - 1;
			while (*pcmd_end == ' ')//Ìø¹ı¿Õ¸ñ
			{
				--pcmd_end;
			}
			pcmd_len = pcmd_end - pcmd + 1;

			//±È½Ïº¯ÊıÃû
			if (strncmp((char *)pfname, (char *)pcmd, pcmd_len > pfname_len ? pcmd_len : pfname_len) == 0)
			{
				break;
			}
		}

		//Ã»ÓĞÆ¥Åäµ½ÃüÁî
		if (i >= CmdTblSize)
		{
			PRINTF("err:not you wanted cmd\n");
			return;
		}
	}

	//µÃµ½º¯Êı±íÀïµÄº¯ÊıµÄ²ÎÊı¸öÊı
	if (args[0] != -1)
	{
		//ÆÕÍ¨ÃüÁî
		cmdtbl_param_num = get_args_num((uint8_t *)CmdTbl[i].fname, (uint8_t *)"(,)");
		if (cmdtbl_param_num == -2)
		{
			;//¿É±ä²ÎÊıÇé¿ö
		}
		else if (cmdtbl_param_num != args[0])
		{
			PRINTF("err:param err\n");
			return ;
		}
	}
	else
	{
		//Ö´ĞĞÏµÍ³ÃüÁî
		system_cmd_exe(i);
		return;
	}

	//´«ÈëÏàÓ¦²ÎÊı£¬Ö´ĞĞÆÕÍ¨ÃüÁî
	switch (args[0])
	{
	case 0://Ã»ÓĞ²ÎÊı
		ret = (*(_args_t(*)(void))CmdTbl[i].pfunc)();
		break;

	case 1:
		ret = (*(_args_t(*)(_args_t))CmdTbl[i].pfunc)(args[1]);
		break;

	case 2:
		ret = (*(_args_t(*)(_args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2]);
		break;

	case 3:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3]);
		break;

	case 4:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4]);
		break;

	case 5:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5]);
		break;

	case 6:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5], args[6]);
		break;

	case 7:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5], args[6],
		        args[7]);
		break;

	case 8:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5],
		        args[6], args[7], args[8]);
		break;

	case 9:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4],
		        args[5], args[6], args[7], args[8], args[9]);
		break;

	case 10:
		ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3],
		        args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
		break;

	default:
		PRINTF("err:params num err\n");
		break;
	}

	if (sizeof(_args_t) == 1)
	{
		PRINTF("=0x%02x,%d;\n", ret, ret);
	}
	else if (sizeof(_args_t) == 2)
	{
		PRINTF("=0x%04x,%d;\n", ret, ret);
	}
	else if (sizeof(_args_t) == 4)
	{
		PRINTF("=0x%08x,%d;\n", ret, ret);
	}
}



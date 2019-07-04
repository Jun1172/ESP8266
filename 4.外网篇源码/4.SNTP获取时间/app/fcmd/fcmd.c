/*******************************************************************************
--����������V0.03
--fcmd.c
--zrpeng
--2015-3-5
--
--note:
	��ģ����Ҫ���ڵ�����;�����Է���Ľ����ֶ����ԣ������C���Ժ���һ�������н׶�ִ��
	��Ҫִ�е�C������
	����:���Ե�Ƭ����PWMʱ��ִ��int timer_pwm_set(int pwm)
          ����timer_pwm_set(88)���Ϳ��Ըı�ռ�ձ���
	��ģ�����һ����Σ�գ ��Ҫ��C�ͼĴ�������������һ�����˽�.

--history:
	0.01����������ֻ��֧��int32_t���Ͳ���
	0.02�������ַ������͵�֧��,eg:disp(50,-60, "ADC:  mV", 10, "89"),
		������CmdTbl�ڲ���Ա��˳���ַ�����ǰ������ָ���ں�
		�������ڴ���ʾ����,c�⺯���������������ò���
		Ŀǰ��������
	0.03���ӶԿɱ������֧��,eg:int printf(const char *fmt, ...)
*******************************************************************************/
#include "fcmd.h"


#define PARAMS_NUM  10 //����֧��10������
#define SYSTEM_CMD  3  //ϵͳ����ĸ���, CmdTbl����ϵͳ�����ͬʱҲҪ�����������

#define _args_t  int	//�������ͣ�����16bit��8bit��Ƭ��Ҫע��

/*******************************************************************************
 * һ�����������нṹ
 */
typedef struct
{
	char *fname;
	void (*pfunc)(void);
} CmdTbl_t;

#include "fcmd_cfg.h"


/*******************************************************************************
 * ��̬��������
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
 * ִ��ģ�鶨���ָ��
 * @n ָ����ţ�ָ��CmdTbl�е����
 */
LOCAL void ICACHE_FLASH_ATTR
system_cmd_exe(uint8_t n)
{
	uint8_t i;

	switch (n)
	{
	case 0://ls:�г����е�����
		PRINTF("------------- function list --------------\n");
		for (i = 0; i < CmdTblSize; i++)
		{
			PRINTF("%s\n", CmdTbl[i].fname);
		}
		PRINTF("-------------------------------------------\n");
		break;

	case 1://addr:�г����к���ָ��ĵ�ַ
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
 * ��ȡ�����Ĳ�������
 * @str ���������ַ���
 * @key �����ָ�����eg: (,) �ַ����ڲ���,���� eg:"auther:zrp,2015"
 *
 * return �����ĸ���
 *        -2   �ɱ����(������������)
 *        -1   һ���ָ���Ҳû�У�eg��ls�����û��()�ָ���
 *        >=0  ����������eg:timer_pwm_set(5,67)���ָ�����(,)��������2������
 */
LOCAL int8_t ICACHE_FLASH_ATTR
get_args_num(uint8_t *str, uint8_t *key)
{
	uint8_t *pch;
	uint8_t *pbrk[PARAMS_NUM * 2]; //ָ��ϵ�(,)
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
		//�жϿɱ���������,eg:int printf(const char *fmt, ...)
		if (strstr((char *)pbrk[0], ".." ) != NULL)
		{
			return -2;
		}
	}

	return brk_cnt - 1;
}

/*
 * �ж��ַ��������Ƿ�Ϊ��, \r \n \f \v \t ' '
 * @head  ����ͷָ��
 * @tail  ����β��
 * return Ϊ�շ���TRUE, ��Ϊ�շ���FALSE
 * note   [head, tail), ����ҿ�
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
 * ���������֧��int32_t���ַ���, eg:disp(10,-60, "string", 10, "sub:(a,b)")
 * @str �����ַ���
 * @key �ָ����ַ�����eg: (,)"
 *
 * note  �ַ���������֧��ת���ַ�, ֱ������"'\����ʶ��, eg:"'\rֱ��ʶ���4���ַ�.
 *       �Ϸ��ַ���ʾ��:disp("value:"56V,""), disp("value:\"56\""),
 *       Ҫ��֤"�ɶԳ���,���򱨴�
 */
LOCAL void ICACHE_FLASH_ATTR
get_args(uint8_t *str, uint8_t *key, _args_t *args)
{
	uint8_t *pch;
	uint8_t *pbrk[PARAMS_NUM * 3]; //ָ��ָ���(,)"
	int8_t brk_cnt;

	brk_cnt = 0;
	pch = (uint8_t *)strpbrk ((char *)str, (char *)key);
	while ((pch != NULL) && (brk_cnt < PARAMS_NUM * 3))	//�ָ�������̫��
	{
		pbrk[brk_cnt] = pch;
		brk_cnt++;
		pch = (uint8_t *)strpbrk ((char *)(pch + 1), (char *)key);
	}

	if (brk_cnt == 0)//û��(,)����ϵͳ����
	{
		args[0] = -1;
	}
	else if (brk_cnt == 1)//ֻ����һ���ָ���˵���������
	{
		args[0] = -2;//��������
	}
	else//2��������
	{
		if (*pbrk[0] == '(' && *pbrk[brk_cnt - 1] == ')')
		{
			args[0] = 0;

			if (brk_cnt == 2)//2�������ֿ��ܣ�һ��������û�в���
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
						args[0] = 1;//��һ������
						args[1] = strtol((char *)(pbrk[0] + 1), NULL, 0);
						break;
					}
				}
			}
			else//3��������, ������ӵ����
			{
				uint8_t quotation_cnt;
				uint8_t i;
				uint8_t args_cnt;
				uint8_t state;
				uint8_t finding;

				//˫���������Ƿ�ɶ�
				quotation_cnt = 0;
				for (i = 1; i < brk_cnt; i++)
				{
					if (*pbrk[i] == '\"')
					{
						quotation_cnt++;
					}
				}

				if (quotation_cnt == 0)//û���ַ�������
				{
					for (i = 0; i < brk_cnt; i++)
					{
						*pbrk[i] = '\0';    //�滻���ַ���������
					}

					for (i = 0; i < brk_cnt; i++)//ת����������
					{
						args[i + 1] = strtol((char *)(pbrk[i] + 1), NULL, 0);
					}

					args[0] = brk_cnt - 1;

					return ;
				}
				else if ((quotation_cnt % 2) != 0) //û�гɶԳ���"
				{
					args[0] = -2;//��������
					return ;
				}

				//����ת��, ��ʱ�ָ�������>=3
				args_cnt = 1;
				state = 0;
				i = 0;
				while (i < brk_cnt - 1)	//��һ��, ��β��')'������
				{
					finding = 1;
					while (finding)
					{
						switch (state)
						{
						case 0://ʶ��",
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

						case 1://���Ͳ���
							{
								args[args_cnt] = strtol((char *)(pbrk[i] + 1), NULL, 0);
								args_cnt++;

								i++;			//��һ��,
								finding = 0;	//����ѭ�����ж�һ�´�ʱi�ķ�Χ
								state = 0;
							}
							break;

						case 2://�ַ�������
							{
								uint8_t is_string_end;

								args[args_cnt] = (_args_t)(pbrk[i] + 1);
								args_cnt++;

								//�ҳ�",�����ж��ַ���������
								is_string_end = 0;
								while (!is_string_end)	//�ҳ��ַ�����β
								{
									i++;
									while (*pbrk[i] != '\"')//�ҳ���Ե�"
									{
										i++;
									}

									i++;		//��һ��,
									if (i >= brk_cnt - 1)
									{
										is_string_end = 1;
										*pbrk[i - 1] = 0;	//�ַ���β��"�滻�ɽ�����
										break;				//$! �������н�β��,�˳���������
									}

									if (*pbrk[i] == ',')	//���ַ���β����?
									{
										//��һ���ж�"��,֮���ǲ��ǿ��ַ�
										if (span_isspace(pbrk[i - 1] + 1, pbrk[i]))
										{
											is_string_end = 1;	//$! ȷʵ���ַ���β��
											*pbrk[i - 1] = 0;	// �ַ���β��"�滻�ɽ�����
											break;
										}
										else
										{
											//"  """" "��,֮�����ַ�
											i++;
											while (*pbrk[i] != '\"')//�ҳ���һ��"
											{
												i++;
											}
										}
									}
									else if (*pbrk[i] == '\"')	//���ǵ�һ��"
									{}
								}

								finding = 0;	//����ѭ�����ж�һ�´�ʱi�ķ�Χ
								state = 0;
							}
							break;
						}
					}
				}

				args[0] = args_cnt - 1;
			}

			*pbrk[0] = 0;	//'('�������֤��������βΪ0
		}
		else
		{
			args[0] = -2;//��ʽ����
		}
	}
}

/*
 * ��CmdTbl�л�ȡ������
 * @str CmdTbl[i].fname
 * @len ���غ���������
 *
 * return �����û�������ָ��
 */
LOCAL uint8_t ICACHE_FLASH_ATTR
*get_fname(uint8_t *str, uint8_t *len)
{
	uint8_t *phead;
	uint8_t *pend;

	phead = (uint8_t *)strchr((char *)str, '(');

	//����β���ո�,\t
	phead--;
	while (*phead == ' ' || *phead == '\t')
	{
		phead--;
	}
	pend = phead;

	//�ҵ�ͷ���� ' ' ���� '*'
	while (*phead != ' ' && *phead != '*' && *phead != '\t')
	{
		phead--;
	}
	phead++;

	*len = pend - phead + 1;
	return phead;
}
/*
 * ����������ִ��
 * @cmd ����������, eg: timer_pwm_set(0, 567)
 */
void ICACHE_FLASH_ATTR
fcmd_exec(uint8_t *cmd)
{
	uint8_t *pcmd = cmd;
	_args_t args[PARAMS_NUM + 1]; 	//��������
	_args_t ret = 0;				//��������ֵ
	uint8_t i;
	int8_t cmdtbl_param_num;

	//������ͷ�ո�
	while (*pcmd == ' ')
	{
		pcmd++;
	}

	//�������
	memset(args, 0, PARAMS_NUM + 1);
	get_args(pcmd, (uint8_t *)"(,)\"", args);

	if (args[0] == -2)
	{
		PRINTF("err:fmt err\n");
		return ;
	}

	//ƥ������
	if (args[0] == -1)
	{
		//ϵͳ����
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
		//��ͨ����
		for (i = SYSTEM_CMD; i < CmdTblSize; i++)
		{
			uint8_t *pcmd_end;
			uint8_t pfname_len;
			uint8_t pcmd_len;
			uint8_t *pfname = get_fname((uint8_t *)CmdTbl[i].fname, &pfname_len);

			//malloc (int size), pcmd_endָ��'c'
			pcmd_end = pcmd + strlen((char *)pcmd) - 1;
			while (*pcmd_end == ' ')//�����ո�
			{
				--pcmd_end;
			}
			pcmd_len = pcmd_end - pcmd + 1;

			//�ȽϺ�����
			if (strncmp((char *)pfname, (char *)pcmd, pcmd_len > pfname_len ? pcmd_len : pfname_len) == 0)
			{
				break;
			}
		}

		//û��ƥ�䵽����
		if (i >= CmdTblSize)
		{
			PRINTF("err:not you wanted cmd\n");
			return;
		}
	}

	//�õ���������ĺ����Ĳ�������
	if (args[0] != -1)
	{
		//��ͨ����
		cmdtbl_param_num = get_args_num((uint8_t *)CmdTbl[i].fname, (uint8_t *)"(,)");
		if (cmdtbl_param_num == -2)
		{
			;//�ɱ�������
		}
		else if (cmdtbl_param_num != args[0])
		{
			PRINTF("err:param err\n");
			return ;
		}
	}
	else
	{
		//ִ��ϵͳ����
		system_cmd_exe(i);
		return;
	}

	//������Ӧ������ִ����ͨ����
	switch (args[0])
	{
	case 0://û�в���
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



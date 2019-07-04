#ifndef _CMD_MEM_H_
#define _CMD_MEM_H_

void md(int addr, int elem_cnt, int elem_size);
int cmp(void *addr1, void *addr2, int elem_cnt, int elem_size);

int sfmd(u32 addr, int elem_cnt, int elem_size);
int sfmw(u32 writeaddr, u8 *pbuf, u32 num);

#endif


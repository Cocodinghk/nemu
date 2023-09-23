#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>
#include "cpu/reg.h"
#include "memory/tlb.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	//return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	//下面是加上cache之后的代码
	/*
	从内存中读一个数据，先去cache1中找
	cache1中找到就读,找不到去cache2中找
	cache2中到得到就读，并且要写到cache1里。找不到去内存里找
	内存里找到了读，并且还要写到cache1和cache2里
	*/
	int32_t set1=(addr>>b_num)&(c1_set_num-1);
	int32_t i=c1_read(addr);//这一句话包括了上面的所有内容
	//现在要读的这一块已经放到c1_cache[set1][i]里了
	int32_t start1=(addr&(block_size-1));
	int8_t tmp[block_size*2];//tmp数组用来存我要读取的数据
	memset(tmp,0,sizeof(tmp));
	if(start1+len<=block_size)//没跨越块
	{
		memcpy(tmp,c1_cache[set1][i].block+start1,len);
	}
	else//跨越两块（最多也就跨两块）
	{
		memcpy(tmp,c1_cache[set1][i].block+start1,block_size-start1);//先存第一个块里的
		//去cache1里找后半部分，也包括最上面注释里的步骤
		int32_t ii=c1_read(addr+block_size-start1);
		int32_t sett=((addr+block_size-start1)>>b_num)&(c1_set_num-1);
		memcpy(tmp+block_size-start1,c1_cache[sett][ii].block,len-(block_size-start1));//后半拉肯定从第二个块的开头开始
	}
	int a=0;
	uint32_t ans=unalign_rw(tmp+a,4)&(~0u>>((4-len)<<3));
	return ans;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	//dram_write(addr, len, data);
	//加上cache后的代码
	/*
	写的时候，先去cache1里找
	找到的话改cache1和cache2，找不到只改cache2
	改cache2的时候，先去cache2里找
	找到的话只改cache2，置脏
	找不到的话，先把内存里的块搬到cache2，然后改cache2，置脏
	*/
	c1_write(addr,len,data);//这一句话就能概括说明所有的操作了
}


int readTLB(lnaddr_t addr);
void writeTLB(lnaddr_t addr,hwaddr_t addr_);
hwaddr_t cmd_page_translate(lnaddr_t addr) {	// 简易调试器
	if(!cpu.cr0.protect_enable || !cpu.cr0.paging) return addr;
	/* addr = 10 dictionary + 10 page + 12 offset */
	uint32_t dictionary = addr >> 22, page = (addr >> 12) & 0x3ff, offset = addr & 0xfff;
	/* 读取页表信息 */
	uint32_t tmp = (cpu.cr3.page_directory_base << 12) + dictionary * 4;		// 页目录基地址 + 页目录号 * 页表项大小
	Page_info dictionary_, page_;
	dictionary_.val = hwaddr_read(tmp, 4);
	tmp = (dictionary_.addr << 12) + page * 4;									// 二级页表基地址 + 页号 + 页表项大小
	page_.val = hwaddr_read(tmp, 4);
	if(dictionary_.p != 1) {
		printf("dirctionary present != 1\n");
		return 0;
	}
	if(page_.p != 1) {
		printf("second page table present != 1\n");
		return 0;
	}
	return (page_.addr << 12) + offset;
}

hwaddr_t page_translate(lnaddr_t addr)
{
	//未进入保护模式或者未开启分页机制
	if(!cpu.cr0.protect_enable||!cpu.cr0.paging)
		return addr;
	uint32_t direct=addr>>22;//页目录
	uint32_t page=(addr>>12)&0x3ff;//二级页表
	uint32_t offset=addr&0xfff;//物理页中的偏移
	//看TLB
	int index=readTLB(addr);
	if(index!=-1)
		return (tlb[index].data<<12)+offset;
	//在TLB没找到，读取页表信息
	uint32_t tmp=(cpu.cr3.page_directory_base<<12)+direct*4;
	//页目录基地址加上页目录编号*4，即为要找的页目录的首地址
	Page_info directt,pagee;
	directt.val=hwaddr_read(tmp,4);
	tmp=(directt.addr<<12)+page*4;//找到二级页表的首地址
	pagee.val=hwaddr_read(tmp,4);//读出物理页的首地址
	Assert(directt.p==1,"directory present != 1");
	Assert(pagee.p==1,"the second page table present != 1");
	hwaddr_t addr_=(pagee.addr<<12)+offset;
	writeTLB(addr,addr_);
	return addr_;
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	assert(len==1||len==2||len==4);
	uint32_t offset=addr&0xfff;
	//处理两个虚拟页被分配到不同的物理页中的情况
	if((int64_t)(offset+len)>0x1000)
	{
		size_t l=0xfff-offset+1;
		uint32_t low_val=lnaddr_read(addr,l);//处理低位
		uint32_t high_val=lnaddr_read(addr+l,len-l);//处理高位
		//把低位和高位组合起来
		return ((high_val<<(l*8))|low_val);
	}
	else{
		hwaddr_t hwaddr=page_translate(addr);
		return hwaddr_read(hwaddr,len);
	}
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	assert(len==1||len==2||len==4);
	uint32_t offset=addr&0xfff;
	if((int64_t)(offset+len)>0x1000)
	{
		size_t l=0xfff-offset+1;
		lnaddr_write(addr,l,data&((1<<(l*8))-1));//写低位
		lnaddr_write(addr+l,len-l,data>>(l*8));//写高位
	}
	else{
		hwaddr_t hwaddr=page_translate(addr);
		hwaddr_write(hwaddr,len,data);
	}
}

lnaddr_t seg_translate(swaddr_t addr,size_t len,uint8_t sreg)
{
	if(cpu.cr0.protect_enable==0) return addr;//在实模式下
	else //进入IA-32模式
	{
		lnaddr_t ans=cpu.sreg[sreg].base+addr;
		return ans;
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
	assert(len==1||len==2||len==4);
	lnaddr_t lnaddr=seg_translate(addr,len,sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
	assert(len == 1 || len == 2 || len == 4);
	lnaddr_t lnaddr=seg_translate(addr,len,sreg);
	lnaddr_write(lnaddr, len, data);
}


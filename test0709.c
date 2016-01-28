#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysLib.h>
#include <intLib.h>
#include <taskLib.h>
#include <string.h>
#include <logLib.h>
#include <drv/pci/pciConfigLib.h>

#define BRIDGE_REG_WRITE32(base,offset,value) \
        PCI_OUT_LONG ((base + offset), value)

#define BRIDGE_REG_READ32(base,offset)    PCI_IN_LONG(base + offset)

#define FPGA_REG_READ32(base,offset) \
        *((volatile unsigned long *)  ((unsigned char *)(base) + offset))

UINT32 abcdSwap(UINT32 x){
	UINT32 a,b,c,d;
	
	a = (x & 0x000000ff) << 24;
	b = (x & 0x0000ff00) << 8;
	c = (x & 0x00ff0000) >> 8;
	d = (x & 0xff000000) >> 24;
	
	return (a+b+c+d);
}

void hello(){
	printf("Hello!\n");
}

void intTest(unsigned int i){
	logMsg("Hello! 0x%08x\n",FPGA_REG_READ32(i,0),0,0,0,0,0);
}

int test0709(){
	int bus;
	int device;
	int function;
	unsigned int bridgeAddr;
	unsigned int fpgaAddr;
	unsigned char intShort;
	int intLine;
	int i;
	
	for(i=0;i<6;i++){
		if(pciFindDevice(0x0709, 0xd212, i, &bus, &device, &function) == ERROR)
		{
			printf ("cfail to find 0709 index %d\n", i);
			return ERROR;
		}
		printf("index is %d\n",i);

		pciConfigInLong (bus, device, function, PCI_CFG_BASE_ADDRESS_0, &bridgeAddr);
		bridgeAddr &= PCI_MEMBASE_MASK;

		pciConfigInLong (bus, device, function, PCI_CFG_BASE_ADDRESS_1, &fpgaAddr);
		fpgaAddr &= PCI_MEMBASE_MASK;
		
		printf("Bridge PCI Address: 0x%08x\n",  bridgeAddr);
		
		printf("FPGA Address: 0x%08x\n",  fpgaAddr);
	
		printf("pciReg 0x0 is: 0x%08x\n", (* (UINT32*)bridgeAddr));
		printf("FPGA 0x0 is: 0x%08x\n", BRIDGE_REG_READ32(fpgaAddr,0));

		pciConfigInByte (bus, device, function, PCI_CFG_DEV_INT_LINE, &intShort);
	
		intLine = intShort;
		
		printf("Interrupt Line: %d\n", intLine);
	
		if(intConnect(INUM_TO_IVEC(intLine), intTest, fpgaAddr) == ERROR)
		{
			printf("intConnect error\n");
			return ERROR;
		}
	
		if(intEnable(intLine) == ERROR)
		{
			printf("intEnable error\n");
			return ERROR;
		}
	
		BRIDGE_REG_WRITE32(bridgeAddr, 0x68, 0x0f0C0900);	
	}
	
	return OK;
}

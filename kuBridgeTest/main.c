#include <pspkernel.h>
#include <pspumd.h>
#include <psppower.h>
#include <pspthreadman_kernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include "kubridge.h"
#include "systemctrl.h"

PSP_MODULE_INFO("kubridge_test", 0, 1, 0);

#define printk pspDebugScreenPrintf

void find_module_test(void)
{
	int ret;
	SceModule2 mod;

	ret = kuKernelFindModuleByName("SystemControl", (SceModule*)&mod);

	if(ret >= 0) {
		printk("%s: %s 0x%04X\n", __func__, mod.modname, mod.attribute);
		printk("%s: entry 0x%08X text_addr 0x%08X\n", __func__, mod.entry_addr, mod.text_addr);
	}
}

void memcpy_test(void)
{
	printk("%s\n", __func__);
	kuKernelMemcpy((void*)0x08900000, (void*)0x88000000, 4 * 1024 * 1024);
}

void peekw_pokew_test(void)
{
	u32 value;
	
	value = kuKernelPeekw((void*)0x88000000); // first kernel memory
	printk("%s: 0x%08X\n", __func__, value);
	kuKernelPokew((void*)0x10000, value); // to scratchpad
}

void cahce_invalidate_test(void)
{
	printk("%s\n", __func__);
	kuKernelIcacheInvalidateAll();
}

void kernel_call_test(void)
{
	struct KernelCallArg args;
	void *func_addr;
	char buf[16];
	char *fmt = "%s"; 
	char *string = "Hello world"; 
	int ret;

	// call sprintf in SysclibForKernel from user space
	func_addr = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysclibForKernel", 0x7661E728);
	args.arg1 = (u32)buf;
	args.arg2 = (u32)fmt;
	args.arg3 = (u32)string;

	ret = kuKernelCall(func_addr, &args);

	if(ret >= 0) {
		printk("%s: sprintf returns 0x%08X\n", __func__, args.ret1);
		printk("%s: %s\n", __func__, buf);
	}
}

void kernel_call_test_with_stack(void)
{
	struct KernelCallArg args;
	void *func_addr;
	char buf[16];
	char *fmt = "%s"; 
	char *string = "Hello world"; 
	int ret;

	// call sprintf in SysclibForKernel from user space
	func_addr = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysclibForKernel", 0x7661E728);
	args.arg1 = (u32)buf;
	args.arg2 = (u32)fmt;
	args.arg3 = (u32)string;
	ret = kuKernelCallExtendStack(func_addr, &args, 0x4000);

	if(ret >= 0) {
		printk("%s: sprintf returns 0x%08X\n", __func__, args.ret1);
		printk("%s: %s\n", __func__, buf);
	}
}

int main_thread(SceSize args, void *argp)
{
	pspDebugScreenInit();

	printk("PRO kuBridge test\n\n");
	find_module_test();
	memcpy_test();
	peekw_pokew_test();
	cahce_invalidate_test();
	kernel_call_test();
	kernel_call_test_with_stack();

	return 0;
}

int module_start(SceSize args, void* argp)
{
	int thid;

	thid = sceKernelCreateThread("main_thread", main_thread, 0x1A, 0x1000, 0, NULL);

	if(thid>=0) {
		sceKernelStartThread(thid, args, argp);
	}

	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
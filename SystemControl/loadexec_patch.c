#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "utils.h"
#include "systemctrl.h"
#include "printk.h"
#include "../Rebootex/Rebootex_bin/rebootex.h"

static int (*LoadReboot)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4) = NULL;

//load reboot wrapper
static int load_reboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
	if (rebootex != NULL) {
		//copy reboot extender
		memcpy((void*)0x88FC0000, rebootex, size_rebootex);

		//reset reboot flags
		memset((void*)0x88FB0000, 0, 0x100);

		//store psp model
		_sw(psp_model, 0x88FB0000);
	}

	//forward
	return (*LoadReboot)(arg1, arg2, arg3, arg4);
}

void patch_sceLoadExec(void)
{
	SceModule2 * loadexec = (SceModule2*)sceKernelFindModuleByName("sceLoadExec");
	unsigned int offsets[6];
	u32 text_addr;

	if (loadexec == NULL) {
		return;
	}

	text_addr = loadexec->text_addr;

	if(psp_model == 4) { // PSP-N1000
		offsets[0] = 0x2F90;
		offsets[1] = 0x2FDC;
	} else {
		offsets[0] = 0x2D44;    // call to LoadReboot
		offsets[1] = 0x2D90;    // lui $at, 0x8860
	}

	//replace LoadReboot function
	_sw(MAKE_CALL(load_reboot), loadexec->text_addr + offsets[0]);

	//patch Rebootex position to 0x88FC0000
	_sw(0x3C0188FC, loadexec->text_addr + offsets[1]); // lui $at, 0x88FC

	//save LoadReboot function
	LoadReboot = (void*)loadexec->text_addr;

	sync_cache();
}
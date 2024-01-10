#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <wafel/dynamic.h>
#include <wafel/ios_dynamic.h>
#include <wafel/utils.h>
#include <wafel/patch.h>
#include <wafel/ios/svc.h>

#include <loadfile.h>

// This fn runs before everything else in kernel mode.
// It should be used to do extremely early patches
// (ie to BSP and kernel, which launches before MCP)
// It jumps to the real IOS kernel entry on exit.
__attribute__((target("arm")))
void kern_main()
{
    // Make sure relocs worked fine and mappings are good
    debug_printf("we in here payloader plugin kern %p\n", kern_main);

    debug_printf("init_linking symbol at: %08x\n", wafel_find_symbol("init_linking"));

    // // Loadfile patch
    ASM_T_PATCH_K(0x050254D4, 
        "ldr r0, _lf_hook\n"
        "blx r0\n"
        "_lf_hook: .word MCP_LoadFile_patch\n"
    );


    // ReadCOS patch
    ASM_T_PATCH_K(0x0501DD74, 
        "ldr r2, _cos_hook\n"
        "blx r2\n"
        "_cos_hook: .word MCP_ReadCOSXml_patch\n"
    );
    // ASM_T_PATCH_K(0x051105CA, 
    //     "ldr r2, _cos_hook2\n"
    //     "blx r2\n"
    //     "_cos_hook2: .word MCP_ReadCOSXml_patch2\n"
    // );
}

// This fn runs before MCP's main thread, and can be used
// to perform late patches and spawn threads under MCP.
// It must return.
void mcp_main()
{

}

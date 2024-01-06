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
    debug_printf("we in here wafel loader plugin kern %p\n", kern_main);

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

    uint8_t *p = (uint8_t*)ios_elf_vaddr_to_paddr(0x1070EAB0);
    debug_printf("0x1070EAB0: %02X %02X %02X %02X %02X %02X %02X %02X\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

    // print some path hopefully
    //ASM_PATCH_K(0x1070EAB0, "mov r1, r5\nmov r2,r6");


    // ASM_PATCH_K(0x1070E938, "B 0x1070EB2C\n");

    // ASM_PATCH_K(0x1070EB2C, 
    //     "mov r1, r4\n"
    //     "nop\n"
    //     "BL 0x107F0C84\n"
    // )

    //char fspatch[] = "%.5s ";
    //MEMCPY_PATCH(0x10801150, fspatch, strlen(fspatch));

    debug_printf("0x1070EAB0: %02X %02X %02X %02X %02X %02X %02X %02X\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

    // MCP_ioctl100_patch
    // ASM_PATCH_K(0x05025242, 
    //     ".thumb\n"
    //     "ldr r2, _iocl100_hook\n"
    //     "blx r2\n"
    //     "_iocl100_hook: .word MCP_ioctl100_patch\n"
    // );

    // ASM_PATCH_K(0x050254D6, 
    //      ".thumb\n"
    //      "bl real_MCP_LoadFile\n"
    // );

    // BL_T_TRAMPOLINE_K(0x050254D6, 0x0501CAA8+1);

    //BRANCH_PATCH_K(0x050254D6, 0x0501CAA8);
    //BRANCH_PATCH(0x050254D6, 1);

    debug_printf("loader patches applied\n");
    debug_printf("0x050254D4: %02X %02X %02X %02X %02X %02X %02X %02X\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    //debug_printf("0x050254D4: %016llX\n", *((uint64_t*) ios_elf_vaddr_to_paddr(0x050254D4)));
}

// This fn runs before MCP's main thread, and can be used
// to perform late patches and spawn threads under MCP.
// It must return.
void mcp_main()
{

}

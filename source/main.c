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


#define _text_start 0x05116000
#define MCP_CODE_BASE_PHYS_ADDR (-0x05100000 + 0x13D80000)
#define mcp_phys_code_base (_text_start + MCP_CODE_BASE_PHYS_ADDR)


void path_print(char* a1, char* relpath, int scratch, int a4){
    debug_printf("relpath: %s\n", relpath);
}

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

    //_patch_copy_k(_text_start, mcp_phys_code_base, 0xA000);
    
    // Loadfile patch
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

    u32 a = 0x1070E934;
    uint8_t *p = (uint8_t*)ios_elf_vaddr_to_paddr(a);
    debug_printf("%p: %02X %02X %02X %02X %02X %02X %02X %02X\n", a, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

    //replace checks with print
    // ASM_PATCH_K(0x1070E930,
    //     "push {R0-R3}\n"
    //     "mov r0, r4\n"
    //     "bl 0x107F0C84\n"
    //     "pop {R0-R3}\n"
    //     "nop\n"
    // );
    //ASM_PATCH_K(0x1070E930, "b 0x1070E944\n");
    //ASM_PATCH_K(0x1070E930, "b 20\n");
    // ASM_PATCH_K(0x1070E930, 
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    //     "nop\n"
    // );
    //worked:
    // ASM_PATCH_K(0x1070E930,
    //     "push {R0-R3}\n"
    //     "mvn r0,#0x108078B0\n"
    //     "mov r0, r4\n"
    // );
    // BL_TRAMPOLINE_K(0x1070E93C, 0x107F0C84);
    // ASM_PATCH_K(0x1070E940,
    //     "pop {R0-R3}\n"
    // );
    // ASM_PATCH_K(0x1070E930,
    //      "LDR r2, pathPrintL\n"
    //      "blx r2\n"
    //      "b pathPrintLA\n"
    //      "pathPrintL: .word path_print\n"
    //      "pathPrintLA: nop\n"
    // );


    // fatal on path too long
    //ASM_PATCH_K(0x1070EAB8, "BL 0x10700000");

    // always print path too long
    // ASM_PATCH_K(0x1070E9C8, "B 0x1070EAAC");

    // print some path hopefully
    //ASM_PATCH_K(0x1070EAB0, "mov r1, r5\nmov r2,r6");


    // ignore path too long
    // ASM_PATCH_K(0x1070EAB0, 
    //     "push {R1-R10,LR}\n"
    //     "mov r1, r5\n");
    // ASM_PATCH_K(0x1070EABC, 
    //     "pop {R1-R10,LR}\n"
    //     "b 0x1070E9CC");


    // ASM_PATCH_K(0x1070E938, "B 0x1070EB2C\n");

    // ASM_PATCH_K(0x1070EB2C, 
    //     "mov r1, r4\n"
    //     "nop\n"
    //     "BL 0x107F0C84\n"
    // )

    //char fspatch[] = "%.5s ";
    //MEMCPY_PATCH(0x10801150, fspatch, strlen(fspatch));

    // debug_printf("%p: %02X %02X %02X %02X %02X %02X %02X %02X\n", a, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

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

    // debug_printf("loader patches applied\n");
    // debug_printf("0x050254D4: %02X %02X %02X %02X %02X %02X %02X %02X\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
    //debug_printf("0x050254D4: %016llX\n", *((uint64_t*) ios_elf_vaddr_to_paddr(0x050254D4)));
}

// This fn runs before MCP's main thread, and can be used
// to perform late patches and spawn threads under MCP.
// It must return.
void mcp_main()
{

}

.extern _MCP_LoadFile_patch
.global MCP_LoadFile_patch
MCP_LoadFile_patch:
    .arm
    ldr R0, [R7,#0xC]
    add r8, lr, #4
    @ldr r12,=0x0501CAA9 @=real_MCP_LoadFile
    ldr r12, =_MCP_LoadFile_patch
    blx r12
    MOVS R4, R0
    bx r8

.extern _MCP_ReadCOSXml_patch
.global MCP_ReadCOSXml_patch
MCP_ReadCOSXml_patch:
    .arm
    add lr, lr, #4
    movs r0, r6
    ldr r2, [r7,#0xc]
    ldr r12, =_MCP_ReadCOSXml_patch
    bx r12

.extern _MCP_ReadCOSXml_patch
.global MCP_ReadCOSXml_patch2
MCP_ReadCOSXml_patch2:
    .arm
    add lr, lr, #4
    movs r1, #0
    movs r2, r5
    ldr r12, =_MCP_ReadCOSXml_patch
    bx r12



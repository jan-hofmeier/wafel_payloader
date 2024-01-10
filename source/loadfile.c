//#include "imports.h"
#include <wafel/services/fsa.h>
#include "loadfile.h"

#include <string.h>
#include <wafel/ios/svc.h>
#include <wafel/utils.h>
#include <wafel/patch.h>

int (*const real_MCP_LoadFile)(ipcmessage *msg) = (void*) (0x0501CAA8 | 1);
int (*const MCP_DoLoadFile)(const char *path, const char *path2, void *outputBuffer, uint32_t outLength, uint32_t pos, int *bytesRead, uint32_t unk) = (void*) (0x05017248 | 1);
int (*const real_MCP_ReadCOSXml_patch)(uint32_t u1, uint32_t u2, MCPPPrepareTitleInfo *xmlData) = (void*) (0x050024ec | 1);
int (*const shellCommand_title_launch)(int argc, char** argv) = (void*) (0x0510c9a0 | 1);

__attribute__((target("thumb")))
static int MCP_LoadCustomFile(void *buffer_out, int buffer_len, int pos)
{
    int fsaFd = FSA_Open();
    FSA_Mount(fsaFd, "/dev/sdcard01", "/vol/storage_homebrew", 2, NULL, 0);
    iosClose(fsaFd);

    int bytesRead = 0;
    int result = MCP_DoLoadFile("/vol/storage_homebrew/wiiu/root.rpx", NULL, buffer_out, buffer_len, pos, &bytesRead, 0);
    if (result >= 0) {
        if (!bytesRead) {
            return 0;
        }
        if (result >= 0) {
            return bytesRead;
        }
    }
    return result;
}

__attribute__((target("arm")))
void undo_patches1(void){
    debug_printf("undoing patches\n");
    // Load File
    ASM_T_PATCH(0x050254D0,
        "bl 0x5027954\n" 
        "ldr R0, [R7,#0xC]\n"
        "bl 0x501caa8\n"
        "movs r4, r0\n"
    );

    // ReadCOS patch
    ASM_T_PATCH(0x0501DD72,
        "ldr r1, [r7, #0x14]\n" 
        "movs r0, r6\n"
        "ldr r2, [r7,#0xc]\n"
        "bl 0x50024ec\n"
    );
    ASM_T_PATCH(0x051105C8,
        ".syntax unified\n"
        "ADDS R0, #0x8E\n" 
        "movs r1, #0\n"
        "movs r2, r5\n"
        "bl 0x50024ec\n"
    );
    debug_printf("done undoing patches\n");
}

const u8 undoLoadFile[] = { 0x68, 0xf8, 0xf7, 0xf7, 0xfa, 0xe7, 0x1c, 0x04 };
const u8 undoCos1[] = { 0x1c, 0x30, 0x68, 0xfa, 0xf7, 0xe4, 0xfb, 0xb8 };

void undo_patches(void){
    debug_printf("undoing patches\n");
    // Load File
    memcpy((void*)0x050254D4, undoLoadFile, sizeof(undoLoadFile));

    // COS 1
    memcpy((void*)0x0501DD74, undoCos1, sizeof(undoCos1));

    debug_printf("done undoing patches\n");
}

__attribute__((target("thumb")))
int __attribute__((used)) _MCP_LoadFile_patch(ipcmessage *msg)
{
    //return real_MCP_LoadFile(msg);
    debug_printf("Inside LoadFile Patch\n");
    MCPLoadFileRequest *request = (MCPLoadFileRequest *) msg->ioctl.buffer_in;

    // we only care about Foreground app/COS-MASTER for now.
    if (request->cafe_pid != 7) {
        return real_MCP_LoadFile(msg);
    }
    debug_printf("LoadFile: %s\n", request->name);
    // Replace the menu RPX (once)
    static int replaced = 0;
    if (!replaced && strncmp(request->name + (strnlen(request->name, 64) - 7), "men.rpx", sizeof("men.rpx")) == 0) {
        replaced = 1;
        undo_patches();
        return MCP_LoadCustomFile(msg->ioctl.buffer_io, msg->ioctl.length_io, request->pos);
    }

    return real_MCP_LoadFile(msg);
}

int __attribute__((used)) _MCP_ReadCOSXml_patch(uint32_t u1, uint32_t u2, MCPPPrepareTitleInfo *xmlData)
{
    int res = real_MCP_ReadCOSXml_patch(u1, u2, xmlData);

    // Give the Wii U menu codegen access for the custom launch.rpx
    if (xmlData->titleId == 0x0005001010040000 ||
        xmlData->titleId == 0x0005001010040100 ||
        xmlData->titleId == 0x0005001010040200) {

        // give title full permissions
        for (uint32_t i = 0; i < 19; i++) {                    
            xmlData->permissions[i].mask = 0xFFFFFFFFFFFFFFFF;
        }

        xmlData->codegen_size = 0x02000000;
        xmlData->codegen_core = 0x80000001;
        xmlData->max_codesize = 0x02800000;
    }

    return res;
}

__attribute__((target("thumb")))
int __attribute__((used)) _MCP_ioctl100_patch(ipcmessage *msg) {
    debug_printf("DNSpresso: MCP_ioctl100_patch\n");

    uint64_t currentColdbootOS = *((uint64_t*)0x050b8174);
    uint64_t currentColdbootTitle = *((uint64_t*)0x050b817c);

    debug_printf("DNSpresso: title0x%016llx os 0x%016llx\n", currentColdbootTitle, currentColdbootOS);

    // This triggers a full cafe relaunch into the specified title
    // not entirely sure how shell commands handle these args but this seems to work
    int argc = 2;
    char* argv[] = {
        "",
        "",
        (char*) (uint32_t) (currentColdbootTitle >> 32),
        (char*) (uint32_t) (currentColdbootTitle & 0xffffffff),
    };
    int res = shellCommand_title_launch(argc, argv);
    debug_printf("DNSpresso: shellCommand_title_launch: %d\n", res);

    return 4;
}

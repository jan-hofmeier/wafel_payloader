//#include "imports.h"
#include <wafel/services/fsa.h>
#include "loadfile.h"

#include <string.h>
#include <wafel/ios/svc.h>
#include <wafel/utils.h>
#include <wafel/patch.h>
#include <wafel/trampoline.h>

#define BL_LoadFile 0x050254d6
#define BL_ReadCos  0x0501dd78

int (*const MCP_DoLoadFile)(const char *path, const char *path2, void *outputBuffer, uint32_t outLength, uint32_t pos, int *bytesRead, uint32_t unk) = (void*) (0x05017248 | 1);

__attribute__((target("thumb")))
static int MCP_LoadCustomFile(void *buffer_out, int buffer_len, int pos)
{
    int fsaFd = FSA_Open();
    FSA_Mount(fsaFd, "/dev/sdcard01", "/vol/storage_homebrew", 2, NULL, 0);

    int bytesRead = 0;
    int result = MCP_DoLoadFile("/vol/storage_homebrew/wiiu/environments/aroma/root.rpx", NULL, buffer_out, buffer_len, pos, &bytesRead, 0);

    FSA_Unmount(fsaFd, "/vol/storage_iosu_homebrew", 0x80000002);
    iosClose(fsaFd);
    
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

u8 undoLoadFile[4];
u8 undoCos1[4];

void undo_patches(void){
    debug_printf("payloader: undoing patches\n");
    // Load File
    memcpy((void*)BL_LoadFile, undoLoadFile, sizeof(undoLoadFile));

    // COS 1
    memcpy((void*)BL_ReadCos, undoCos1, sizeof(undoCos1));

    debug_printf("payloader: done undoing patches\n");
}

__attribute__((target("thumb")))
int __attribute__((used)) MCP_LoadFile_patch(ipcmessage *msg, int r1, int r2, int r3, int (*real_MCP_LoadFile)(ipcmessage *msg))
{
    //return real_MCP_LoadFile(msg);
    debug_printf("Inside LoadFile Patch. real_MCP_LoadFile: %p\n", real_MCP_LoadFile);
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
        int result = MCP_LoadCustomFile(msg->ioctl.buffer_io, msg->ioctl.length_io, request->pos);
        if(result > 0)
            return result;
    }

    return real_MCP_LoadFile(msg);
}

int __attribute__((used)) MCP_ReadCOSXml_patch(uint32_t u1, uint32_t u2, MCPPPrepareTitleInfo *xmlData, int r3, int (*real_MCP_ReadCOSXml)(uint32_t, uint32_t, MCPPPrepareTitleInfo*))
{
    int res = real_MCP_ReadCOSXml(u1, u2, xmlData);

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



void loadfile_install_patches(void){
    memcpy16(undoLoadFile, (void*)ios_elf_vaddr_to_paddr(BL_LoadFile), sizeof(undoLoadFile));
    trampoline_t_blreplace(BL_LoadFile, MCP_LoadFile_patch);
    memcpy16(undoCos1, (void*)ios_elf_vaddr_to_paddr(BL_ReadCos), sizeof(undoCos1));
    trampoline_t_blreplace(BL_ReadCos, MCP_ReadCOSXml_patch);
}

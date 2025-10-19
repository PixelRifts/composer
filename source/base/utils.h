/* date = March 11th 2022 2:07 pm */

#ifndef UTILS_H
#define UTILS_H

#include "defines.h"
#include "mem.h"
#include "str.h"

//~ Time

typedef u64 U_DenseTime;

typedef struct U_DateTime {
    u16 ms;
    u8  sec;
    u8  minute;
    u8  hour;
    u8  day;
    u8  month;
    s32 year;
} U_DateTime;

U_DenseTime U_DenseTimeFromDateTime(U_DateTime* datetime);
U_DateTime  U_DateTimeFromDenseTime(U_DenseTime densetime);

//~ Filepaths

string U_FixFilepath(M_Arena* arena, string filepath);
string U_GetFullFilepath(M_Arena* arena, string filename);
string U_GetFilenameFromFilepath(string filepath);
string U_GetDirectoryFromFilepath(string filepath);

//~ Frame Arena
// This is an arena simply useful for one frame

void     U_FrameArenaInit(void);
void     U_FrameArenaFree(void);
M_Arena* U_GetFrameArena(void);
void     U_ResetFrameArena(void);

//~ Ptr Stack

typedef struct U_PtrStack {
    void** elems;
    u32 len;
    u32 cap;
} U_PtrStack;

void  U_PtrStackInit(U_PtrStack* stack, u32 cap);
b8    U_PtrStackIsEmpty(U_PtrStack* stack);
void  U_PtrStackClear(U_PtrStack* stack);
void  U_PtrStackPush(U_PtrStack* stack, void* elem);
void* U_PtrStackPop(U_PtrStack* stack);
void* U_PtrStackPopSafe(U_PtrStack* stack);
void  U_PtrStackFree(U_PtrStack* stack);

#endif //UTILS_H

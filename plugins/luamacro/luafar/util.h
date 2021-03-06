#ifndef LUAFAR_UTIL_H
#define LUAFAR_UTIL_H

#include <plugin.hpp>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* convert a stack index to positive */
#define abs_index(L,i) ((i)>0 || (i)<=LUA_REGISTRYINDEX ? (i):lua_gettop(L)+(i)+1)

#define LUAFAR_TIMER_CALL  0x1
#define LUAFAR_TIMER_UNREF 0x2

typedef struct
{
	GUID* PluginGuid;
	struct PluginStartupInfo *Info;
	int interval, interval_changed;
	int funcRef, objRef;
	FILETIME tStart;
	int needClose;
	int enabled;
} TTimerData;

typedef struct
{
	TTimerData *timerData;
	int regAction;
	int data;
} TSynchroData;

typedef struct
{
	lua_State         *L;
	struct PluginStartupInfo *Info;
	HANDLE            hDlg;
	BOOL              isOwned;
	BOOL              wasError;
} TDialogData;

int   GetAttrFromTable(lua_State *L);
int   GetIntFromArray(lua_State *L, int index);
int   luaLF_FieldError(lua_State *L, const char* key, const char* expected_typename);
int   luaLF_SlotError(lua_State *L, int key, const char* expected_typename);
void  PushPanelItem(lua_State *L, const struct PluginPanelItem *PanelItem);
void  PushPanelItems(lua_State *L, const struct PluginPanelItem *PanelItems, size_t ItemsNumber);
void  PutMouseEvent(lua_State *L, const MOUSE_EVENT_RECORD* rec, BOOL table_exist);
unsigned __int64 GetFileSizeFromTable(lua_State *L, const char *key);
FILETIME GetFileTimeFromTable(lua_State *L, const char *key);
void  PutFileTimeToTable(lua_State *L, const char* key, FILETIME ft);
TDialogData* NewDialogData(lua_State* L, struct PluginStartupInfo *Info, HANDLE hDlg,
                           BOOL isOwned);
int GetFarColor(lua_State *L, int pos, struct FarColor* Color);
void PushFarColor(lua_State *L, const struct FarColor* Color);

typedef struct
{
	intptr_t X,Y;
	intptr_t Size;
	struct FAR_CHAR_INFO VBuf[1];
} TFarUserControl;

TFarUserControl* CheckFarUserControl(lua_State* L, int pos);

#endif

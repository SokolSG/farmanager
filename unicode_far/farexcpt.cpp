/*
farexcpt.cpp

��� ��� ����������
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "farexcpt.hpp"
#include "plugins.hpp"
#include "macro.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "colors.hpp"
#include "colormix.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "configdb.hpp"
#include "console.hpp"
#include "language.hpp"

int WriteEvent(DWORD DumpType, // FLOG_*
               EXCEPTION_POINTERS *xp,
               Plugin *Module,
               void *RawData,DWORD RawDataSize,
               DWORD RawDataFlags,DWORD RawType)
{
	return 0;
}

/* ************************************************************************
   $ 16.10.2000 SVS
   ����������� ���������� ����������.
*/

static bool Is_STACK_OVERFLOW=false;
bool UseExternalHandler=false;

// Some parametes for _xfilter function
static const wchar_t* From=0;
static EXCEPTION_POINTERS *xp=nullptr;    // ������ ��������
static Plugin *Module=nullptr;     // ������, ��������� � ����������.

extern void CreatePluginStartupInfo(const Plugin *pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF);

intptr_t ExcDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_CTLCOLORDLGITEM:
		{
			FarDialogItem di;
			Dlg->SendMessage(DM_GETDLGITEMSHORT,Param1,&di);

			if (di.Type==DI_EDIT)
			{
				FarColor Color=ColorIndexToColor(COL_WARNDIALOGTEXT);
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
			}
		}
		break;

		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==KEY_EVENT)
			{
				int key = InputRecordToKey(record);
				if (Param1==10 && (key==KEY_LEFT || key == KEY_NUMPAD4 || key==KEY_SHIFTTAB))
				{
					Dlg->SendMessage(DM_SETFOCUS,11,0);
					return TRUE;
				}
				else if (Param1==11 && (key==KEY_RIGHT || key == KEY_NUMPAD6 || key==KEY_TAB))
				{
					Dlg->SendMessage(DM_SETFOCUS,10,0);
					return TRUE;
				}
			}
		}
		break;

		case DN_CLOSE:
		{
			if (Param1 == 10 && !Module) //terminate
			{
				std::terminate();
			}
		}
		break;

	default:
		break;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

static bool LanguageLoaded()
{
	return Global && Global->Lang;
}

static bool ExcDialog(const string& ModuleName,LPCWSTR Exception,LPVOID Adress)
{
	string strAddr = str_printf(L"0x%p",Adress);

	FarDialogItem EditDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,8,0,nullptr,nullptr,0,MSG(MExcTrappedException)},
		{DI_TEXT,     5,2, 17,2,0,nullptr,nullptr,0,MSG(MExcException)},
		{DI_TEXT,    18,2, 70,2,0,nullptr,nullptr,0,Exception},
		{DI_TEXT,     5,3, 17,3,0,nullptr,nullptr,0,MSG(MExcAddress)},
		{DI_TEXT,    18,3, 70,3,0,nullptr,nullptr,0,strAddr.data()},
		{DI_TEXT,     5,4, 17,4,0,nullptr,nullptr,0,MSG(MExcFunction)},
		{DI_TEXT,    18,4, 70,4,0,nullptr,nullptr,0,From},
		{DI_TEXT,     5,5, 17,5,0,nullptr,nullptr,0,MSG(MExcModule)},
		{DI_EDIT,    18,5, 70,5,0,nullptr,nullptr,DIF_READONLY|DIF_SELECTONENTRY,ModuleName.data()},
		{DI_TEXT,    -1,6, 0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_CENTERGROUP,MSG(Module? MExcUnload : MExcTerminate)},
		{DI_BUTTON,   0,7, 0,7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MExcDebugger)},
	};
	auto EditDlg = MakeDialogItemsEx(EditDlgData);
	Dialog Dlg(EditDlg, ExcDlgProc);
	Dlg.SetDialogMode(DMODE_WARNINGSTYLE|DMODE_NOPLUGINS);
	Dlg.SetPosition(-1,-1,76,10);
	Dlg.Process();
	return Dlg.GetExitCode()==11;
}

static bool ExcDump(const string& ModuleName,LPCWSTR Exception,LPVOID Adress)
{
	string strAddr = str_printf(L"0x%p",Adress);

	string Msg[4];
	if (LanguageLoaded())
	{
		Msg[0] = MSG(MExcException);
		Msg[1] = MSG(MExcAddress);
		Msg[2] = MSG(MExcFunction);
		Msg[3] = MSG(MExcModule);
	}
	else
	{
		Msg[0] = L"Exception:";
		Msg[1] = L"Address:  ";
		Msg[2] = L"Function: ";
		Msg[3] = L"Module:   ";
	}

	string Dump =
		Msg[0] + L" " + Exception + L"\n" +
		Msg[1] + L" " + strAddr + L"\n" +
		Msg[2] + L" " + From + L"\n" +
		Msg[3] + L" " + ModuleName + L"\n";

	std::wcerr << Dump << std::endl;

	return false;
}

static DWORD WINAPI _xfilter(LPVOID dummy=nullptr)
{
	if (Global)
		Global->ProcessException=TRUE;
	DWORD Result = EXCEPTION_EXECUTE_HANDLER;
	BOOL Res=FALSE;

	if (!Is_STACK_OVERFLOW &&Global && Global->Opt->ExceptUsed && !Global->Opt->strExceptEventSvc.empty())
	{
		HMODULE m = LoadLibrary(Global->Opt->strExceptEventSvc.data());

		if (m)
		{
			typedef BOOL (WINAPI *ExceptionProc_t)(EXCEPTION_POINTERS *xp,
				                                    const PLUGINRECORD *Module,
				                                    const PluginStartupInfo *LocalStartupInfo,
				                                    LPDWORD Result);
			ExceptionProc_t p = (ExceptionProc_t)GetProcAddress(m,"ExceptionProc");

			if (p)
			{
				static PluginStartupInfo LocalStartupInfo;
				ClearStruct(LocalStartupInfo);
				static FarStandardFunctions LocalStandardFunctions;
				ClearStruct(LocalStandardFunctions);
				CreatePluginStartupInfo(nullptr, &LocalStartupInfo, &LocalStandardFunctions);
				LocalStartupInfo.ModuleName = Global->Opt->strExceptEventSvc.data();
				static PLUGINRECORD PlugRec;

				if (Module)
				{
					ClearStruct(PlugRec);
					PlugRec.TypeRec=RTYPE_PLUGIN;
					PlugRec.SizeRec=sizeof(PLUGINRECORD);
					PlugRec.ModuleName=Module->GetModuleName().data();
					PlugRec.WorkFlags=Module->GetWorkFlags();
					PlugRec.CallFlags=Module->GetFuncFlags();
					PlugRec.FuncFlags=0;
					PlugRec.FuncFlags|=Module->HasGetGlobalInfo()?PICFF_GETGLOBALINFO:0;
					PlugRec.FuncFlags|=Module->HasSetStartupInfo()?PICFF_SETSTARTUPINFO:0;
					PlugRec.FuncFlags|=Module->HasOpen()?PICFF_OPENPANEL:0;
					PlugRec.FuncFlags|=Module->HasAnalyse()?PICFF_ANALYSE:0;
					PlugRec.FuncFlags|=Module->HasClosePanel()?PICFF_CLOSEPANEL:0;
					PlugRec.FuncFlags|=Module->HasGetPluginInfo()?PICFF_GETPLUGININFO:0;
					PlugRec.FuncFlags|=Module->HasGetOpenPanelInfo()?PICFF_GETOPENPANELINFO:0;
					PlugRec.FuncFlags|=Module->HasGetFindData()?PICFF_GETFINDDATA:0;
					PlugRec.FuncFlags|=Module->HasFreeFindData()?PICFF_FREEFINDDATA:0;
					PlugRec.FuncFlags|=Module->HasGetVirtualFindData()?PICFF_GETVIRTUALFINDDATA:0;
					PlugRec.FuncFlags|=Module->HasFreeVirtualFindData()?PICFF_FREEVIRTUALFINDDATA:0;
					PlugRec.FuncFlags|=Module->HasSetDirectory()?PICFF_SETDIRECTORY:0;
					PlugRec.FuncFlags|=Module->HasGetFiles()?PICFF_GETFILES:0;
					PlugRec.FuncFlags|=Module->HasPutFiles()?PICFF_PUTFILES:0;
					PlugRec.FuncFlags|=Module->HasDeleteFiles()?PICFF_DELETEFILES:0;
					PlugRec.FuncFlags|=Module->HasMakeDirectory()?PICFF_MAKEDIRECTORY:0;
					PlugRec.FuncFlags|=Module->HasProcessHostFile()?PICFF_PROCESSHOSTFILE:0;
					PlugRec.FuncFlags|=Module->HasSetFindList()?PICFF_SETFINDLIST:0;
					PlugRec.FuncFlags|=Module->HasConfigure()?PICFF_CONFIGURE:0;
					PlugRec.FuncFlags|=Module->HasExitFAR()?PICFF_EXITFAR:0;
					PlugRec.FuncFlags|=Module->HasProcessPanelInput()?PICFF_PROCESSPANELINPUT:0;
					PlugRec.FuncFlags|=Module->HasProcessPanelEvent()?PICFF_PROCESSPANELEVENT:0;
					PlugRec.FuncFlags|=Module->HasProcessEditorEvent()?PICFF_PROCESSEDITOREVENT:0;
					PlugRec.FuncFlags|=Module->HasCompare()?PICFF_COMPARE:0;
					PlugRec.FuncFlags|=Module->HasProcessEditorInput()?PICFF_PROCESSEDITORINPUT:0;
					PlugRec.FuncFlags|=Module->HasGetMinFarVersion()?PICFF_MINFARVERSION:0;
					PlugRec.FuncFlags|=Module->HasProcessViewerEvent()?PICFF_PROCESSVIEWEREVENT:0;
					PlugRec.FuncFlags|=Module->HasProcessDialogEvent()?PICFF_PROCESSDIALOGEVENT:0;
					PlugRec.FuncFlags|=Module->HasProcessSynchroEvent()?PICFF_PROCESSSYNCHROEVENT:0;
					PlugRec.FuncFlags|=Module->HasProcessConsoleInput()?PICFF_PROCESSCONSOLEINPUT:0;
					PlugRec.FuncFlags|=Module->HasCloseAnalyse()?PICFF_CLOSEANALYSE:0;
				}

				Res=p(xp,(Module?&PlugRec:nullptr),&LocalStartupInfo,&Result);
			}

			FreeLibrary(m);
		}
	}

	if (Res)
	{
		if (!Module)
		{
			if (Global)
				Global->CriticalInternalError=TRUE;
		}

		return Result;
	}

	struct __ECODE
	{
		NTSTATUS Code;     // ��� ����������
		const wchar_t* DefaultMsg; // Lng-files may not be loaded yet
		LNGID IdMsg;    // ID ��������� �� LNG-�����
		DWORD RetCode;  // ��� ������?
	} ECode[]=

	{
		#define CODEANDTEXT(x) x, L###x
		{CODEANDTEXT(EXCEPTION_ACCESS_VIOLATION), MExcRAccess, EXCEPTION_EXECUTE_HANDLER},
		{CODEANDTEXT(EXCEPTION_ARRAY_BOUNDS_EXCEEDED), MExcOutOfBounds, EXCEPTION_EXECUTE_HANDLER},
		{CODEANDTEXT(EXCEPTION_INT_DIVIDE_BY_ZERO),MExcDivideByZero, EXCEPTION_EXECUTE_HANDLER},
		{CODEANDTEXT(EXCEPTION_STACK_OVERFLOW),MExcStackOverflow, EXCEPTION_EXECUTE_HANDLER},
		{CODEANDTEXT(EXCEPTION_BREAKPOINT),MExcBreakPoint, EXCEPTION_EXECUTE_HANDLER},
		{CODEANDTEXT(EXCEPTION_FLT_DIVIDE_BY_ZERO),MExcFloatDivideByZero,EXCEPTION_EXECUTE_HANDLER}, // BUGBUG: Floating-point exceptions (VC) are disabled by default. See http://msdn2.microsoft.com/en-us/library/aa289157(vs.71).aspx#floapoint_topic8
		{CODEANDTEXT(EXCEPTION_FLT_OVERFLOW),MExcFloatOverflow,EXCEPTION_EXECUTE_HANDLER},           // BUGBUG:  ^^^
		{CODEANDTEXT(EXCEPTION_FLT_STACK_CHECK),MExcFloatStackOverflow,EXCEPTION_EXECUTE_HANDLER},   // BUGBUG:  ^^^
		{CODEANDTEXT(EXCEPTION_FLT_UNDERFLOW),MExcFloatUnderflow,EXCEPTION_EXECUTE_HANDLER},         // BUGBUG:  ^^^
		{CODEANDTEXT(EXCEPTION_ILLEGAL_INSTRUCTION),MExcBadInstruction,EXCEPTION_EXECUTE_HANDLER},
		{CODEANDTEXT(EXCEPTION_PRIV_INSTRUCTION),MExcBadInstruction,EXCEPTION_EXECUTE_HANDLER},
		{CODEANDTEXT(EXCEPTION_DATATYPE_MISALIGNMENT), MExcDatatypeMisalignment, EXCEPTION_EXECUTE_HANDLER},
		// ���� ���������.

		#undef CODEANDTEXT
	};
	// EXCEPTION_CONTINUE_EXECUTION  ??????
	DWORD rc;
	string strBuf1, strBuf2;
	LangString strBuf;
	string strFileName;
	BOOL ShowMessages=FALSE;
	// ������� ������ ����������
	EXCEPTION_RECORD *xr = xp->ExceptionRecord;

	// ������� ���� ����� ������� ���������
	WriteEvent(FLOG_ALL,xp,Module,nullptr,0);

	// CONTEXT ����� ������������ ��� ����������� ��� ������ � ���
	//         ����������� ���������...
	// CONTEXT *xc = xp->ContextRecord;
	rc = Result;// EXCEPTION_EXECUTE_HANDLER;
	/*$ 23.01.2001 skv
	  ����������� ���������� �� ����� ������������.
	*/

	if (!Module)
	{
		if (Global)
		{
			strFileName=Global->g_strFarModuleName;
		}
		else
		{
			api::GetModuleFileName(nullptr, strFileName);
		}
	}
	else
	{
		strFileName = Module->GetModuleName();
	}

	LPCWSTR Exception=nullptr;
	// ���������� "��������" FAR`� ���������� � ����������...
	auto ItemIterator = std::find_if(CONST_RANGE(ECode, i)
	{
		return i.Code == static_cast<NTSTATUS>(xr->ExceptionCode);
	});

	if (ItemIterator != std::cend(ECode))
	{
		Exception=LanguageLoaded()? MSG(ItemIterator->IdMsg) : ItemIterator->DefaultMsg;
		rc=ItemIterator->RetCode;

		if (xr->ExceptionCode == static_cast<DWORD>(EXCEPTION_ACCESS_VIOLATION))
		{
			int Offset = 0;
			// ��� ������ �� ���� ����� ����������� ����������� ����
			// if ( xr->ExceptionInformation[0] == 8 ) Offset = 2 else Offset = xr->ExceptionInformation[0],
			// � �� M$ �������� ��� ���-������ xr->ExceptionInformation[0] == 4 � ��� ����� � ������ ����.

			switch (xr->ExceptionInformation[0])
			{
				case 0:
					Offset = 0;
					break;
				case 1:
					Offset = 1;
					break;
				case 8:
					Offset = 2;
					break;
			}

			strBuf2 = str_printf(L"0x%p", xr->ExceptionInformation[1]+10);
			if (LanguageLoaded())
			{
				strBuf = MExcRAccess+Offset;
				strBuf << strBuf2;
				Exception=strBuf.data();
			}
			else
			{
				const wchar_t* AVs[] = {L"read from ", L"write to ", L"execute at "};
				strBuf1 = Exception;
				strBuf1.append(L" (").append(AVs[Offset]).append(strBuf2).append(L")");
				Exception=strBuf1.data();
			}
		}
	}

	if (!Exception)
	{
		const wchar_t* Template = LanguageLoaded()? MSG(MExcUnknown) : L"Unknown exception";
		strBuf2 = str_printf(L"%s (0x%X)", Template, xr->ExceptionCode);
		Exception = strBuf2.data();
	}

	int MsgCode=0;
	if (Global && Global->FrameManager && !Global->FrameManager->ManagerIsDown())
	{
		MsgCode=ExcDialog(strFileName,Exception,xr->ExceptionAddress);
		ShowMessages=TRUE;
	}
	else
	{
		MsgCode = ExcDump(strFileName,Exception,xr->ExceptionAddress);
	}

	if (ShowMessages && (Is_STACK_OVERFLOW || !Module))
	{
		Global->CriticalInternalError=TRUE;
	}

	if(MsgCode==1)
	{
		SetErrorMode(Global->ErrorMode&~SEM_NOGPFAULTERRORBOX);
		rc=EXCEPTION_CONTINUE_SEARCH;
		UseExternalHandler=true;
	}
	else
	{
		rc = EXCEPTION_EXECUTE_HANDLER;
	}
	//return UnhandledExceptionFilter(xp);
	return rc;
}

DWORD WINAPI xfilter(Plugin *Module, const wchar_t* From, EXCEPTION_POINTERS *xp)
{
	DWORD Result=EXCEPTION_CONTINUE_SEARCH;

	if(!UseExternalHandler)
	{
		// dummy parametrs setting
		::From=From;
		::xp=xp;
		::Module=Module;

		if (xp->ExceptionRecord->ExceptionCode == static_cast<DWORD>(STATUS_STACK_OVERFLOW)) // restore stack & call_xfilter ;
		{
			Is_STACK_OVERFLOW=true;
#ifdef _M_IA64
			// TODO: Bad way to restore IA64 stacks (CreateThread)
			// Can you do smartly? See REMINDER file, section IA64Stacks
			static HANDLE hThread = nullptr;

			if (!(hThread = CreateThread(nullptr, 0, _xfilter, nullptr, 0, nullptr)))
			{
				std::terminate();
			}

			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
#else
			static struct
			{
				BYTE      stack_space[32768];
				intptr_t ret_addr;
				intptr_t args[4];
			} _stack;
			_stack.ret_addr = 0;
#ifndef _WIN64
#ifdef _M_ARM
			// BUGBUG
#else
			//_stack.args[0] = (intptr_t)From;
			//_stack.args[1] = (intptr_t)xp;
			//_stack.args[2] = (intptr_t)Module;
			xp->ContextRecord->Esp = (DWORD)(intptr_t)(&_stack.ret_addr);
			xp->ContextRecord->Eip = (DWORD)(intptr_t)(&_xfilter);
#endif
#else
			//xp->ContextRecord->Rcx = (intptr_t)From;
			//xp->ContextRecord->Rdx = (intptr_t)xp;
			//xp->ContextRecord->R8  = (intptr_t)Module;
			xp->ContextRecord->Rsp = (intptr_t)(&_stack.ret_addr);
			xp->ContextRecord->Rip = (intptr_t)(&_xfilter);
#endif
#endif
			Result=EXCEPTION_CONTINUE_EXECUTION;
		}
		else
		{
			Result=_xfilter();
		}
	}
	return Result;
}

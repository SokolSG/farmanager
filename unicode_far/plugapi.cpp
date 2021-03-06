/*
plugapi.cpp

API, ��������� �������� (�������, ����, ...)
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

#include "plugapi.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "scantree.hpp"
#include "rdrwdsk.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "plugins.hpp"
#include "savescr.hpp"
#include "flink.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "frame.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "lockscrn.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "colormix.hpp"
#include "message.hpp"
#include "eject.hpp"
#include "filefilter.hpp"
#include "fileowner.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "exitcode.hpp"
#include "processname.hpp"
#include "RegExp.hpp"
#include "TaskBar.hpp"
#include "console.hpp"
#include "plugsettings.hpp"
#include "farversion.hpp"
#include "mix.hpp"
#include "FarGuid.hpp"
#include "clipboard.hpp"
#include "strmix.hpp"
#include "PluginSynchro.hpp"
#include "copy.hpp"
#include "panelmix.hpp"
#include "xlat.hpp"
#include "dirinfo.hpp"
#include "language.hpp"

namespace pluginapi
{
inline Plugin* GuidToPlugin(const GUID* Id) {return (Id && Global->CtrlObject)? Global->CtrlObject->Plugins->FindPlugin(*Id) : nullptr;}

int WINAPIV apiSprintf(wchar_t* Dest, const wchar_t* Format, ...) //?deprecated
{
	va_list argptr;
	va_start(argptr, Format);
	int Result = _vsnwprintf(Dest, 32000, Format, argptr); //vswprintf(Det,L"%s",(const char *)str) -- MinGW gcc >= 4.6
	va_end(argptr);
	return Result;
}

int WINAPIV apiSnprintf(wchar_t* Dest, size_t Count, const wchar_t* Format, ...)
{
	va_list argptr;
	va_start(argptr, Format);
	int Result =  _vsnwprintf(Dest, Count, Format, argptr);
	va_end(argptr);
	return Result;
}

#ifndef _MSC_VER
int WINAPIV apiSscanf(const wchar_t* Src, const wchar_t* Format, ...)
{
	va_list argptr;
	va_start(argptr, Format);
	int Result = vswscanf(Src, Format, argptr);
	va_end(argptr);
	return Result;
}
#endif

wchar_t *WINAPI apiItoa(int value, wchar_t *string, int radix)
{
	if (string)
		return _itow(value,string,radix);

	return nullptr;
}

wchar_t *WINAPI apiItoa64(__int64 value, wchar_t *string, int radix)
{
	if (string)
		return _i64tow(value, string, radix);

	return nullptr;
}

int WINAPI apiAtoi(const wchar_t *s)
{
	if (s)
		return _wtoi(s);

	return 0;
}
__int64 WINAPI apiAtoi64(const wchar_t *s)
{
	return s?_wtoi64(s):0;
}

namespace cfunctions
{
	void qsortex(char *base, size_t nel, size_t width, int (WINAPI *comp_fp)(const void *, const void *,void*), void *user);
	void* bsearchex(const void* key,const void* base,size_t nelem,size_t width,int (WINAPI *fcmp)(const void*, const void*,void*),void* userparam);
};

void WINAPI apiQsort(void *base, size_t nelem, size_t width, int (WINAPI *fcmp)(const void *, const void *,void *),void *user)
{
	if (base && fcmp)
	{
		cfunctions::qsortex((char*)base,nelem,width,fcmp,user);
	}
}

void *WINAPI apiBsearch(const void *key, const void *base, size_t nelem, size_t width, int (WINAPI *fcmp)(const void *, const void *, void *),void *user)
{
	if (key && fcmp && base)
		return cfunctions::bsearchex(key,base,nelem,width,fcmp,user);

	return nullptr;
}

wchar_t* WINAPI apiQuoteSpace(wchar_t *Str)
{
	return QuoteSpace(Str);
}

wchar_t* WINAPI apiInsertQuote(wchar_t *Str)
{
	return InsertQuote(Str);
}

void WINAPI apiUnquote(wchar_t *Str)
{
	return Unquote(Str);
}

wchar_t* WINAPI apiRemoveLeadingSpaces(wchar_t *Str)
{
	return RemoveLeadingSpaces(Str);
}

wchar_t * WINAPI apiRemoveTrailingSpaces(wchar_t *Str)
{
	return RemoveTrailingSpaces(Str);
}

wchar_t* WINAPI apiRemoveExternalSpaces(wchar_t *Str)
{
	return RemoveExternalSpaces(Str);
}

wchar_t* WINAPI apiQuoteSpaceOnly(wchar_t *Str)
{
	return QuoteSpaceOnly(Str);
}

intptr_t WINAPI apiInputBox(
    const GUID* PluginId,
    const GUID* Id,
    const wchar_t *Title,
    const wchar_t *Prompt,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    wchar_t *DestText,
    size_t DestSize,
    const wchar_t *HelpTopic,
    unsigned __int64 Flags
)
{
	if (Global->FrameManager->ManagerIsDown())
		return FALSE;

	string strDest;
	int nResult = GetString(Title,Prompt,HistoryName,SrcText,strDest,HelpTopic,Flags&~FIB_CHECKBOX,nullptr,nullptr,GuidToPlugin(PluginId),Id);
	xwcsncpy(DestText, strDest.data(), DestSize);
	return nResult;
}

/* ������� ������ ������ */
BOOL WINAPI apiShowHelp(
    const wchar_t *ModuleName,
    const wchar_t *HelpTopic,
    FARHELPFLAGS Flags
)
{
	if (Global->FrameManager->ManagerIsDown())
		return FALSE;

	if (!HelpTopic)
		HelpTopic=L"Contents";

	UINT64 OFlags=Flags;
	Flags&=~(FHELP_NOSHOWERROR|FHELP_USECONTENTS);
	string strTopic;
	string strMask;

	// ��������� � ������ ������ ���� �� ������������ � � ��� ������,
	// ���� ����� FHELP_FARHELP...
	if ((Flags&FHELP_FARHELP) || *HelpTopic==L':')
	{
		strTopic = HelpTopic+((*HelpTopic == L':')?1:0);
	}
	else if (ModuleName && (Flags&FHELP_GUID))
	{
		if (!*ModuleName || *reinterpret_cast<const GUID*>(ModuleName) == FarGuid)
		{
			Flags|=FHELP_FARHELP;
			strTopic = HelpTopic+((*HelpTopic == L':')?1:0);
		}
		else
		{
			Plugin* plugin = Global->CtrlObject->Plugins->FindPlugin(*reinterpret_cast<const GUID*>(ModuleName));
			if (plugin)
			{
				Flags|=FHELP_CUSTOMPATH;
				strTopic = Help::MakeLink(ExtractFilePath(plugin->GetModuleName()), HelpTopic);
			}
		}
	}
	else
	{
		if (ModuleName)
		{
			// FHELP_SELFHELP=0 - ���������� ������ ���-� ��� Info.ModuleName
			//                   � �������� ����� �� ����� ���������� �������
			/* $ 17.11.2000 SVS
			   � �������� FHELP_SELFHELP ����� ����? ��������� - 0
			   � ����� ����� ��������� ����, ��� ������� �� �������� :-(
			*/
			string strPath;
			if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE|FHELP_CUSTOMPATH)))
			{
				strPath = ModuleName;

				if (Flags == FHELP_SELFHELP || (Flags&(FHELP_CUSTOMFILE)))
				{
					if (Flags&FHELP_CUSTOMFILE)
						strMask=PointToName(strPath);
					else
						strMask.clear();

					CutToSlash(strPath);
				}
			}
			else
				return FALSE;

			strTopic = Help::MakeLink(strPath, HelpTopic);
		}
		else
			return FALSE;
	}

	{
		Help Hlp(strTopic,strMask.data(),OFlags);

		if (Hlp.GetError())
			return FALSE;
	}

	return TRUE;
}

/* $ 05.07.2000 IS
  �������, ������� ����� ����������� � � ���������, � � �������, �...
*/
intptr_t WINAPI apiAdvControl(const GUID* PluginId, ADVANCED_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{
	if (ACTL_SYNCHRO==Command) //must be first
	{
		Global->PluginSynchroManager->Synchro(true, *PluginId, Param2);
		return 0;
	}
	if (ACTL_GETWINDOWTYPE==Command)
	{
		WindowType* info=(WindowType*)Param2;
		if (CheckStructSize(info))
		{
			WINDOWINFO_TYPE type=ModalType2WType(Manager::GetCurrentWindowType());
			switch(type)
			{
			case WTYPE_PANELS:
			case WTYPE_VIEWER:
			case WTYPE_EDITOR:
			case WTYPE_DIALOG:
			case WTYPE_VMENU:
			case WTYPE_HELP:
				info->Type=type;
				return TRUE;
			default:
				break;
			}
		}
		return FALSE;
	}

	switch (Command)
	{
		case ACTL_GETFARMANAGERVERSION:
		case ACTL_GETCOLOR:
		case ACTL_GETARRAYCOLOR:
		case ACTL_GETFARHWND:
		case ACTL_SETPROGRESSSTATE:
		case ACTL_SETPROGRESSVALUE:
		case ACTL_GETFARRECT:
		case ACTL_GETCURSORPOS:
		case ACTL_SETCURSORPOS:
		case ACTL_PROGRESSNOTIFY:
			break;
		default:

			if (Global->FrameManager->ManagerIsDown())
				return 0;
	}

	switch (Command)
	{
		case ACTL_GETFARMANAGERVERSION:
		{
			if (Param2)
				*(VersionInfo*)Param2=FAR_VERSION;

			return TRUE;
		}
		/* $ 24.08.2000 SVS
		   ������� ������������ (��� �����) �������
		   (const INPUT_RECORD*)Param2 - ��� �������, ������� �������, ��� nullptr
		   ���� ��� ����� ����� ������� �����.
		   ���������� 0;
		*/
		case ACTL_WAITKEY:
		{
			return WaitKey(Param2?InputRecordToKey((const INPUT_RECORD*)Param2):-1,0,false);
		}
		/* $ 04.12.2000 SVS
		  ACTL_GETCOLOR - �������� ������������ ���� �� �������, �������������
		   � farcolor.hpp
		  Param2 - [OUT] �������� �����
		  Return - TRUE ���� OK ��� FALSE ���� ������ �������.
		*/
		case ACTL_GETCOLOR:
		{
			if (static_cast<UINT>(Param1) < Global->Opt->Palette.size())
			{
				*static_cast<FarColor*>(Param2) = Global->Opt->Palette[static_cast<size_t>(Param1)];
				return TRUE;
			}
			return FALSE;
		}
		/* $ 04.12.2000 SVS
		  ACTL_GETARRAYCOLOR - �������� ���� ������ ������
		  Param1 - ������ ������ (� ��������� FarColor)
		  Param2 - ��������� �� ����� ��� nullptr, ����� �������� ����������� ������
		  Return - ������ �������.
		*/
		case ACTL_GETARRAYCOLOR:
		{
			if (Param2 && static_cast<size_t>(Param1) >= Global->Opt->Palette.size())
			{
				Global->Opt->Palette.CopyTo(reinterpret_cast<FarColor*>(Param2));
			}
			return Global->Opt->Palette.size();
		}
		/*
		  Param=FARColor{
		    DWORD Flags;
		    int StartIndex;
		    int ColorItem;
		    LPBYTE Colors;
		  };
		*/
		case ACTL_SETARRAYCOLOR:
		{
			FarSetColors *Pal=(FarSetColors*)Param2;
			if (CheckStructSize(Pal))
			{

				if (Pal->Colors && Pal->StartIndex+Pal->ColorsCount <= Global->Opt->Palette.size())
				{
					Global->Opt->Palette.Set(Pal->StartIndex, Pal->Colors, Pal->ColorsCount);
					if (Pal->Flags&FSETCLR_REDRAW)
					{
						Global->ScrBuf->Lock(); // �������� ������ ����������
						Global->FrameManager->ResizeAllFrame();
						Global->FrameManager->PluginCommit(); // ��������.
						Global->ScrBuf->Unlock(); // ��������� ����������
					}

					return TRUE;
				}
			}

			return FALSE;
		}
		/* $ 14.12.2000 SVS
		  ACTL_EJECTMEDIA - ������� ���� �� �������� ����������
		  Param - ��������� �� ��������� ActlEjectMedia
		  Return - TRUE - �������� ����������, FALSE - ������.
		*/
		case ACTL_EJECTMEDIA:
		{
			return CheckStructSize((ActlEjectMedia*)Param2)?EjectVolume((wchar_t)((ActlEjectMedia*)Param2)->Letter,
			                         ((ActlEjectMedia*)Param2)->Flags):FALSE;
			/*
			      if(Param)
			      {
							ActlEjectMedia *aem=(ActlEjectMedia *)Param;
			        char DiskLetter[4]=" :\\";
			        DiskLetter[0]=(char)aem->Letter;
			        int DriveType = FAR_GetDriveType(DiskLetter,nullptr,FALSE); // ����� �� ���������� ��� CD

			        if(DriveType == DRIVE_USBDRIVE && RemoveUSBDrive((char)aem->Letter,aem->Flags))
			          return TRUE;
			        if(DriveType == DRIVE_SUBSTITUTE && DelSubstDrive(DiskLetter))
			          return TRUE;
			        if(IsDriveTypeCDROM(DriveType) && EjectVolume((char)aem->Letter,aem->Flags))
			          return TRUE;

			      }
			      return FALSE;
			*/
		}
		/*
		    case ACTL_GETMEDIATYPE:
		    {
					ActlMediaType *amt=(ActlMediaType *)Param;
		      char DiskLetter[4]=" :\\";
		      DiskLetter[0]=(amt)?(char)amt->Letter:0;
		      return FAR_GetDriveType(DiskLetter,nullptr,(amt && !(amt->Flags&MEDIATYPE_NODETECTCDROM)));
		    }
		*/
		/* $ 05.06.2001 tran
		   ����� ACTL_ ��� ������ � �������� */
		case ACTL_GETWINDOWINFO:
		{
			WindowInfo *wi=(WindowInfo*)Param2;
			if (CheckStructSize(wi))
			{
				string strType, strName;
				Frame *f=nullptr;
				bool modal=false;

				/* $ 22.12.2001 VVM
				  + ���� Pos == -1 �� ����� ������� ����� */
				if (wi->Pos == -1)
				{
					f = Global->FrameManager->GetCurrentFrame();
					modal=(Global->FrameManager->IndexOfStack(f)>=0);
				}
				else
				{
					if (wi->Pos >= 0 && wi->Pos < static_cast<intptr_t>(Global->FrameManager->GetFrameCount()))
					{
						f = Global->FrameManager->GetFrame(wi->Pos);
					}
					else if(wi->Pos >= static_cast<intptr_t>(Global->FrameManager->GetFrameCount()) && wi->Pos < static_cast<intptr_t>(Global->FrameManager->GetFrameCount() + Global->FrameManager->GetModalStackCount()))
					{
						f = Global->FrameManager->GetModalFrame(wi->Pos - Global->FrameManager->GetFrameCount());
						modal=true;
					}
				}

				if (!f)
					return FALSE;

				f->GetTypeAndName(strType, strName);

				if (wi->TypeNameSize && wi->TypeName)
				{
					xwcsncpy(wi->TypeName,strType.data(),wi->TypeNameSize);
				}
				else
				{
					wi->TypeNameSize=strType.size()+1;
				}

				if (wi->NameSize && wi->Name)
				{
					xwcsncpy(wi->Name,strName.data(),wi->NameSize);
				}
				else
				{
					wi->NameSize=strName.size()+1;
				}

				if(-1==wi->Pos) wi->Pos = Global->FrameManager->IndexOf(f);
				if(-1==wi->Pos) wi->Pos = Global->FrameManager->IndexOfStack(f) + Global->FrameManager->GetFrameCount();
				wi->Type=ModalType2WType(f->GetType());
				wi->Flags=0;
				if (f->IsFileModified())
					wi->Flags|=WIF_MODIFIED;
				if (f == Global->FrameManager->GetCurrentFrame())
					wi->Flags|=WIF_CURRENT;
				if (modal)
					wi->Flags|=WIF_MODAL;

				switch (wi->Type)
				{
					case WTYPE_VIEWER:
						wi->Id=static_cast<FileViewer*>(f)->GetId();
						break;
					case WTYPE_EDITOR:
						wi->Id=static_cast<FileEditor*>(f)->GetId();
						break;
					case WTYPE_VMENU:
					case WTYPE_DIALOG:
						wi->Id=(intptr_t)f;
						break;
					default:
						wi->Id=0;
						break;
				}
				return TRUE;
			}

			return FALSE;
		}
		case ACTL_GETWINDOWCOUNT:
		{
			return Global->FrameManager->GetFrameCount() + Global->FrameManager->GetModalStackCount();
		}
		case ACTL_SETCURRENTWINDOW:
		{
			// �������� ������������ �������, ���� ��������� � ��������� ���������/������.
			if (!Global->FrameManager->InModalEV() && Global->FrameManager->GetFrame(Param1))
			{
				int TypeFrame = Global->FrameManager->GetCurrentFrame()->GetType();

				// �������� ������������ �������, ���� ��������� � ����� ��� ������� (���� ���������)
				if (TypeFrame != MODALTYPE_HELP && TypeFrame != MODALTYPE_DIALOG)
				{
					Frame* PrevFrame = Global->FrameManager->GetCurrentFrame();
					Global->FrameManager->ActivateFrame(Param1);
					Global->FrameManager->DeactivateFrame(PrevFrame, 0);
					return TRUE;
				}
			}

			return FALSE;
		}
		/*$ 26.06.2001 SKV
		  ��� ����������� ������ � ACTL_SETCURRENTWINDOW
		  (� ����� ��� ��� ���� � �������)
		*/
		case ACTL_COMMIT:
		{
			return Global->FrameManager->PluginCommit();
		}
		/* $ 15.09.2001 tran
		   ���������� �������� */
		case ACTL_GETFARHWND:
		{
			return (intptr_t)Global->Console->GetWindow();
		}
		case ACTL_REDRAWALL:
		{
			int Ret = Global->FrameManager->ProcessKey(KEY_CONSOLE_BUFFER_RESIZE);
			Global->FrameManager->PluginCommit();
			return Ret;
		}

		case ACTL_SETPROGRESSSTATE:
		{
			Global->TBC->SetProgressState(static_cast<TBPFLAG>(Param1));
			return TRUE;
		}

		case ACTL_SETPROGRESSVALUE:
		{
			BOOL Result=FALSE;
			ProgressValue* PV=static_cast<ProgressValue*>(Param2);
			if(CheckStructSize(PV))
			{
				Global->TBC->SetProgressValue(PV->Completed,PV->Total);
				Result=TRUE;
			}
			return Result;
		}

		case ACTL_QUIT:
		{
			Global->CloseFARMenu=TRUE;
			Global->FrameManager->ExitMainLoop(FALSE);
			return TRUE;
		}

		case ACTL_GETFARRECT:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					SMALL_RECT& Rect=*static_cast<PSMALL_RECT>(Param2);
					if(Global->Opt->WindowMode)
					{
						Result=Global->Console->GetWorkingRect(Rect);
					}
					else
					{
						COORD Size;
						if(Global->Console->GetSize(Size))
						{
							Rect.Left=0;
							Rect.Top=0;
							Rect.Right=Size.X-1;
							Rect.Bottom=Size.Y-1;
							Result=TRUE;
						}
					}
				}
				return Result;
			}
			break;

		case ACTL_GETCURSORPOS:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					COORD& Pos=*static_cast<PCOORD>(Param2);
					Result=Global->Console->GetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_SETCURSORPOS:
			{
				BOOL Result=FALSE;
				if(Param2)
				{
					COORD& Pos=*static_cast<PCOORD>(Param2);
					Result=Global->Console->SetCursorPosition(Pos);
				}
				return Result;
			}
			break;

		case ACTL_PROGRESSNOTIFY:
		{
			Global->TBC->Flash();
			return TRUE;
		}

		default:
			break;

	}

	return FALSE;
}

static DWORD NormalizeControlKeys(DWORD Value)
{
	DWORD result=Value&(LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED);
	if(Value&RIGHT_CTRL_PRESSED) result|=LEFT_CTRL_PRESSED;
	if(Value&RIGHT_ALT_PRESSED) result|=LEFT_ALT_PRESSED;
	return result;
}

intptr_t WINAPI apiMenuFn(
    const GUID* PluginId,
    const GUID* Id,
    intptr_t X,
    intptr_t Y,
    intptr_t MaxHeight,
    unsigned __int64 Flags,
    const wchar_t *Title,
    const wchar_t *Bottom,
    const wchar_t *HelpTopic,
    const FarKey *BreakKeys,
    intptr_t *BreakCode,
    const FarMenuItem *Item,
    size_t ItemsNumber
)
{
	if (Global->FrameManager->ManagerIsDown())
		return -1;

	if (Global->DisablePluginsOutput)
		return -1;

	int ExitCode;
	{
		VMenu2 FarMenu(NullToEmpty(Title),nullptr,0,MaxHeight);
		Global->CtrlObject->Macro.SetMode(MACROAREA_MENU);
		FarMenu.SetPosition(X,Y,0,0);
		if(Id)
		{
			FarMenu.SetId(*Id);
		}

		if (BreakCode)
			*BreakCode=-1;

		{
			string strTopic;

			if (Help::MkTopic(GuidToPlugin(PluginId), NullToEmpty(HelpTopic), strTopic))
				FarMenu.SetHelp(strTopic);
		}

		if (Bottom)
			FarMenu.SetBottomTitle(Bottom);

		// ����� ����� ����
		DWORD MenuFlags=0;

		if (Flags & FMENU_SHOWAMPERSAND)
			MenuFlags|=VMENU_SHOWAMPERSAND;

		if (Flags & FMENU_WRAPMODE)
			MenuFlags|=VMENU_WRAPMODE;

		if (Flags & FMENU_CHANGECONSOLETITLE)
			MenuFlags|=VMENU_CHANGECONSOLETITLE;

		FarMenu.SetFlags(MenuFlags);
		size_t Selected=0;

		for (size_t i=0; i < ItemsNumber; i++)
		{
			MenuItemEx CurItem;
			CurItem.Flags=Item[i].Flags;
			CurItem.strName.clear();
			// ��������� MultiSelected, �.�. � ��� ������ ������ � ����� �� ������������, ��������� ������ ������
			DWORD SelCurItem=CurItem.Flags&LIF_SELECTED;
			CurItem.Flags&=~LIF_SELECTED;

			if (!Selected && !(CurItem.Flags&LIF_SEPARATOR) && SelCurItem)
			{
				CurItem.Flags|=SelCurItem;
				Selected++;
			}

			CurItem.strName=NullToEmpty(Item[i].Text);
			if(CurItem.Flags&LIF_SEPARATOR)
			{
				CurItem.AccelKey=0;
			}
			else
			{
				INPUT_RECORD input = {};
				FarKeyToInputRecord(Item[i].AccelKey,&input);
				CurItem.AccelKey=InputRecordToKey(&input);
			}
			FarMenu.AddItem(CurItem);
		}

		if (!Selected)
			FarMenu.SetSelectPos(0,1);

		// ����� ����, � ������� ���������
		if (Flags & FMENU_AUTOHIGHLIGHT)
			FarMenu.AssignHighlights(FALSE);

		if (Flags & FMENU_REVERSEAUTOHIGHLIGHT)
			FarMenu.AssignHighlights(TRUE);

		FarMenu.SetTitle(NullToEmpty(Title));

		ExitCode=FarMenu.RunEx([&](int Msg, void *param)->int
		{
			if (Msg!=DN_INPUT || !BreakKeys)
				return 0;

			INPUT_RECORD *ReadRec=static_cast<INPUT_RECORD*>(param);
			int ReadKey=InputRecordToKey(ReadRec);

			if (ReadKey==KEY_NONE)
				return 0;

			for (int I=0; BreakKeys[I].VirtualKeyCode; I++)
			{
				if (Global->CtrlObject->Macro.IsExecuting())
				{
					int VirtKey,ControlState;
					TranslateKeyToVK(ReadKey,VirtKey,ControlState,ReadRec);
				}

				if (ReadRec->Event.KeyEvent.wVirtualKeyCode==BreakKeys[I].VirtualKeyCode)
				{
					if (NormalizeControlKeys(ReadRec->Event.KeyEvent.dwControlKeyState) == NormalizeControlKeys(BreakKeys[I].ControlKeyState))
					{
						if (BreakCode)
							*BreakCode=I;

						FarMenu.Close(-2, true);
						return 1;
					}
				}
			}
			return 0;
		});
	}
//  CheckScreenLock();
	return ExitCode;
}

// ������� FarDefDlgProc ��������� ������� �� ���������
intptr_t WINAPI apiDefDlgProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	if (hDlg) // ��������� ������ ����� ��� hDlg=0
		return static_cast<Dialog*>(hDlg)->DefProc(Msg,Param1,Param2);

	return 0;
}

// ������� ��������� �������
intptr_t WINAPI apiSendDlgMessage(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	if (hDlg) // ��������� ������ ����� ��� hDlg=0
		return static_cast<Dialog*>(hDlg)->SendMessage(Msg,Param1,Param2);

	return 0;
}

HANDLE WINAPI apiDialogInit(const GUID* PluginId, const GUID* Id, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2,
                            const wchar_t *HelpTopic, const FarDialogItem *Item,
                            size_t ItemsNumber, intptr_t Reserved, unsigned __int64 Flags,
                            FARWINDOWPROC DlgProc, void* Param)
{
	HANDLE hDlg=INVALID_HANDLE_VALUE;

	if (Global->FrameManager->ManagerIsDown())
		return hDlg;

	if (Global->DisablePluginsOutput || !ItemsNumber || !Item)
		return hDlg;

	// ����! ������ ��������� ������������� X2 � Y2
	if (X2 < 0 || Y2 < 0)
		return hDlg;

	{
		Dialog *FarDialog = new PluginDialog(pass_as_container(Item, ItemsNumber), DlgProc, Param);

		if (FarDialog->InitOK())
		{
			hDlg = FarDialog;
			FarDialog->SetPosition(X1,Y1,X2,Y2);

			if (Flags & FDLG_WARNING)
				FarDialog->SetDialogMode(DMODE_WARNINGSTYLE);

			if (Flags & FDLG_SMALLDIALOG)
				FarDialog->SetDialogMode(DMODE_SMALLDIALOG);

			if (Flags & FDLG_NODRAWSHADOW)
				FarDialog->SetDialogMode(DMODE_NODRAWSHADOW);

			if (Flags & FDLG_NODRAWPANEL)
				FarDialog->SetDialogMode(DMODE_NODRAWPANEL);

			if (Flags & FDLG_KEEPCONSOLETITLE)
				FarDialog->SetDialogMode(DMODE_KEEPCONSOLETITLE);

			if (Flags & FDLG_NONMODAL)
				FarDialog->SetCanLoseFocus(TRUE);

			FarDialog->SetHelp(NullToEmpty(HelpTopic));

			FarDialog->SetId(*Id);
			/* $ 29.08.2000 SVS
			   �������� ����� ������� - ������ � �������� ��� ������������ HelpTopic
			*/
			FarDialog->SetPluginOwner(GuidToPlugin(PluginId));
		}
		else
		{
			delete FarDialog;
		}
	}
	return hDlg;
}

intptr_t WINAPI apiDialogRun(HANDLE hDlg)
{
	if (Global->FrameManager->ManagerIsDown())
		return -1;

	if (hDlg==INVALID_HANDLE_VALUE)
		return -1;

	Dialog *FarDialog = (Dialog *)hDlg;

	FarDialog->Process();
	int ExitCode=FarDialog->GetExitCode();

	if (Global->IsMainThread()) // BUGBUG, findfile
		Global->FrameManager->RefreshFrame(); //?? - //AY - ��� ����� ���� ��������� ������ ����� ������ �� �������

	return ExitCode;
}

void WINAPI apiDialogFree(HANDLE hDlg)
{
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		delete static_cast<PluginDialog*>(hDlg);
	}
}

const wchar_t* WINAPI apiGetMsgFn(const GUID* PluginId,intptr_t MsgId)
{
	Plugin *pPlugin = GuidToPlugin(PluginId);
	if (pPlugin)
	{
		string strPath = pPlugin->GetModuleName();
		CutToSlash(strPath);

		if (pPlugin->InitLang(strPath))
			return pPlugin->GetMsg(static_cast<LNGID>(MsgId));
	}
	return L"";
}

intptr_t WINAPI apiMessageFn(const GUID* PluginId,const GUID* Id,unsigned __int64 Flags,const wchar_t *HelpTopic,
                        const wchar_t * const *Items,size_t ItemsNumber,
                        intptr_t ButtonsNumber)
{
	if (Flags&FMSG_ERRORTYPE)
		Global->CatchError();

	if (Global->FrameManager->ManagerIsDown())
		return -1;

	if (Global->DisablePluginsOutput)
		return -1;

	if ((!(Flags&(FMSG_ALLINONE|FMSG_ERRORTYPE)) && ItemsNumber<2) || !Items)
		return -1;

	wchar_t_ptr SingleItems;

	// ������ ���������� ����� ��� FMSG_ALLINONE
	if (Flags&FMSG_ALLINONE)
	{
		ItemsNumber=0;

		SingleItems.reset(StrLength(reinterpret_cast<const wchar_t*>(Items)) + 2);
		if (!SingleItems)
			return -1;

		wchar_t *Msg=wcscpy(SingleItems.get(), (const wchar_t *)Items);

		while ((Msg = wcschr(Msg, L'\n')) )
		{
			if (*++Msg == L'\0')
				break;

			++ItemsNumber;
		}

		ItemsNumber++; //??
	}

	std::vector<const wchar_t*> MsgItems(ItemsNumber+ADDSPACEFORPSTRFORMESSAGE);

	if (Flags&FMSG_ALLINONE)
	{
		int I=0;
		wchar_t *Msg=SingleItems.get();
		// ������ ���������� ����� � �������� �� ������
		wchar_t *MsgTemp;

		while ((MsgTemp = wcschr(Msg, L'\n')) )
		{
			*MsgTemp=L'\0';
			MsgItems[I]=Msg;
			Msg=MsgTemp+1;

			if (*Msg == L'\0')
				break;

			++I;
		}

		if (*Msg)
		{
			MsgItems[I]=Msg;
		}
	}
	else
	{
		for (size_t i=0; i < ItemsNumber; i++)
			MsgItems[i]=Items[i];
	}

	/* $ 22.03.2001 tran
	   ItemsNumber++ -> ++ItemsNumber
	   �������� ��������� ������� */
	switch (Flags&0x000F0000)
	{
		case FMSG_MB_OK:
			ButtonsNumber=1;
			MsgItems[ItemsNumber++]=MSG(MOk);
			break;
		case FMSG_MB_OKCANCEL:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MOk);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
		case FMSG_MB_ABORTRETRYIGNORE:
			ButtonsNumber=3;
			MsgItems[ItemsNumber++]=MSG(MAbort);
			MsgItems[ItemsNumber++]=MSG(MRetry);
			MsgItems[ItemsNumber++]=MSG(MIgnore);
			break;
		case FMSG_MB_YESNO:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MYes);
			MsgItems[ItemsNumber++]=MSG(MNo);
			break;
		case FMSG_MB_YESNOCANCEL:
			ButtonsNumber=3;
			MsgItems[ItemsNumber++]=MSG(MYes);
			MsgItems[ItemsNumber++]=MSG(MNo);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
		case FMSG_MB_RETRYCANCEL:
			ButtonsNumber=2;
			MsgItems[ItemsNumber++]=MSG(MRetry);
			MsgItems[ItemsNumber++]=MSG(MCancel);
			break;
	}

	// ����������� �� ������
	size_t MaxLinesNumber = static_cast<size_t>(ScrY-3-(ButtonsNumber?1:0));
	size_t LinesNumber = ItemsNumber-ButtonsNumber-1;
	if (LinesNumber > MaxLinesNumber)
	{
		ItemsNumber -= (LinesNumber-MaxLinesNumber);
		for (int i=1; i <= ButtonsNumber; i++)
			MsgItems[MaxLinesNumber+i]=MsgItems[LinesNumber+i];
	}

	Plugin* PluginNumber = GuidToPlugin(PluginId);
	// ���������� �����
	string strTopic;
	if (PluginNumber)
	{
		Help::MkTopic(PluginNumber,NullToEmpty(HelpTopic),strTopic);
	}

	// ���������������... �����
	Frame *frame = Global->FrameManager->GetBottomFrame();

	if (frame)
		frame->Lock(); // ������� ���������� ������

	int MsgCode=Message(Flags&(FMSG_WARNING|FMSG_ERRORTYPE|FMSG_KEEPBACKGROUND|FMSG_LEFTALIGN), ButtonsNumber, MsgItems[0], &MsgItems[1], ItemsNumber-1, EmptyToNull(strTopic.data()), PluginNumber, Id);

	/* $ 15.05.2002 SKV
	  ������ ����������� ���� ����� ��, ��� ��������.
	*/
	if (frame)
		frame->Unlock(); // ������ ����� :-)

	//CheckScreenLock();

	return MsgCode;
}

intptr_t WINAPI apiPanelControl(HANDLE hPlugin,FILE_CONTROL_COMMANDS Command,intptr_t Param1,void* Param2)
{
	_FCTLLOG(CleverSysLog CSL(L"Control"));
	_FCTLLOG(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));
	_ALGO(CleverSysLog clv(L"FarPanelControl"));
	_ALGO(SysLog(L"(hPlugin=0x%08X, Command=%s, Param1=[%d/0x%08X], Param2=[%d/0x%08X])",hPlugin,_FCTL_ToName(Command),(int)Param1,Param1,(int)Param2,Param2));

	if (Command == FCTL_CHECKPANELSEXIST)
		return !Global->Opt->OnlyEditorViewerUsed;

	if (Global->Opt->OnlyEditorViewerUsed || !Global->CtrlObject || Global->FrameManager->ManagerIsDown())
		return 0;

	FilePanels *FPanels=Global->CtrlObject->Cp();
	CommandLine *CmdLine=Global->CtrlObject->CmdLine;

	switch (Command)
	{
		case FCTL_CLOSEPANEL:
			Global->g_strDirToSet = NullToEmpty((wchar_t *)Param2);
		case FCTL_GETPANELINFO:
		case FCTL_GETPANELITEM:
		case FCTL_GETSELECTEDPANELITEM:
		case FCTL_GETCURRENTPANELITEM:
		case FCTL_GETPANELDIRECTORY:
		case FCTL_GETCOLUMNTYPES:
		case FCTL_GETCOLUMNWIDTHS:
		case FCTL_UPDATEPANEL:
		case FCTL_REDRAWPANEL:
		case FCTL_SETPANELDIRECTORY:
		case FCTL_BEGINSELECTION:
		case FCTL_SETSELECTION:
		case FCTL_CLEARSELECTION:
		case FCTL_ENDSELECTION:
		case FCTL_SETVIEWMODE:
		case FCTL_SETSORTMODE:
		case FCTL_SETSORTORDER:
		case FCTL_SETNUMERICSORT:
		case FCTL_SETCASESENSITIVESORT:
		case FCTL_SETDIRECTORIESFIRST:
		case FCTL_GETPANELFORMAT:
		case FCTL_GETPANELHOSTFILE:
		case FCTL_GETPANELPREFIX:
		case FCTL_SETACTIVEPANEL:
		{
			if (!FPanels)
				return FALSE;

			if (!hPlugin || hPlugin == PANEL_ACTIVE || hPlugin == PANEL_PASSIVE)
			{
				Panel *pPanel = (!hPlugin || hPlugin == PANEL_ACTIVE)?FPanels->ActivePanel:FPanels->GetAnotherPanel(FPanels->ActivePanel);

				if (Command == FCTL_SETACTIVEPANEL && hPlugin == PANEL_ACTIVE)
					return TRUE;

				if (pPanel)
				{
					return pPanel->SetPluginCommand(Command,Param1,Param2);
				}

				return FALSE; //???
			}

			HANDLE hInternal;
			Panel *LeftPanel=FPanels->LeftPanel;
			Panel *RightPanel=FPanels->RightPanel;
			int Processed=FALSE;

			if (LeftPanel && LeftPanel->GetMode()==PLUGIN_PANEL)
			{
				auto PlHandle = LeftPanel->GetPluginHandle();

				if (PlHandle)
				{
					hInternal=PlHandle->hPlugin;

					if (hPlugin==hInternal)
					{
						Processed=LeftPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			if (RightPanel && RightPanel->GetMode()==PLUGIN_PANEL)
			{
				auto PlHandle = RightPanel->GetPluginHandle();

				if (PlHandle)
				{
					hInternal=PlHandle->hPlugin;

					if (hPlugin==hInternal)
					{
						Processed=RightPanel->SetPluginCommand(Command,Param1,Param2);
					}
				}
			}

			return Processed;
		}
		case FCTL_SETUSERSCREEN:
		{
			if (!FPanels || !FPanels->LeftPanel || !FPanels->RightPanel)
				return FALSE;

			Global->KeepUserScreen++;
			FPanels->LeftPanel->ProcessingPluginCommand++;
			FPanels->RightPanel->ProcessingPluginCommand++;
			Global->ScrBuf->FillBuf();
			ScrollScreen(1);
			SaveScreen SaveScr;
			{
				RedrawDesktop Redraw;
				CmdLine->Hide();
				SaveScr.RestoreArea(FALSE);
			}
			Global->KeepUserScreen--;
			FPanels->LeftPanel->ProcessingPluginCommand--;
			FPanels->RightPanel->ProcessingPluginCommand--;
			return TRUE;
		}
		case FCTL_GETUSERSCREEN:
		{
			Global->FrameManager->ShowBackground();
			int Lock=Global->ScrBuf->GetLockCount();
			Global->ScrBuf->SetLockCount(0);
			MoveCursor(0,ScrY-1);
			SetInitialCursorType();
			Global->ScrBuf->Flush();
			Global->ScrBuf->SetLockCount(Lock);
			return TRUE;
		}
		case FCTL_GETCMDLINE:
		{
			string strParam;

			CmdLine->GetString(strParam);

			if (Param1&&Param2)
				xwcsncpy((wchar_t*)Param2,strParam.data(),Param1);

			return (int)strParam.size()+1;
		}
		case FCTL_SETCMDLINE:
		case FCTL_INSERTCMDLINE:
		{
			{
				SCOPED_ACTION(SetAutocomplete)(CmdLine);
				if (Command==FCTL_SETCMDLINE)
					CmdLine->SetString((const wchar_t*)Param2);
				else
					CmdLine->InsertString((const wchar_t*)Param2);
			}
			CmdLine->Redraw();
			return TRUE;
		}
		case FCTL_SETCMDLINEPOS:
		{
			CmdLine->SetCurPos(Param1);
			CmdLine->Redraw();
			return TRUE;
		}
		case FCTL_GETCMDLINEPOS:
		{
			if (Param2)
			{
				*(int *)Param2=CmdLine->GetCurPos();
				return TRUE;
			}

			return FALSE;
		}
		case FCTL_GETCMDLINESELECTION:
		{
			CmdLineSelect *sel=(CmdLineSelect*)Param2;
			if (CheckStructSize(sel))
			{
				CmdLine->GetSelection(sel->SelStart,sel->SelEnd);
				return TRUE;
			}

			return FALSE;
		}
		case FCTL_SETCMDLINESELECTION:
		{
			CmdLineSelect *sel=(CmdLineSelect*)Param2;
			if (CheckStructSize(sel))
			{
				CmdLine->Select(sel->SelStart,sel->SelEnd);
				CmdLine->Redraw();
				return TRUE;
			}

			return FALSE;
		}
		case FCTL_ISACTIVEPANEL:
		{
			if (!hPlugin || hPlugin == PANEL_ACTIVE)
				return TRUE;

			Panel *pPanel = FPanels->ActivePanel;

			if (pPanel && (pPanel->GetMode() == PLUGIN_PANEL))
			{
				auto PlHandle = pPanel->GetPluginHandle();

				if (PlHandle)
				{
					if (PlHandle->hPlugin == hPlugin)
						return TRUE;
				}
			}

			return FALSE;
		}
		default:
			break;
	}

	return FALSE;
}


HANDLE WINAPI apiSaveScreen(intptr_t X1,intptr_t Y1,intptr_t X2,intptr_t Y2)
{
	if (Global->DisablePluginsOutput || Global->FrameManager->ManagerIsDown())
		return nullptr;

	if (X2==-1)
		X2=ScrX;

	if (Y2==-1)
		Y2=ScrY;

	return new SaveScreen(X1,Y1,X2,Y2);
}


void WINAPI apiRestoreScreen(HANDLE hScreen)
{
	if (Global->DisablePluginsOutput || Global->FrameManager->ManagerIsDown())
		return;

	if (!hScreen)
		Global->ScrBuf->FillBuf();

	if (hScreen)
		delete(SaveScreen *)hScreen;
}

void FreeDirList(std::vector<PluginPanelItem>* Items)
{
	std::for_each(ALL_RANGE(*Items), FreePluginPanelItem);
	delete Items;
}

intptr_t WINAPI apiGetDirList(const wchar_t *Dir,PluginPanelItem **pPanelItem,size_t *pItemsNumber)
{
	if (Global->FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
		return FALSE;

	string strDirName;
	ConvertNameToFull(Dir, strDirName);
	{
		auto PR_FarGetDirListMsg = [](){ Message(0,0,L"",MSG(MPreparingList)); };

		SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<PreRedrawItem>(PR_FarGetDirListMsg));
		SCOPED_ACTION(SaveScreen);
		api::FAR_FIND_DATA FindData;
		string strFullName;
		ScanTree ScTree(false);
		ScTree.SetFindPath(strDirName,L"*");
		*pItemsNumber=0;
		*pPanelItem=nullptr;

		auto Items = new std::vector<PluginPanelItem>;

		DWORD StartTime=GetTickCount();
		bool MsgOut = false;
		while (ScTree.GetNextName(&FindData,strFullName))
		{
			DWORD CurTime=GetTickCount();
			if (CurTime-StartTime>static_cast<DWORD>(Global->Opt->RedrawTimeout))
			{
				if (CheckForEsc())
				{
					FreeDirList(Items);
					return FALSE;
				}

				if (!MsgOut)
				{
					SetCursorType(false, 0);
					PR_FarGetDirListMsg();
					MsgOut = true;
				}
			}

			if (Items->size() == Items->capacity())
			{
				Items->reserve(Items->size() + 4096);
			}

			Items->emplace_back(VALUE_TYPE(*Items)());
			auto& Item = Items->back();
			ClearStruct(Item);
			FindData.strFileName = strFullName;
			FindDataExToPluginPanelItem(&FindData, &Item);
		}

		*pItemsNumber=Items->size();

		// magic trick to store vector pointer for apiFreeDirList().
		Items->emplace_back(VALUE_TYPE(*Items)());
		Items->back().Reserved[0] = reinterpret_cast<intptr_t>(Items);

		*pPanelItem=Items->data();
	}
	return TRUE;
}

intptr_t WINAPI apiGetPluginDirList(const GUID* PluginId, HANDLE hPlugin, const wchar_t *Dir, PluginPanelItem **pPanelItem, size_t *pItemsNumber)
{
	if (Global->FrameManager->ManagerIsDown() || !Dir || !*Dir || !pItemsNumber || !pPanelItem)
		return FALSE;
	return GetPluginDirList(GuidToPlugin(PluginId), hPlugin, Dir, pPanelItem, pItemsNumber);
}

void WINAPI apiFreeDirList(PluginPanelItem *PanelItem, size_t nItemsNumber)
{
	auto Items = reinterpret_cast<std::vector<PluginPanelItem>*>(PanelItem[nItemsNumber].Reserved[0]);
	Items->pop_back(); // not needed anymore, see magic trick above
	return FreeDirList(Items);
}

void WINAPI apiFreePluginDirList(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber)
{
	FreePluginDirList(hPlugin, PanelItem);
}

intptr_t WINAPI apiViewer(const wchar_t *FileName,const wchar_t *Title,
                     intptr_t X1,intptr_t Y1,intptr_t X2, intptr_t Y2,unsigned __int64 Flags, uintptr_t CodePage)
{
	if (Global->FrameManager->ManagerIsDown())
		return FALSE;

	class ConsoleTitle ct;
	int DisableHistory = (Flags & VF_DISABLEHISTORY) != 0;

	// $ 15.05.2002 SKV - �������� ����� ������������ ��������� ������ �� ����������.
	if (Global->FrameManager->InModalEV())
	{
		Flags&=~VF_NONMODAL;
	}

	if (Flags & VF_NONMODAL)
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileViewer *Viewer=new FileViewer(FileName,TRUE,DisableHistory,Title,X1,Y1,X2,Y2,CodePage);

		if (!Viewer)
			return FALSE;

		/* $ 14.06.2002 IS
		   ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
		   ��������� �� ��������� � VF_DELETEONCLOSE
		*/
		if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
			Viewer->SetTempViewName(FileName, (Flags&VF_DELETEONCLOSE) != 0);

		Viewer->SetEnableF6(Flags & VF_ENABLE_F6);

		/* $ 21.05.2002 SKV
		  ��������� ���� ���� ������ ���� �� ��� ������ ����.
		*/
		if (!(Flags&VF_IMMEDIATERETURN))
		{
			Global->FrameManager->ExecuteNonModal();
		}
		else
		{
			if (Global->GlobalSaveScrPtr)
				Global->GlobalSaveScrPtr->Discard();

			Global->FrameManager->PluginCommit();
		}
	}
	else
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileViewer Viewer(FileName,FALSE,DisableHistory,Title,X1,Y1,X2,Y2,CodePage);

		Viewer.SetEnableF6(Flags & VF_ENABLE_F6);

		/* $ 28.05.2001 �� ��������� �����, ������� ����� ����� ������� ��������� ���� */
		Viewer.SetDynamicallyBorn(false);
		Global->FrameManager->EnterModalEV();
		Global->FrameManager->ExecuteModal();
		Global->FrameManager->ExitModalEV();

		/* $ 14.06.2002 IS
		   ��������� VF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
		   ��������� �� ��������� � VF_DELETEONCLOSE
		*/
		if (Flags & (VF_DELETEONCLOSE|VF_DELETEONLYFILEONCLOSE))
			Viewer.SetTempViewName(FileName, (Flags&VF_DELETEONCLOSE) != 0);

		if (!Viewer.GetExitCode())
		{
			return FALSE;
		}
	}

	return TRUE;
}

intptr_t WINAPI apiEditor(const wchar_t* FileName, const wchar_t* Title, intptr_t X1, intptr_t Y1, intptr_t X2, intptr_t Y2, unsigned __int64 Flags, intptr_t StartLine, intptr_t StartChar, uintptr_t CodePage)
{
	if (Global->FrameManager->ManagerIsDown())
		return EEC_OPEN_ERROR;

	ConsoleTitle ct;
	/* $ 12.07.2000 IS
	 �������� ������ ��������� (������ ��� ��������������) � ��������
	 ������������ ���������, ���� ���� ��������������� ����
	*/
	int CreateNew = (Flags & EF_CREATENEW) != 0;
	int Locked=(Flags & EF_LOCKED) != 0;
	int DisableHistory=(Flags & EF_DISABLEHISTORY) != 0;
	int DisableSavePos=(Flags & EF_DISABLESAVEPOS) != 0;
	/* $ 14.06.2002 IS
	   ��������� EF_DELETEONLYFILEONCLOSE - ���� ���� ����� ����� ������
	   ��������� �� ��������� � EF_DELETEONCLOSE
	*/
	int DeleteOnClose = 0;

	if (Flags & EF_DELETEONCLOSE)
		DeleteOnClose = 1;
	else if (Flags & EF_DELETEONLYFILEONCLOSE)
		DeleteOnClose = 2;

	int OpMode=FEOPMODE_QUERY;

	if ((Flags&EF_OPENMODE_MASK) )
		OpMode=Flags&EF_OPENMODE_MASK;

	/*$ 15.05.2002 SKV
	  �������� ����� ������������ ���������, ���� ��������� � ���������
	  ��������� ��� ������.
	*/
	if (Global->FrameManager->InModalEV())
	{
		Flags&=~EF_NONMODAL;
	}

	int editorExitCode;
	int ExitCode=EEC_OPEN_ERROR;
	string strTitle(NullToEmpty(Title));

	if (Flags & EF_NONMODAL)
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileEditor *Editor=new FileEditor(NullToEmpty(FileName), CodePage,
		                                  (CreateNew?FFILEEDIT_CANNEWFILE:0)|FFILEEDIT_ENABLEF6|
		                                   (DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|
		                                   (Locked?FFILEEDIT_LOCKED:0)|
		                                   (DisableSavePos?FFILEEDIT_DISABLESAVEPOS:0),
		                                  StartLine,StartChar,&strTitle,
		                                  X1,Y1,X2,Y2,
		                                  DeleteOnClose,OpMode);

		if (Editor)
		{
			editorExitCode=Editor->GetExitCode();

			// ��������� - �������� ���� �������� (������ ��������� XC_OPEN_ERROR - ��. ��� FileEditor::Init())
			if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
			{
				delete Editor;
				Editor=nullptr;
				return editorExitCode;
			}

			Editor->SetEnableF6((Flags & EF_ENABLE_F6)!=0);
			Editor->SetPluginTitle(&strTitle);

			/* $ 21.05.2002 SKV - ��������� ���� ����, ������ ���� �� ��� ������ ����. */
			if (!(Flags&EF_IMMEDIATERETURN))
			{
				Global->FrameManager->ExecuteNonModal();
			}
			else
			{
				if (Global->GlobalSaveScrPtr)
					Global->GlobalSaveScrPtr->Discard();

				Global->FrameManager->PluginCommit();
			}

			ExitCode=XC_MODIFIED;
		}
	}
	else
	{
		/* 09.09.2001 IS ! ������� ��� ����� � �������, ���� ����������� */
		FileEditor Editor(FileName,CodePage,
		                  (CreateNew?FFILEEDIT_CANNEWFILE:0)|
		                    (DisableHistory?FFILEEDIT_DISABLEHISTORY:0)|
		                    (Locked?FFILEEDIT_LOCKED:0)|
		                    (DisableSavePos?FFILEEDIT_DISABLESAVEPOS:0),
		                  StartLine,StartChar,&strTitle,
		                  X1,Y1,X2,Y2,
		                  DeleteOnClose,OpMode);
		editorExitCode=Editor.GetExitCode();

		// �������� ������������ (������ ������ ����� ����)
		if (editorExitCode == XC_OPEN_ERROR || editorExitCode == XC_LOADING_INTERRUPTED)
			ExitCode=editorExitCode;
		else
		{
			Editor.SetDynamicallyBorn(false);
			Editor.SetEnableF6((Flags & EF_ENABLE_F6)!=0);
			Editor.SetPluginTitle(&strTitle);
			/* $ 15.05.2002 SKV
			  ����������� ���� � ����� �/�� ���������� ���������.
			*/
			Global->FrameManager->EnterModalEV();
			Global->FrameManager->ExecuteModal();
			Global->FrameManager->ExitModalEV();
			ExitCode = Editor.GetExitCode();

			if (ExitCode)
			{
#if 0

				if (OpMode==FEOPMODE_BREAKIFOPEN && ExitCode==XC_QUIT)
					ExitCode = XC_OPEN_ERROR;
				else
#endif
					ExitCode = Editor.IsFileChanged()?XC_MODIFIED:XC_NOT_MODIFIED;
			}
		}
	}

	return ExitCode;
}

void WINAPI apiText(intptr_t X,intptr_t Y,const FarColor* Color,const wchar_t *Str)
{
	if (Global->DisablePluginsOutput || Global->FrameManager->ManagerIsDown())
		return;

	if (!Str)
	{
		int PrevLockCount=Global->ScrBuf->GetLockCount();
		Global->ScrBuf->SetLockCount(0);
		Global->ScrBuf->Flush();
		Global->ScrBuf->SetLockCount(PrevLockCount);
	}
	else
	{
		Text(X,Y,*Color,Str);
	}
}

intptr_t WINAPI apiEditorControl(intptr_t EditorID, EDITOR_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{
	if (Global->FrameManager->ManagerIsDown())
		return 0;

	FileEditor* currentEditor=Global->CtrlObject->Plugins->GetCurEditor();
	if (currentEditor && currentEditor->GetId() == EditorID) EditorID = -1;

	if (EditorID == -1)
	{
		if (currentEditor)
			return currentEditor->EditorControl(Command,Param1,Param2);

		return 0;
	}
	else
	{
		static const simple_pair<decltype(&Manager::GetFrame), decltype(&Manager::GetFrameCount)> Functions[] =
		{
			{&Manager::GetFrame, &Manager::GetFrameCount},
			{&Manager::GetModalFrame, &Manager::GetModalStackCount},
		};

		FOR(const auto& i, Functions)
		{
			size_t count=(Global->FrameManager->*i.second)();
			for(size_t j = 0;j < count; ++j)
			{
				Frame *frame=(Global->FrameManager->*i.first)(j);
				if (frame->GetType() == MODALTYPE_EDITOR)
				{
					if (((FileEditor*)frame)->GetId() == EditorID)
					{
						return ((FileEditor*)frame)->EditorControl(Command,Param1,Param2);
					}
				}
			}
		}
	}

	return 0;
}

intptr_t WINAPI apiViewerControl(intptr_t ViewerID, VIEWER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{
	if (Global->FrameManager->ManagerIsDown())
		return 0;

	if (ViewerID == -1)
	{
		if (Global->CtrlObject->Plugins->GetCurViewer())
			return Global->CtrlObject->Plugins->GetCurViewer()->ViewerControl(Command,Param1,Param2);

		return 0;
	}
	else
	{
		int idx=0;
		Frame *frame;
		while((frame = Global->FrameManager->GetFrame(idx++)))
		{
			if (frame->GetType() == MODALTYPE_VIEWER)
			{
				if (((FileViewer*)frame)->GetId() == ViewerID)
				{
					return ((FileViewer*)frame)->ViewerControl(Command,Param1,Param2);
				}
			}
		}
	}

	return 0;
}

void WINAPI apiUpperBuf(wchar_t *Buf, intptr_t Length)
{
	return UpperBuf(Buf, Length);
}

void WINAPI apiLowerBuf(wchar_t *Buf, intptr_t Length)
{
	return LowerBuf(Buf, Length);
}

void WINAPI apiStrUpper(wchar_t *s1)
{
	return StrUpper(s1);
}

void WINAPI apiStrLower(wchar_t *s1)
{
	return StrLower(s1);
}

wchar_t WINAPI apiUpper(wchar_t Ch)
{
	return Upper(Ch);
}

wchar_t WINAPI apiLower(wchar_t Ch)
{
	return Lower(Ch);
}

int WINAPI apiStrCmpNI(const wchar_t *s1, const wchar_t *s2, intptr_t n)
{
	return StrCmpNI(s1, s2, n);
}

int WINAPI apiStrCmpI(const wchar_t *s1, const wchar_t *s2)
{
	return StrCmpI(s1, s2);
}

int WINAPI apiIsLower(wchar_t Ch)
{
	return IsLower(Ch);
}

int WINAPI apiIsUpper(wchar_t Ch)
{
	return IsUpper(Ch);
}

int WINAPI apiIsAlpha(wchar_t Ch)
{
	return IsAlpha(Ch);
}

int WINAPI apiIsAlphaNum(wchar_t Ch)
{
	return IsAlphaNum(Ch);
}

wchar_t* WINAPI apiTruncStr(wchar_t *Str,intptr_t MaxLength)
{
	return TruncStr(Str, MaxLength);
}

wchar_t* WINAPI apiTruncStrFromCenter(wchar_t *Str, intptr_t MaxLength)
{
	return TruncStrFromCenter(Str, MaxLength);
}

wchar_t* WINAPI apiTruncStrFromEnd(wchar_t *Str,intptr_t MaxLength)
{
	return TruncStrFromEnd(Str, MaxLength);
}

wchar_t* WINAPI apiTruncPathStr(wchar_t *Str, intptr_t MaxLength)
{
	return TruncPathStr(Str, MaxLength);
}

const wchar_t* WINAPI apiPointToName(const wchar_t *lpwszPath)
{
	return PointToName(lpwszPath);
}

size_t WINAPI apiGetFileOwner(const wchar_t *Computer,const wchar_t *Name, wchar_t *Owner,size_t Size)
{
	string strOwner;
	GetFileOwner(NullToEmpty(Computer), NullToEmpty(Name), strOwner);

	if (Owner && Size)
		xwcsncpy(Owner,strOwner.data(),Size);

	return strOwner.size()+1;
}

size_t WINAPI apiConvertPath(CONVERTPATHMODES Mode,const wchar_t *Src, wchar_t *Dest, size_t DestSize)
{
	if (Src && *Src)
	{
		string strDest;

		switch (Mode)
		{
			case CPM_NATIVE:
				strDest=NTPath(Src);
				break;
			case CPM_REAL:
				ConvertNameToReal(Src, strDest);
				break;
			case CPM_FULL:
			default:
				ConvertNameToFull(Src, strDest);
				break;
		}

		if (Dest && DestSize)
			xwcsncpy(Dest, strDest.data(), DestSize);

		return strDest.size() + 1;
	}
	else
	{
		if (Dest && DestSize)
			*Dest = 0;

		return 1;
	}
}

size_t WINAPI apiGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest, size_t DestSize)
{
	if (Src && *Src)
	{
		string strSrc(Src);
		string strDest;
		AddEndSlash(strDest);
		GetReparsePointInfo(strSrc,strDest,nullptr);

		if (DestSize && Dest)
			xwcsncpy(Dest,strDest.data(),DestSize);

		return strDest.size()+1;
	}
	else
	{
		if (DestSize && Dest)
			*Dest = 0;
		return 1;
	}
}

size_t WINAPI apiGetNumberOfLinks(const wchar_t* Name)
{
	string strName(Name);
	return GetNumberOfLinks(strName);
}

size_t WINAPI apiGetPathRoot(const wchar_t *Path, wchar_t *Root, size_t DestSize)
{
	if (Path && *Path)
	{
		string strPath(Path), strRoot;
		GetPathRoot(strPath,strRoot);

		if (DestSize && Root)
			xwcsncpy(Root,strRoot.data(),DestSize);

		return strRoot.size()+1;
	}
	else
	{
		if (DestSize && Root)
			*Root = 0;

		return 1;
	}
}

BOOL WINAPI apiCopyToClipboard(enum FARCLIPBOARD_TYPE Type, const wchar_t *Data)
{
	switch(Type)
	{
		case FCT_STREAM:
			return SetClipboard(Data);
		case FCT_COLUMN:
			return SetClipboardFormat(FCF_VERTICALBLOCK_UNICODE, Data);
		default:
			break;
	}
	return FALSE;
}

static size_t apiPasteFromClipboardEx(bool Type, wchar_t *Data, size_t Size)
{
	string str;
	if(Type? GetClipboardFormat(FCF_VERTICALBLOCK_UNICODE, str) : GetClipboard(str))
	{
		if(Data && Size)
		{
			Size = std::min(Size, str.size() + 1);
			wmemcpy(Data, str.data(), Size);
		}
		return str.size() + 1;
	}
	return 0;
}

size_t WINAPI apiPasteFromClipboard(enum FARCLIPBOARD_TYPE Type, wchar_t *Data, size_t Length)
{
	size_t size=0;
	switch(Type)
	{
		case FCT_STREAM:
			{
				string str;
				if(GetClipboardFormat(FCF_VERTICALBLOCK_UNICODE, str))
				{
					break;
				}
			}
		case FCT_ANY:
			size=apiPasteFromClipboardEx(false,Data,Length);
			break;
		case FCT_COLUMN:
			size=apiPasteFromClipboardEx(true,Data,Length);
			break;
	}
	return size;
}

intptr_t WINAPI apiMacroControl(const GUID* PluginId, FAR_MACRO_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{
	if (Global->CtrlObject) // ��� ������� �� ���� ������.
	{
		KeyMacro& Macro=Global->CtrlObject->Macro; //??

		switch (Command)
		{
			// Param1=0, Param2 - 0
			case MCTL_LOADALL: // �� ������� � ������ ��� � ���������� �����������
			{
				return !Macro.IsRecording() && Macro.Load(!Macro.IsExecuting(), !Global->Opt->OnlyEditorViewerUsed);
			}

			// Param1=0, Param2 - 0
			case MCTL_SAVEALL:
			{
				return !Macro.IsRecording() && Macro.Save(true);
			}

			// Param1=FARMACROSENDSTRINGCOMMAND, Param2 - MacroSendMacroText*
			case MCTL_SENDSTRING:
			{
				MacroSendMacroText *Data=(MacroSendMacroText*)Param2;
				if (CheckStructSize(Data) && Data->SequenceText)
				{
					if (Param1==MSSC_POST)
					{
						UINT64 Flags = MFLAGS_POSTFROMPLUGIN;
						if (Data->Flags & KMFLAGS_ENABLEOUTPUT)        Flags |= MFLAGS_ENABLEOUTPUT;
						if (Data->Flags & KMFLAGS_NOSENDKEYSTOPLUGINS) Flags |= MFLAGS_NOSENDKEYSTOPLUGINS;

						return Macro.PostNewMacro(Data->SequenceText,Flags,InputRecordToKey(&Data->AKey));
					}
					else if (Param1==MSSC_CHECK)
					{
						return Macro.ParseMacroString(Data->SequenceText,(Data->Flags&KMFLAGS_SILENTCHECK)!=0,false);
					}
				}
				break;
			}

			// Param1=0, Param2 - MacroExecuteString*
			case MCTL_EXECSTRING:
			{
				MacroExecuteString *Data=(MacroExecuteString*)Param2;
				return CheckStructSize(Data) && Macro.ExecuteString(Data) ? 1:0;
			}

			// Param1=0, Param2 - 0
			case MCTL_GETSTATE:
			{
				return Macro.GetCurRecord();
			}

			// Param1=0, Param2 - 0
			case MCTL_GETAREA:
			{
				return Macro.GetMode();
			}

			case MCTL_ADDMACRO:
			{
				if (!Param2)
					break;

				MacroAddMacro *Data=(MacroAddMacro*)Param2;
				MACROFLAGS_MFLAGS Flags = 0;

				if (Data->Flags & KMFLAGS_ENABLEOUTPUT)
					Flags |= MFLAGS_ENABLEOUTPUT;

				if (Data->Flags & KMFLAGS_NOSENDKEYSTOPLUGINS)
					Flags |= MFLAGS_NOSENDKEYSTOPLUGINS;

				if (CheckStructSize(Data) && Data->SequenceText && *Data->SequenceText)
				{
					if (Data->Area == MACROAREA_COMMON)
						Data->Area=MACROAREA_COMMON_INTERNAL;

					return Macro.AddMacro(Data->SequenceText,Data->Description,Data->Area,Flags,Data->AKey,*PluginId,Data->Id,Data->Callback);
				}
				break;
			}

			case MCTL_DELMACRO:
			{
				return Macro.DelMacro(*PluginId,Param2);
			}

			//Param1=size of buffer, Param2 - MacroParseResult*
			case MCTL_GETLASTERROR:
			{
				DWORD ErrCode=MPEC_SUCCESS;
				COORD ErrPos={};
				string ErrSrc;

				Macro.GetMacroParseError(&ErrCode,&ErrPos,&ErrSrc);

				int Size = ALIGN(sizeof(MacroParseResult));
				size_t stringOffset = Size;
				Size += static_cast<int>((ErrSrc.size() + 1)*sizeof(wchar_t));

				MacroParseResult *Result = (MacroParseResult *)Param2;

				if (Param1 >= Size && CheckStructSize(Result))
				{
					Result->StructSize = sizeof(MacroParseResult);
					Result->ErrCode = ErrCode;
					Result->ErrPos = ErrPos;
					Result->ErrSrc = (const wchar_t *)((char*)Param2+stringOffset);
					wmemcpy(const_cast<wchar_t*>(Result->ErrSrc), ErrSrc.data(), ErrSrc.size()+1);
				}

				return Size;
			}

			default: //FIXME
				break;
		}
	}


	return 0;
}

intptr_t WINAPI apiPluginsControl(HANDLE Handle, FAR_PLUGINS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{
	switch (Command)
	{
		case PCTL_LOADPLUGIN:
		case PCTL_FORCEDLOADPLUGIN:
			if (Param1 == PLT_PATH)
			{
				if (Param2)
				{
					string strPath;
					ConvertNameToFull(reinterpret_cast<const wchar_t*>(Param2), strPath);
					return reinterpret_cast<intptr_t>(Global->CtrlObject->Plugins->LoadPluginExternal(strPath, Command == PCTL_FORCEDLOADPLUGIN));
				}
			}
			break;

		case PCTL_FINDPLUGIN:
		{
			Plugin* plugin = nullptr;
			switch(Param1)
			{
				case PFM_GUID:
					plugin = Global->CtrlObject->Plugins->FindPlugin(*reinterpret_cast<GUID*>(Param2));
					break;

				case PFM_MODULENAME:
				{
					string strPath;
					ConvertNameToFull(reinterpret_cast<const wchar_t*>(Param2), strPath);
					auto ItemIterator = std::find_if(CONST_RANGE(*Global->CtrlObject->Plugins, i)
					{
						return !StrCmpI(i->GetModuleName(), strPath);
					});
					if (ItemIterator != Global->CtrlObject->Plugins->cend())
					{
						plugin = *ItemIterator;
					}
					break;
				}
			}
			if(plugin&&Global->CtrlObject->Plugins->IsPluginUnloaded(plugin)) plugin=nullptr;
			return reinterpret_cast<intptr_t>(plugin);
		}

		case PCTL_UNLOADPLUGIN:
			{
				return Global->CtrlObject->Plugins->UnloadPluginExternal(static_cast<Plugin*>(Handle));
			}
			break;

		case PCTL_GETPLUGININFORMATION:
			{
				FarGetPluginInformation* Info = reinterpret_cast<FarGetPluginInformation*>(Param2);
				if (!Info || (CheckStructSize(Info) && static_cast<size_t>(Param1) > sizeof(FarGetPluginInformation)))
				{
					Plugin* plugin = reinterpret_cast<Plugin*>(Handle);
					if(plugin)
					{
						return Global->CtrlObject->Plugins->GetPluginInformation(plugin, Info, Param1);
					}
				}
			}
			break;

		case PCTL_GETPLUGINS:
			{
				size_t PluginsCount = Global->CtrlObject->Plugins->GetPluginsCount();
				if(Param1 && Param2)
				{
					HANDLE* Plugins = static_cast<HANDLE*>(Param2);
					size_t Count = std::min(static_cast<size_t>(Param1), PluginsCount);
					size_t index = 0;
					FOR(const auto& i, *Global->CtrlObject->Plugins)
					{
						Plugins[index++] = i;
						if(index == Count)
							break;
					}
				}
				return PluginsCount;
			}
			break;
	}

	return 0;
}

intptr_t WINAPI apiFileFilterControl(HANDLE hHandle, FAR_FILE_FILTER_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{
	FileFilter *Filter=nullptr;

	if (Command != FFCTL_CREATEFILEFILTER)
	{
		if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		Filter = (FileFilter *)hHandle;
	}

	switch (Command)
	{
		case FFCTL_CREATEFILEFILTER:
		{
			if (!Param2)
				break;

			*((HANDLE *)Param2) = INVALID_HANDLE_VALUE;

			if (hHandle != nullptr && hHandle != PANEL_ACTIVE && hHandle != PANEL_PASSIVE && hHandle != PANEL_NONE)
				break;

			switch (Param1)
			{
				case FFT_PANEL:
				case FFT_FINDFILE:
				case FFT_COPY:
				case FFT_SELECT:
				case FFT_CUSTOM:
					break;
				default:
					return FALSE;
			}

			Filter = new FileFilter((Panel *)hHandle, (FAR_FILE_FILTER_TYPE)Param1);
			*((HANDLE *)Param2) = (HANDLE)Filter;
			return TRUE;

			break;
		}
		case FFCTL_FREEFILEFILTER:
		{
			delete Filter;
			return TRUE;
		}
		case FFCTL_OPENFILTERSMENU:
		{
			return Filter->FilterEdit();
		}
		case FFCTL_STARTINGTOFILTER:
		{
			Filter->UpdateCurrentTime();
			return TRUE;
		}
		case FFCTL_ISFILEINFILTER:
		{
			if (!Param2)
				break;
			return Filter->FileInFilter(*reinterpret_cast<const PluginPanelItem*>(Param2));
		}
	}

	return FALSE;
}

intptr_t WINAPI apiRegExpControl(HANDLE hHandle, FAR_REGEXP_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{
	RegExp* re=nullptr;

	if (Command != RECTL_CREATE)
	{
		if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		re = (RegExp*)hHandle;
	}

	switch (Command)
	{
		case RECTL_CREATE:

			if (!Param2)
				break;

			*((HANDLE*)Param2) = INVALID_HANDLE_VALUE;
			re = new RegExp;

			*((HANDLE*)Param2) = (HANDLE)re;
			return TRUE;

			break;
		case RECTL_FREE:
			delete re;
			return TRUE;
		case RECTL_COMPILE:
			return re->Compile((const wchar_t*)Param2,OP_PERLSTYLE);
		case RECTL_OPTIMIZE:
			return re->Optimize();
		case RECTL_MATCHEX:
		{
			RegExpSearch* data=(RegExpSearch*)Param2;
			return re->MatchEx(data->Text,data->Text+data->Position,data->Text+data->Length,data->Match,data->Count
#ifdef NAMEDBRACKETS
			                   ,data->Reserved
#endif
			                  );
		}
		case RECTL_SEARCHEX:
		{
			RegExpSearch* data=(RegExpSearch*)Param2;
			return re->SearchEx(data->Text,data->Text+data->Position,data->Text+data->Length,data->Match,data->Count
#ifdef NAMEDBRACKETS
			                    ,data->Reserved
#endif
			                   );
		}
		case RECTL_BRACKETSCOUNT:
			return re->GetBracketsCount();
	}

	return FALSE;
}

intptr_t WINAPI apiSettingsControl(HANDLE hHandle, FAR_SETTINGS_CONTROL_COMMANDS Command, intptr_t Param1, void* Param2)
{

	AbstractSettings* settings=nullptr;

	if (Command != SCTL_CREATE)
	{
		if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
			return FALSE;

		settings = (AbstractSettings*)hHandle;
	}

	switch (Command)
	{
		case SCTL_CREATE:

			if (!Param2)
				break;

			{
				FarSettingsCreate* data = (FarSettingsCreate*)Param2;
				if (CheckStructSize(data))
				{
					if (data->Guid == FarGuid)
					{
						settings = new FarSettings();
					}
					else
					{
						Plugin* plugin = Global->CtrlObject->Plugins->FindPlugin(data->Guid);
						if (plugin)
						{
							settings = new PluginSettings(data->Guid, Param1 == PSL_LOCAL);
						}
					}
					if (settings && settings->IsValid())
					{
						data->Handle=settings;
						return TRUE;
					}
					delete settings;
				}
			}
			break;
		case SCTL_FREE:
			{
				delete settings;
			}
			return TRUE;
		case SCTL_SET:
			return CheckStructSize((const FarSettingsItem*)Param2)?settings->Set(*(const FarSettingsItem*)Param2):FALSE;
		case SCTL_GET:
			return CheckStructSize((const FarSettingsItem*)Param2)?settings->Get(*(FarSettingsItem*)Param2):FALSE;
		case SCTL_ENUM:
			return CheckStructSize((FarSettingsEnum*)Param2)?settings->Enum(*(FarSettingsEnum*)Param2):FALSE;
		case SCTL_DELETE:
			return CheckStructSize((const FarSettingsValue*)Param2)?settings->Delete(*(const FarSettingsValue*)Param2):FALSE;
		case SCTL_CREATESUBKEY:
		case SCTL_OPENSUBKEY:
			return CheckStructSize((const FarSettingsValue*)Param2)?settings->SubKey(*(const FarSettingsValue*)Param2, Command==SCTL_CREATESUBKEY):0;
	}

	return FALSE;
}

size_t WINAPI apiGetCurrentDirectory(size_t Size,wchar_t* Buffer)
{
	string strCurDir;
	api::GetCurrentDirectory(strCurDir);

	if (Buffer && Size)
	{
		xwcsncpy(Buffer,strCurDir.data(),Size);
	}

	return strCurDir.size()+1;
}

size_t WINAPI apiFormatFileSize(unsigned __int64 Size, intptr_t Width, FARFORMATFILESIZEFLAGS Flags, wchar_t *Dest, size_t DestSize)
{
	static const simple_pair<unsigned __int64, unsigned __int64> FlagsPair[] =
	{
		{FFFS_COMMAS,         COLUMN_COMMAS},         // ��������� ����������� ����� ��������
		{FFFS_THOUSAND,       COLUMN_THOUSAND},       // ������ �������� 1024 ������������ �������� 1000
		{FFFS_FLOATSIZE,      COLUMN_FLOATSIZE},      // ���������� ������ ����� � ����� Windows Explorer (�.�. 999 ���� ����� �������� ��� 999, � 1000 ���� ��� 0.97 K)
		{FFFS_ECONOMIC,       COLUMN_ECONOMIC},       // ����������� �����, �� ���������� ������ ����� ��������� ������� ����� (�.�. 0.97K)
		{FFFS_MINSIZEINDEX,   COLUMN_MINSIZEINDEX},   // ���������� ���������� ������ ��� ��������������
		{FFFS_SHOWBYTESINDEX, COLUMN_SHOWBYTESINDEX}, // ���������� �������� B,K,M,G,T,P,E
	};

	unsigned __int64 FinalFlags=Flags & COLUMN_MINSIZEINDEX_MASK;
	std::for_each(CONST_RANGE(FlagsPair, i)
	{
		if (Flags & i.first)
			FinalFlags |= i.second;
	});

	string strDestStr;
	FileSizeToStr(strDestStr,Size,Width,FinalFlags);

	if (Dest && DestSize)
	{
		xwcsncpy(Dest,strDestStr.data(),DestSize);
	}

	return strDestStr.size()+1;
}

/* $ 30.07.2001 IS
     1. ��������� ������������ ����������.
     2. ������ ��������� ��������� �� ������� �� ����� ������
     3. ����� ����� ���� ������������ ���������� ���� (�� ��������,
        ������������� � ��.). ����� ���� ��������� ����� ������, �����������
        �������� ��� ������ � �������, ����� ��������� ����� ����������,
        ����� ��������� ����� � �������. ������, ��� ��� � ������ ���� :-)
*/

void WINAPI apiRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,FRSUSERFUNC Func,unsigned __int64 Flags,void *Param)
{
	if (Func && InitDir && *InitDir && Mask && *Mask)
	{
		filemasks FMask;

		if (!FMask.Set(Mask, FMF_SILENT)) return;

		Flags=Flags&0x000000FF; // ������ ������� ����!
		ScanTree ScTree((Flags & FRS_RETUPDIR)!=0, (Flags & FRS_RECUR)!=0, (Flags & FRS_SCANSYMLINK)!=0);
		api::FAR_FIND_DATA FindData;
		string strFullName;
		ScTree.SetFindPath(InitDir,L"*");

		bool Found = false;
		while (!Found && ScTree.GetNextName(&FindData,strFullName))
		{
			if (FMask.Compare(FindData.strFileName))
			{
				PluginPanelItem fdata = {};
				FindDataExToPluginPanelItem(&FindData, &fdata);

				Found = !Func(&fdata,strFullName.data(),Param);
				FreePluginPanelItem(fdata);
			}
		}
	}
}

/* $ 14.09.2000 SVS
 + ������� FarMkTemp - ��������� ����� ���������� ����� � ������ �����.
    Dest - �������� ����������
    Template - ������ �� �������� ������� mktemp, �������� "FarTmpXXXXXX"
    ������ ��������� ������ ���������.
*/
size_t WINAPI apiMkTemp(wchar_t *Dest, size_t DestSize, const wchar_t *Prefix)
{
	string strDest;
	if (FarMkTempEx(strDest, Prefix, TRUE) && Dest && DestSize)
	{
		xwcsncpy(Dest, strDest.data(), DestSize);
	}
	return strDest.size()+1;
}

size_t WINAPI apiProcessName(const wchar_t *param1, wchar_t *param2, size_t size, PROCESSNAME_FLAGS flags)
{
	//             0xFFFF - length
	//           0xFF0000 - mode
	// 0xFFFFFFFFFF000000 - flags

	PROCESSNAME_FLAGS Flags = flags&0xFFFFFFFFFF000000;
	PROCESSNAME_FLAGS Mode = flags&0xFF0000;
	int Length = flags&0xFFFF;

	switch(Mode)
	{
	case PN_CMPNAME:
		{
			return CmpName(param1, param2, (Flags&PN_SKIPPATH)!=0);
		}

	case PN_CMPNAMELIST:
	case PN_CHECKMASK:
		{
			static filemasks Masks;
			static string PrevMask;
			static bool ValidMask = false;
			if(PrevMask != param1)
			{
				ValidMask = Masks.Set(param1, FMF_SILENT);
				PrevMask = param1;
			}
			BOOL Result = FALSE;
			if(ValidMask)
			{
				Result = (Mode == PN_CHECKMASK)? TRUE : Masks.Compare((Flags&PN_SKIPPATH)? PointToName(param2) : param2);
			}
			else
			{
				if(Flags&PN_SHOWERRORMESSAGE)
				{
					Masks.ErrorMessage();
				}
			}
			return Result;
		}

	case PN_GENERATENAME:
		{
			string strResult = NullToEmpty(param2);
			int nResult = ConvertWildcards(NullToEmpty(param1), strResult, Length);
			xwcsncpy(param2, strResult.data(), size);
			return nResult;
		}
	}
	return FALSE;
}

BOOL WINAPI apiColorDialog(const GUID* PluginId, COLORDIALOGFLAGS Flags, struct FarColor *Color)
{
	BOOL Result = FALSE;
	if (!Global->FrameManager->ManagerIsDown())
	{
		Result = Global->Console->GetColorDialog(*Color, true, false);
	}
	return Result;
}

size_t WINAPI apiInputRecordToKeyName(const INPUT_RECORD* Key, wchar_t *KeyText, size_t Size)
{
	int iKey = InputRecordToKey(Key);
	string strKT;
	if (!KeyToText(iKey,strKT))
		return 0;
	size_t len = strKT.size();
	if (Size && KeyText)
	{
		if (Size <= len)
			len = Size-1;
		wmemcpy(KeyText, strKT.data(), len);
		KeyText[len] = 0;
	}
	else if (KeyText)
		*KeyText = 0;
	return len+1;
}

BOOL WINAPI apiKeyNameToInputRecord(const wchar_t *Name,INPUT_RECORD* RecKey)
{
	int Key=KeyNameToKey(Name);
	return Key > 0? KeyToInputRecord(Key,RecKey) != 0 : FALSE;
}

BOOL WINAPI apiMkLink(const wchar_t *Src,const wchar_t *Dest, LINK_TYPE Type, MKLINK_FLAGS Flags)
{
	int Result=0;

	if (Src && *Src && Dest && *Dest)
	{
		switch (Type)
		{
		case LINK_HARDLINK:
			Result=MkHardLink(Src, Dest, (Flags&MLF_SHOWERRMSG) == 0);
			break;
		case LINK_JUNCTION:
		case LINK_VOLMOUNT:
		case LINK_SYMLINKFILE:
		case LINK_SYMLINKDIR:
			{
				ReparsePointTypes LinkType=RP_JUNCTION;

				switch (Type)
				{
				case LINK_VOLMOUNT:
					LinkType=RP_VOLMOUNT;
					break;
				case LINK_SYMLINK:
					LinkType=RP_SYMLINK;
					break;
				case LINK_SYMLINKFILE:
					LinkType=RP_SYMLINKFILE;
					break;
				case LINK_SYMLINKDIR:
					LinkType=RP_SYMLINKDIR;
					break;
				default:
					break;
				}

				Result=MkSymLink(Src,Dest, LinkType, (Flags&MLF_SHOWERRMSG) == 0);
			}
			break;
		default:
			break;
		}
	}

	if (Result && !(Flags&MLF_DONOTUPDATEPANEL))
		ShellUpdatePanels(nullptr,FALSE);

	return Result;
}

BOOL WINAPI apiAddEndSlash(wchar_t *Path)
{
	return AddEndSlash(Path) ? TRUE : FALSE;
}

wchar_t* WINAPI apiXlat(wchar_t *Line,intptr_t StartPos,intptr_t EndPos,XLAT_FLAGS Flags)
{
	return Xlat(Line, StartPos, EndPos, Flags);
}

HANDLE WINAPI apiCreateFile(const wchar_t *Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
{
	return api::CreateFile(Object,DesiredAccess,ShareMode,SecurityAttributes,CreationDistribution,FlagsAndAttributes,TemplateFile);
}

DWORD WINAPI apiGetFileAttributes(const wchar_t *FileName)
{
	return api::GetFileAttributes(FileName);
}

BOOL WINAPI apiSetFileAttributes(const wchar_t *FileName,DWORD dwFileAttributes)
{
	return api::SetFileAttributes(FileName,dwFileAttributes);
}

BOOL WINAPI apiMoveFileEx(const wchar_t *ExistingFileName,const wchar_t *NewFileName,DWORD dwFlags)
{
	return api::MoveFileEx(ExistingFileName,NewFileName,dwFlags);
}

BOOL WINAPI apiDeleteFile(const wchar_t *FileName)
{
	return api::DeleteFile(FileName);
}

BOOL WINAPI apiRemoveDirectory(const wchar_t *DirName)
{
	return api::RemoveDirectory(DirName);
}

BOOL WINAPI apiCreateDirectory(const wchar_t *PathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return api::CreateDirectory(PathName,lpSecurityAttributes);
}

intptr_t WINAPI apiCallFar(intptr_t CheckCode, FarMacroCall* Data)
{
	if (Global->CtrlObject)
	{
		KeyMacro& Macro=Global->CtrlObject->Macro;
		return Macro.CallFar(CheckCode, Data);
	}
	return 0;
}

void WINAPI apiCallPlugin(MacroPluginReturn* Data, FarMacroCall** Target, int *Boolean)
{
	if (Global->CtrlObject)
		Global->CtrlObject->Macro.CallPluginSynchro(Data, Target, Boolean);
}

namespace cfunctions
{
	void* bsearchex(const void* key,const void* base,size_t nelem,size_t width,int (WINAPI *fcmp)(const void*, const void*,void*),void* userparam)
	{
		if(width)
		{
			size_t low=0,high=nelem;
			while(low<high)
			{
				size_t curr=(low+high)/2;
				const void* ptr = (((const char*)base)+curr*width);
				int cmp=fcmp(key,ptr,userparam);
				if(0==cmp)
				{
					return const_cast<void*>(ptr);
				}
				else if(cmp<0)
				{
					high=curr;
				}
				else
				{
					low=curr+1;
				}
			}
		}
		return nullptr;
	}

	/* start qsortex */

	/*
	Copyright Prototronics, 1987
	Totem Lake P.O. 8117
	Kirkland, Washington 98034

	(206) 820-1972

	Licensed to Zortech. */
	/*
	Modified by Joe Huffman (d.b.a Prototronics) June 11, 1987 from Ray Gardner's,
	(Denver, Colorado) public domain version. */

	/*    qsortex()  --  Quicksort function
	**
	**    Usage:   qsortex(base, nbr_elements, width_bytes, compare_function);
	**                char *base;
	**                unsigned int nbr_elements, width_bytes;
	**                int (*compare_function)();
	**
	**    Sorts an array starting at base, of length nbr_elements, each
	**    element of size width_bytes, ordered via compare_function; which
	**    is called as  (*compare_function)(ptr_to_element1, ptr_to_element2)
	**    and returns < 0 if element1 < element2, 0 if element1 = element2,
	**    > 0 if element1 > element2.  Most of the refinements are due to
	**    R. Sedgewick.  See "Implementing Quicksort Programs", Comm. ACM,
	**    Oct. 1978, and Corrigendum, Comm. ACM, June 1979.
	*/

	static void iswap(int *a, int *b, size_t n_to_swap);       /* swap ints */
	static void cswap(char *a, char *b, size_t n_to_swap);     /* swap chars */

	//static unsigned int n_to_swap;  /* nbr of chars or ints to swap */
	int _maxspan = 7;               /* subfiles of _maxspan or fewer elements */
	/* will be sorted by a simple insertion sort */

	/* Adjust _maxspan according to relative cost of a swap and a compare.  Reduce
	_maxspan (not less than 1) if a swap is very expensive such as when you have
	an array of large structures to be sorted, rather than an array of pointers to
	structures.  The default value is optimized for a high cost for compares. */

#define SWAP(a,b) (*swap_fp)(a,b,n_to_swap)
#define COMPEX(a,b,u) (*comp_fp)(a,b,u)
#define COMP(a,b) (*comp_fp)(a,b)

	typedef void (__cdecl *SWAP_FP)(void *, void *, size_t);

	void __cdecl qsortex(char *base, size_t nel, size_t width,
		int (WINAPI *comp_fp)(const void *, const void *,void*), void *user)
	{
		char *stack[40], **sp;                 /* stack and stack pointer        */
		char *i, *j, *limit;                   /* scan and limit pointers        */
		size_t thresh;                         /* size of _maxspan elements in   */
		void (__cdecl  *swap_fp)(void *, void *, size_t);               /* bytes */
		size_t n_to_swap;

		if ((width % sizeof(int)) )
		{
			swap_fp = (SWAP_FP)cswap;
			n_to_swap = width;
		}
		else
		{
			swap_fp = (SWAP_FP)iswap;
			n_to_swap = width / sizeof(int);
		}

		thresh = _maxspan * width;             /* init threshold                 */
		sp = stack;                            /* init stack pointer             */
		limit = base + nel * width;            /* pointer past end of array      */

		for (;;)                               /* repeat until done then return  */
		{
			while ((size_t)(limit - base) > thresh) /* if more than _maxspan elements */
			{
				/*swap middle, base*/
				SWAP(((size_t)(limit - base) >> 1) -
					((((size_t)(limit - base) >> 1)) % width) + base, base);
				i = base + width;                /* i scans from left to right     */
				j = limit - width;               /* j scans from right to left     */

				if (COMPEX(i, j,user) > 0)              /* Sedgewick's                    */
					SWAP(i, j);                    /*    three-element sort          */

				if (COMPEX(base, j,user) > 0)           /*        sets things up          */
					SWAP(base, j);                 /*            so that             */

				if (COMPEX(i, base,user) > 0)           /*              *i <= *base <= *j */
					SWAP(i, base);                 /* *base is the pivot element     */

				for (;;)
				{
					do                            /* move i right until *i >= pivot */
					i += width;

					while (COMPEX(i, base,user) < 0);

					do                            /* move j left until *j <= pivot  */
					j -= width;

					while (COMPEX(j, base,user) > 0);

					if (i > j)                    /* break loop if pointers crossed */
						break;

					SWAP(i, j);                   /* else swap elements, keep scanning */
				}

				SWAP(base, j);                   /* move pivot into correct place  */

				if (j - base > limit - i)        /* if left subfile is larger...   */
				{
					sp[0] = base;                 /* stack left subfile base        */
					sp[1] = j;                    /*    and limit                   */
					base = i;                     /* sort the right subfile         */
				}
				else                             /* else right subfile is larger   */
				{
					sp[0] = i;                    /* stack right subfile base       */
					sp[1] = limit;                /*    and limit                   */
					limit = j;                    /* sort the left subfile          */
				}

				sp += 2;                        /* increment stack pointer        */
			}

			/* Insertion sort on remaining subfile. */
			i = base + width;

			while (i < limit)
			{
				j = i;

				while (j > base && COMPEX(j - width, j,user) > 0)
				{
					SWAP(j - width, j);
					j -= width;
				}

				i += width;
			}

			if (sp > stack)    /* if any entries on stack...     */
			{
				sp -= 2;         /* pop the base and limit         */
				base = sp[0];
				limit = sp[1];
			}
			else              /* else stack empty, all done     */
				break;          /* Return. */
		}
	}

	static void iswap(int *a, int *b, size_t n_to_swap)   /* swap ints */
	{
		do
		{
			int tmp = *a;
			*a = *b;
			*b = tmp;
			a++; b++;
		}
		while (--n_to_swap);
	}

	static void cswap(char *a, char *b, size_t n_to_swap)  /* swap chars */
	{
		do
		{
			char tmp = *a;
			*a = *b;
			*b = tmp;
			a++; b++;
		}
		while (--n_to_swap);
	}

	/* end qsortex */
};

};

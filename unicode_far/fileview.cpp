/*
fileview.cpp

�������� ����� - ���������� ��� viewer.cpp
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

#include "fileview.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "history.hpp"
#include "manager.hpp"
#include "fileedit.hpp"
#include "cmdline.hpp"
#include "savescr.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "stddlg.hpp"
#include "macroopcode.hpp"
#include "plugins.hpp"
#include "language.hpp"
#include "exitcode.hpp"

FileViewer::FileViewer(const string& Name,int EnableSwitch,int DisableHistory, int DisableEdit,
                       __int64 ViewStartPos,const wchar_t *PluginData, NamesList *ViewNamesList,bool ToSaveAs,
                       uintptr_t aCodePage, const wchar_t *Title, int DeleteOnClose):
	View(false,aCodePage),
	FullScreen(true),
	DisableEdit(DisableEdit),
	delete_on_close(0),
	str_title(NullToEmpty(Title))
{
	_OT(SysLog(L"[%p] FileViewer::FileViewer(I variant...)", this));
	if (!str_title.empty())
		View.SetTitle(Title);
	if (DeleteOnClose)
	{
		delete_on_close = DeleteOnClose == 1 ? 1 : 2;
		SetTempViewName(Name, DeleteOnClose == 1);
	}
	SetPosition(0,0,ScrX,ScrY);
	Init(Name,EnableSwitch,DisableHistory,ViewStartPos,PluginData,ViewNamesList,ToSaveAs);
}


FileViewer::FileViewer(const string& Name,int EnableSwitch,int DisableHistory,
                       const wchar_t *Title, int X1,int Y1,int X2,int Y2,uintptr_t aCodePage):
	View(false,aCodePage),
	DisableEdit(TRUE),
	delete_on_close(0),
	str_title(NullToEmpty(Title))
{
	_OT(SysLog(L"[%p] FileViewer::FileViewer(II variant...)", this));

	if (X1 < 0)
		X1=0;

	if (X2 < 0 || X2 > ScrX)
		X2=ScrX;

	if (Y1 < 0)
		Y1=0;

	if (Y2 < 0 || Y2 > ScrY)
		Y2=ScrY;

	if (X1 >= X2)
	{
		X1=0;
		X2=ScrX;
	}

	if (Y1 >= Y2)
	{
		Y1=0;
		Y2=ScrY;
	}

	SetPosition(X1,Y1,X2,Y2);
	FullScreen=(!X1 && !Y1 && X2==ScrX && Y2==ScrY);
	View.SetTitle(Title);
	Init(Name, EnableSwitch, DisableHistory, -1, L"", nullptr, false);
}


void FileViewer::Init(const string& name,int EnableSwitch,int disableHistory,
	__int64 ViewStartPos,const wchar_t *PluginData,
	NamesList *ViewNamesList,bool ToSaveAs)
{
	RedrawTitle = FALSE;
	ViewKeyBar.SetOwner(this);
	ViewKeyBar.SetPosition(X1,Y2,X2,Y2);
	KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
	TitleBarVisible = Global->Opt->ViOpt.ShowTitleBar;
	FARMACROAREA OldMacroMode=Global->CtrlObject->Macro.GetMode();
	MacroMode = MACROAREA_VIEWER;
	Global->CtrlObject->Macro.SetMode(MACROAREA_VIEWER);
	View.SetPluginData(PluginData);
	View.SetHostFileViewer(this);
	DisableHistory=disableHistory; ///
	strName = name;
	SetCanLoseFocus(EnableSwitch);
	SaveToSaveAs=ToSaveAs;
	InitKeyBar();

	if (!View.OpenFile(strName,TRUE)) // $ 04.07.2000 tran + add TRUE as 'warning' parameter
	{
		DisableHistory = TRUE;  // $ 26.03.2002 DJ - ��� ������� �������� - �� ����� ����� � �������
		// FrameManager->DeleteFrame(this); // �����? ������ �� ��� �� ������� � ������� ��������!
		ExitCode=FALSE;
		Global->CtrlObject->Macro.SetMode(OldMacroMode);
		return;
	}

	if (ViewStartPos!=-1)
		View.SetFilePos(ViewStartPos);

	if (ViewNamesList)
		View.SetNamesList(*ViewNamesList);

	ExitCode=TRUE;
	ViewKeyBar.Show();

	if (!Global->Opt->ViOpt.ShowKeyBar)
		ViewKeyBar.Hide0();

	ShowConsoleTitle();
	F3KeyOnly=true;

	if (EnableSwitch)
	{
		Global->FrameManager->InsertFrame(this);
	}
	else
	{
		Global->FrameManager->ExecuteFrame(this);
	}
}


void FileViewer::InitKeyBar()
{
	ViewKeyBar.SetLabels(Global->Opt->OnlyEditorViewerUsed?MSingleViewF1:MViewF1);

	if (DisableEdit)
		ViewKeyBar.Change(KBL_MAIN,L"",6-1);

	if (!GetCanLoseFocus())
		ViewKeyBar.Change(KBL_MAIN,L"",12-1);

	if (!GetCanLoseFocus())
		ViewKeyBar.Change(KBL_ALT,L"",11-1);

	ViewKeyBar.SetCustomLabels(KBA_VIEWER);
	SetKeyBar(&ViewKeyBar);
	View.SetPosition(X1,Y1+(Global->Opt->ViOpt.ShowTitleBar?1:0),X2,Y2-(Global->Opt->ViOpt.ShowKeyBar?1:0));
	View.SetViewKeyBar(&ViewKeyBar);
}

void FileViewer::Show()
{
	if (FullScreen)
	{
		if (Global->Opt->ViOpt.ShowKeyBar)
		{
			ViewKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
			ViewKeyBar.Redraw();
		}

		SetPosition(0,0,ScrX,ScrY-(Global->Opt->ViOpt.ShowKeyBar?1:0));
		View.SetPosition(0,(Global->Opt->ViOpt.ShowTitleBar?1:0),ScrX,ScrY-(Global->Opt->ViOpt.ShowKeyBar?1:0));
	}

	ScreenObjectWithShadow::Show();
	ShowStatus();
}


void FileViewer::DisplayObject()
{
	View.Show();
}

__int64 FileViewer::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (OpCode == MCODE_F_KEYBAR_SHOW)
	{
		int PrevMode=Global->Opt->ViOpt.ShowKeyBar?2:1;
		switch (iParam)
		{
			case 0:
				break;
			case 1:
				Global->Opt->ViOpt.ShowKeyBar=1;
				ViewKeyBar.Show();
				Show();
				KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
				break;
			case 2:
				Global->Opt->ViOpt.ShowKeyBar=0;
				ViewKeyBar.Hide();
				Show();
				KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
				break;
			case 3:
				ProcessKey(KEY_CTRLB);
				break;
			default:
				PrevMode=0;
				break;
		}
		return PrevMode;
	}
	return View.VMProcess(OpCode,vParam,iParam);
}

int FileViewer::ProcessKey(int Key)
{
	if (RedrawTitle && (((unsigned int)Key & 0x00ffffff) < KEY_END_FKEY || IsInternalKeyReal((unsigned int)Key & 0x00ffffff)))
		ShowConsoleTitle();

	if (Key!=KEY_F3 && Key!=KEY_IDLE)
		F3KeyOnly=false;

	switch (Key)
	{
#if 0
			/* $ 30.05.2003 SVS
			   ���� :-) Shift-F4 � ���������/������� ��������� ��������� ������ ��������/������
			   ���� �����������
			*/
		case KEY_SHIFTF4:
		{
			if (!Global->Opt->OnlyEditorViewerUsed)
				Global->CtrlObject->Cp()->ActivePanel->ProcessKey(Key);

			return TRUE;
		}
#endif
		/* $ 22.07.2000 tran
		   + ����� �� ctrl-f10 � ���������� ������� �� ���� */
		case KEY_CTRLF10:
		case KEY_RCTRLF10:
		{
			if (View.isTemporary())
			{
				return TRUE;
			}

			SaveScreen Sc;
			string strFileName;
			View.GetFileName(strFileName);
			Global->CtrlObject->Cp()->GoToFile(strFileName);
			RedrawTitle = TRUE;
			return TRUE;
		}
		// $ 15.07.2000 tran + CtrlB switch KeyBar
		case KEY_CTRLB:
		case KEY_RCTRLB:
			Global->Opt->ViOpt.ShowKeyBar=!Global->Opt->ViOpt.ShowKeyBar;

			if (Global->Opt->ViOpt.ShowKeyBar)
				ViewKeyBar.Show();
			else
				ViewKeyBar.Hide0(); // 0 mean - Don't purge saved screen

			Show();
			KeyBarVisible = Global->Opt->ViOpt.ShowKeyBar;
			return TRUE;
		case KEY_CTRLSHIFTB:
		case KEY_RCTRLSHIFTB:
		{
			Global->Opt->ViOpt.ShowTitleBar=!Global->Opt->ViOpt.ShowTitleBar;
			TitleBarVisible = Global->Opt->ViOpt.ShowTitleBar;
			Show();
			return TRUE;
		}
		case KEY_CTRLO:
		case KEY_RCTRLO:

			if (!Global->Opt->OnlyEditorViewerUsed)
			{
				if (Global->FrameManager->ShowBackground())
				{
					SetCursorType(false, 0);
					WaitKey();
					Global->FrameManager->RefreshFrame();
				}
			}

			return TRUE;
		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:

			if (F3KeyOnly)
				return TRUE;

		case KEY_ESC:
		case KEY_F10:
			Global->FrameManager->DeleteFrame();
			return TRUE;
		case KEY_F6:

			if (!DisableEdit)
			{
				UINT cp=View.VM.CodePage;
				string strViewFileName;
				View.GetFileName(strViewFileName);
				api::File Edit;
				while(!Edit.Open(strViewFileName, FILE_READ_DATA, FILE_SHARE_READ|(Global->Opt->EdOpt.EditOpenedForWrite?FILE_SHARE_WRITE:0), nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN))
				{
					Global->CatchError();
					if(!OperationFailed(strViewFileName, MEditTitle, MSG(MEditCannotOpen), false))
						continue;
					else
						return TRUE;
				}
				Edit.Close();
				__int64 FilePos=View.GetFilePos();
				DWORD flags = (GetCanLoseFocus()?FFILEEDIT_ENABLEF6:0)|(SaveToSaveAs?FFILEEDIT_SAVETOSAVEAS:0)|(DisableHistory?FFILEEDIT_DISABLEHISTORY:0);
				FileEditor *ShellEditor = new FileEditor(
					strViewFileName, cp, flags, -2,
					static_cast<int>(FilePos), // TODO: Editor StartChar should be __int64
					str_title.empty() ? nullptr: &str_title,
					-1,-1, -1, -1, delete_on_close );

				int load = ShellEditor->GetExitCode();
				if (load == XC_LOADING_INTERRUPTED || load == XC_OPEN_ERROR)
				{
					delete ShellEditor;
				}
				else
				{
					ShellEditor->SetEnableF6(true);
					/* $ 07.05.2001 DJ ��������� NamesList */
					ShellEditor->SetNamesList(View.GetNamesList());

					// ���� ������������� � ��������, �� ������� ���� ��� �� �����
					SetTempViewName(L"");
					SetExitCode(0);

					Global->FrameManager->DeleteFrame(this); // Insert ��� ���� ������ ������������
				}
				ShowTime(2);
			}

			return TRUE;

		case KEY_ALTSHIFTF9:
		case KEY_RALTSHIFTF9:
			// ������ � ��������� ������ ViewerOptions
			Global->Opt->LocalViewerConfig(View.ViOpt);

			if (Global->Opt->ViOpt.ShowKeyBar)
				ViewKeyBar.Show();

			View.Show();
			return TRUE;
		case KEY_ALTF11:
		case KEY_RALTF11:
			if (GetCanLoseFocus())
				Global->CtrlObject->CmdLine->ShowViewEditHistory();

			return TRUE;
		default:
//      ���� ����� - �� ������� (�� �������� � ���������� :-)
//      if (Global->CtrlObject->Macro.IsExecuting() || !View.ProcessViewerInput(&ReadRec))
		{
			/* $ 22.03.2001 SVS
			   ��� ������� �� ��������� :-)
			*/
			if (!Global->CtrlObject->Macro.IsExecuting())
				if (Global->Opt->ViOpt.ShowKeyBar)
					ViewKeyBar.Show();

			if (!ViewKeyBar.ProcessKey(Key))
				return View.ProcessKey(Key);
		}
		return TRUE;
	}
}


int FileViewer::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	F3KeyOnly = false;
	if (!View.ProcessMouse(MouseEvent))
		if (!ViewKeyBar.ProcessMouse(MouseEvent))
			return FALSE;

	return TRUE;
}


int FileViewer::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MScreensView);
	View.GetFileName(strName);
	return MODALTYPE_VIEWER;
}


void FileViewer::ShowConsoleTitle()
{
	View.ShowConsoleTitle();
	RedrawTitle = FALSE;
}


void FileViewer::SetTempViewName(const string& Name, BOOL DeleteFolder)
{
	delete_on_close = (DeleteFolder ? 1 : 2);
	View.SetTempViewName(Name, DeleteFolder);
}


FileViewer::~FileViewer()
{
	_OT(SysLog(L"[%p] ~FileViewer::FileViewer()",this));
}

void FileViewer::OnDestroy()
{
	_OT(SysLog(L"[%p] FileViewer::OnDestroy()",this));

	if (!DisableHistory && (Global->CtrlObject->Cp()->ActivePanel || strName != L"-"))
	{
		string strFullFileName;
		View.GetFileName(strFullFileName);
		Global->CtrlObject->ViewHistory->AddToHistory(strFullFileName, HR_VIEWER);
	}
}

int FileViewer::FastHide()
{
	return Global->Opt->AllCtrlAltShiftRule & CASR_VIEWER;
}

int FileViewer::ViewerControl(int Command, intptr_t Param1, void *Param2)
{
	_VCTLLOG(CleverSysLog SL(L"FileViewer::ViewerControl()"));
	_VCTLLOG(SysLog(L"(Command=%s, Param2=[%d/0x%08X])",_VCTL_ToName(Command),(int)Param2,Param2));
	return View.ViewerControl(Command,Param1,Param2);
}

const string& FileViewer::GetTitle(string &Title)
{
	return View.GetTitle(Title);
}

__int64 FileViewer::GetViewFileSize() const
{
	return View.GetViewFileSize();
}

__int64 FileViewer::GetViewFilePos() const
{
	return View.GetViewFilePos();
}

void FileViewer::ShowStatus()
{
	if (!IsTitleBarVisible())
		return;

	string strName;
	GetTitle(strName);
	int NameLength = ScrX+1 - 40;

	if (Global->Opt->ViewerEditorClock && IsFullScreen())
		NameLength -= 3+5;

	NameLength = std::max(NameLength, 20);

	TruncPathStr(strName, NameLength);
	const wchar_t *lpwszStatusFormat = L"%-*s %c %5u %13I64u %7.7s %-4I64d %3d%%";
	string strStatus = str_printf(
	    lpwszStatusFormat,
	    NameLength,
	    strName.data(),
	    L"thd"[View.VM.Hex],
	    View.VM.CodePage,
	    View.FileSize,
	    MSG(MViewerStatusCol),
	    View.LeftPos,
	    (View.LastPage ? 100:ToPercent(View.FilePos,View.FileSize))
	);
	SetColor(COL_VIEWERSTATUS);
	GotoXY(X1,Y1);
	Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(View.Width+(View.ViOpt.ShowScrollbar?1:0))<<strStatus;

	if (Global->Opt->ViewerEditorClock && IsFullScreen())
		ShowTime(FALSE);
}

void FileViewer::OnChangeFocus(int focus)
{
	Frame::OnChangeFocus(focus);
	Global->CtrlObject->Plugins->SetCurViewer(&View);
	int FCurViewerID=View.ViewerID;
	Global->CtrlObject->Plugins->ProcessViewerEvent(focus?VE_GOTFOCUS:VE_KILLFOCUS,nullptr,FCurViewerID);
}

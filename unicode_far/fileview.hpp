#pragma once

/*
fileview.hpp

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

#include "frame.hpp"
#include "viewer.hpp"
#include "keybar.hpp"

class FileViewer:public Frame
{
	private:
		virtual void Show() override;
		virtual void DisplayObject() override;
		Viewer View;
		int RedrawTitle;
		KeyBar ViewKeyBar;
		bool F3KeyOnly;
		bool FullScreen;
		int DisableEdit;
		int DisableHistory;

		string strName;

		typedef class Frame inherited;
		/* $ 17.08.2001 KM
		  ��������� ��� ������ �� AltF7. ��� �������������� ���������� ����� ��
		  ������ ��� ������� F2 ������� ����� ShiftF2.
		*/
		bool SaveToSaveAs;

		int delete_on_close;
		string    str_title;

	public:
		FileViewer(
			const string& Name,int EnableSwitch=FALSE,int DisableHistory=FALSE,
			int DisableEdit=FALSE,__int64 ViewStartPos=-1,const wchar_t *PluginData=nullptr,
			NamesList *ViewNamesList=nullptr,bool ToSaveAs=false,uintptr_t aCodePage=CP_DEFAULT,
			const wchar_t *Title=nullptr, int DeleteOnClose=0);
		FileViewer(const string& Name,int EnableSwitch,int DisableHistory,
			const wchar_t *Title,int X1,int Y1,int X2,int Y2,uintptr_t aCodePage=CP_DEFAULT);
		virtual ~FileViewer();

	public:
		void Init(const string& Name,int EnableSwitch,int DisableHistory,
			__int64 ViewStartPos,const wchar_t *PluginData,NamesList *ViewNamesList,bool ToSaveAs);
		virtual void InitKeyBar() override;
		virtual int ProcessKey(int Key) override;
		virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
		virtual void ShowConsoleTitle() override;
		/* $ 14.06.2002 IS
		   �������� DeleteFolder - ������� �� ������ ����, �� � �������, ���
		   ���������� (���� ������� ����). �� ��������� - TRUE (��������
		   ��������� SetTempViewName ����� ��, ��� � ������)
		*/
		void SetTempViewName(const string& Name,BOOL DeleteFolder=TRUE);
		virtual void OnDestroy() override;
		virtual void OnChangeFocus(int focus) override;

		virtual int GetTypeAndName(string &strType, string &strName) override;
		virtual const wchar_t *GetTypeName() override {return L"[FileView]";}
		virtual int GetType() const override { return MODALTYPE_VIEWER; }

		void SetEnableF6(int AEnable) { DisableEdit = !AEnable; InitKeyBar(); }
		/* $ ������� ��� ���� CtrlAltShift OT */
		virtual int FastHide() override;

		/* $ 17.08.2001 KM
		  ��������� ��� ������ �� AltF7. ��� �������������� ���������� ����� ��
		  ������ ��� ������� F2 ������� ����� ShiftF2.
		*/
		void SetSaveToSaveAs(bool ToSaveAs) { SaveToSaveAs=ToSaveAs; InitKeyBar(); }
		int  ViewerControl(int Command, intptr_t Param1, void *Param2);
		bool IsFullScreen() const {return FullScreen;}
		virtual const string& GetTitle(string &Title) override;
		__int64 GetViewFileSize() const;
		__int64 GetViewFilePos() const;
		void ShowStatus();
		int GetId() const { return View.ViewerID; }
};

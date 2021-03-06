/*
	Copyright (C) 2011-2012 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "slot1_config.h"
#include <windowsx.h>
#include "resource.h"
#include "main.h"
#include "debug.h"
#include "../slot1.h"
#include "../NDSSystem.h"
#include <shlobj.h>

HWND wndConfigSlot1 = NULL;
NDS_SLOT1_TYPE temp_type_slot1 = NDS_SLOT1_NONE;
u8 last_type_slot1 = 0;
char tmp_fat_path[MAX_PATH] = {0};
HWND OKbutton_slot1 = NULL;
bool _OKbutton_slot1 = false;
bool needReset_slot1 = true;


INT_PTR CALLBACK Slot1None(HWND dialog, UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG: 
		{
			_OKbutton_slot1 = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

INT_PTR CALLBACK Slot1R4(HWND dialog, UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG: 
		{
			SetWindowText(GetDlgItem(dialog, IDC_PATH), tmp_fat_path);
			_OKbutton_slot1 = (tmp_fat_path!="");
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wparam))
			{
				case IDC_BROWSE:
				{
					BROWSEINFO bp={0};

					bp.hwndOwner=dialog;
					bp.pidlRoot=NULL;
					bp.pszDisplayName=NULL;
					bp.lpszTitle="Select directory for FAT image building";
					bp.ulFlags=BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_USENEWUI;
					bp.lpfn=NULL;
	
					LPITEMIDLIST tmp = SHBrowseForFolder((LPBROWSEINFO)&bp);
					if (tmp!=NULL) 
					{
						memset(tmp_fat_path, 0, sizeof(tmp_fat_path));
						SHGetPathFromIDList(tmp, tmp_fat_path);
						if (tmp_fat_path[strlen(tmp_fat_path)-1] != '\\')
							tmp_fat_path[strlen(tmp_fat_path)] = '\\';
						SetWindowText(GetDlgItem(dialog, IDC_PATH), tmp_fat_path);
					}
					if (strlen(tmp_fat_path))
							EnableWindow(OKbutton_slot1, TRUE);
						else
							EnableWindow(OKbutton_slot1, FALSE);
					break;
				}
			}
		}
	}
	return FALSE;
}

u32	Slot1_IDDs[NDS_SLOT1_COUNT] = {
	IDD_SLOT1_NONE,
	IDD_SLOT1_NONE,
	IDD_SLOT1_R4,
	IDD_SLOT1_NONE
};

DLGPROC Slot1_Procs[NDS_SLOT1_COUNT] = {
	Slot1None,
	Slot1None,
	Slot1R4,
	Slot1None
};


//==============================================================================
BOOL CALLBACK Slot1Box_Proc(HWND dialog, UINT msg,WPARAM wparam,LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG: 
		{
			OKbutton_slot1 = GetDlgItem(dialog, IDOK);
			for(int i = 0; i < NDS_SLOT1_COUNT; i++)
				ComboBox_AddString(GetDlgItem(dialog, IDC_ADDONS_LIST), slot1List[i].name);
			ComboBox_SetCurSel(GetDlgItem(dialog, IDC_ADDONS_LIST), temp_type_slot1);
			u8 tmp_info[512];
			slot1List[temp_type_slot1].info((char *)tmp_info);
			SetWindowText(GetDlgItem(dialog, IDC_ADDONS_INFO), (char *)tmp_info);

			_OKbutton_slot1 = false;
			wndConfigSlot1=CreateDialogW(hAppInst, MAKEINTRESOURCEW(Slot1_IDDs[temp_type_slot1]), 
										dialog, (DLGPROC)Slot1_Procs[temp_type_slot1]);
			if ( (temp_type_slot1 == 0) || (_OKbutton_slot1) )
				EnableWindow(OKbutton_slot1, TRUE);
			else
				EnableWindow(OKbutton_slot1, FALSE);
			return TRUE;
		}
	
		case WM_COMMAND:
		{
			switch (LOWORD(wparam))
			{
				case IDOK:
					{
						int Msg = IDYES;
						//zero 30-aug-2012 - do we really need to reset whenever we change the slot 1 device? why should resetting matter? if thats what you want to do, then do it.
						//if (romloaded && (needReset_slot1 || (temp_type_slot1!=slot1_device_type)) )
						//{
						//	Msg = MessageBox(dialog, 
						//			"After change slot1 device game will reset!\nAre you sure to continue?", "DeSmuME",
						//			MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2);
						//}
						if (Msg == IDYES)
						{
							if (wndConfigSlot1) DestroyWindow(wndConfigSlot1);
							EndDialog(dialog, TRUE);
						}
					}
				return TRUE;
				case IDCANCEL:
					if (wndConfigSlot1) DestroyWindow(wndConfigSlot1);
					EndDialog(dialog, FALSE);
				return TRUE;

				case IDC_ADDONS_LIST:
					if (HIWORD(wparam) == CBN_SELENDOK)
					{
						temp_type_slot1 = (NDS_SLOT1_TYPE)ComboBox_GetCurSel(GetDlgItem(dialog, IDC_ADDONS_LIST));
						if (temp_type_slot1 != last_type_slot1)
						{
							if (wndConfigSlot1) DestroyWindow(wndConfigSlot1);
							_OKbutton_slot1 = false;
							wndConfigSlot1=CreateDialogW(hAppInst, 
								MAKEINTRESOURCEW(Slot1_IDDs[temp_type_slot1]), dialog, 
								(DLGPROC)Slot1_Procs[temp_type_slot1]);
							if ( (temp_type_slot1 == 0) || (_OKbutton_slot1) )
								EnableWindow(OKbutton_slot1, TRUE);
							else
								EnableWindow(OKbutton_slot1, FALSE);
							u8 tmp_info[512];
							slot1List[temp_type_slot1].info((char *)tmp_info);
							SetWindowText(GetDlgItem(dialog, IDC_ADDONS_INFO), (char *)tmp_info);
							last_type_slot1 = temp_type_slot1;
						}
					}
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}

void slot1Dialog(HWND hwnd)
{
	strcpy(tmp_fat_path, slot1GetFatDir().c_str());
	temp_type_slot1 = slot1GetCurrentType();
	last_type_slot1 = temp_type_slot1;
	_OKbutton_slot1 = false;
	needReset_slot1 = true;
	u32 res=DialogBoxW(hAppInst, MAKEINTRESOURCEW(IDD_SLOT1CONFIG), hwnd, (DLGPROC)Slot1Box_Proc);
	if (res)
	{
		switch (temp_type_slot1)
		{
			case NDS_SLOT1_NONE:
				if (temp_type_slot1 != slot1GetCurrentType())
					needReset_slot1 = true;
				else
					needReset_slot1 = false;
				break;
			case NDS_SLOT1_RETAIL:
				break;
			case NDS_SLOT1_R4:
				if (strlen(tmp_fat_path))
				{
					slot1SetFatDir(tmp_fat_path);
					WritePrivateProfileString("Slot1","FAT_path",tmp_fat_path,IniName);
				}
				break;
			case NDS_SLOT1_RETAIL_NAND:
				break;
			default:
				return;
		}
		WritePrivateProfileInt("Slot1","type",temp_type_slot1,IniName);

		slot1Change((NDS_SLOT1_TYPE)temp_type_slot1);
		//zero 30-aug-2012 
		//if (romloaded && needReset_slot1)
		//	NDS_Reset();
		return;
	}
}
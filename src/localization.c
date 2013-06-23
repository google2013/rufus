/*
 * Rufus: The Reliable USB Formatting Utility
 * Localization functions, a.k.a. "Everybody is doing it wrong but me!"
 * Copyright © 2013 Pete Batard <pete@akeo.ie>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Memory leaks detection - define _CRTDBG_MAP_ALLOC as preprocessor macro */
#ifdef _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

#include "rufus.h"
#include "resource.h"
#include "msapi_utf8.h"
#include "localization.h"

#define LOC_CTRL(x) { #x, x }

// TODO: move this to an autogenerated file
loc_control_id control_id[] = {
	LOC_CTRL(IDS_DEVICE_TXT),
	LOC_CTRL(IDS_PARTITION_TYPE_TXT),
	LOC_CTRL(IDS_FILESYSTEM_TXT),
	LOC_CTRL(IDS_CLUSTERSIZE_TXT),
	LOC_CTRL(IDS_LABEL_TXT),
	LOC_CTRL(IDS_FORMATOPTIONS_TXT),
	LOC_CTRL(IDC_ADVANCED_GROUP),
	LOC_CTRL(IDC_BADBLOCKS),
	LOC_CTRL(IDC_QUICKFORMAT),
	LOC_CTRL(IDC_BOOT),
	LOC_CTRL(IDC_SET_ICON),
	LOC_CTRL(IDC_ADVANCED),
	LOC_CTRL(IDC_NBPASSES),
	LOC_CTRL(IDC_BOOTTYPE),
	LOC_CTRL(IDC_SELECT_ISO),
};

/* c control ID (no space, no quotes), s: quoted string, i: 32 bit signed integer,  */
loc_parse parse_cmd[] = {
	{ 'l', LC_LOCALE, "s" },
	{ 'v', LC_VERSION, "ii" },
	{ 't', LC_TEXT, "cs" },
	{ 'p', LC_PARENT, "c" },
	{ 'r', LC_RESIZE, "cii" },
	{ 'm', LC_MOVE, "cii" },
	{ 'f', LC_FONT, "si" },
	{ 'd', LC_DIRECTION, "i" },
};
size_t PARSE_CMD_SIZE = ARRAYSIZE(parse_cmd);
int  loc_line_nr = 0;
char loc_filename[32];

void free_loc_cmd(loc_cmd* lcmd)
{
	if (lcmd == NULL)
		return;
	safe_free(lcmd->text[0]);
	safe_free(lcmd->text[1]);
	free(lcmd);
}

// TODO: we need to store a revert for every action we execute here,
// or do we want to reinstantiate the dialogs?
BOOL execute_loc_cmd(loc_cmd* lcmd)
{
	size_t i;
	static HWND hParent = NULL;
	static char parent_name[128] = "IDD_DIALOG";	// Keep a copy of the parent
	HWND hCtrl = NULL;

	if (lcmd == NULL)
		return FALSE;

	if (hParent == NULL)
		hParent = hMainDialog;

//	uprintf("cmd #%d: ('%s', '%s') (%d, %d)\n",
//		lcmd->command, lcmd->text[0], lcmd->text[1], lcmd->num[0], lcmd->num[1]);
	if (lcmd->command <= LC_TEXT) {
		// Any command before LC_VERSION takes a control ID in text[0]
		for (i=0; i<ARRAYSIZE(control_id); i++) {
			if (safe_strcmp(lcmd->text[0], control_id[i].name) == 0) {
				hCtrl = GetDlgItem(hParent, control_id[i].id);
				break;
			}
		}
		if (hCtrl == NULL) {
			luprintf("'%s' is not a member of '%s'\n", lcmd->text[0], parent_name);
		}
	}

	switch(lcmd->command) {
	case LC_TEXT:
		if (hCtrl != NULL) {
			SetWindowTextU(hCtrl, lcmd->text[1]);
		}
		break;
	case LC_MOVE:
		if (hCtrl != NULL) {
			ResizeMoveCtrl(hParent, hCtrl, lcmd->num[0], lcmd->num[1], 0,  0);
		}
		break;
	case LC_RESIZE:
		if (hCtrl != NULL) {
			ResizeMoveCtrl(hParent, hCtrl, 0, 0, lcmd->num[0], lcmd->num[1]);
		}
		break;
	case LC_PARENT:
		// ???
		break;
	}
	return TRUE;

	// /!\ lcmd is freed after this call => if text messages need to be stored, they
	// must be removed from cmd so that they won't be freed
}

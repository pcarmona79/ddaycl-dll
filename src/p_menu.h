/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_menu.h,v $
 *   $Revision: 1.5 $
 *   $Date: 2002/06/04 19:49:49 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef __P_MENU_H__
#define __P_MENU_H__

#include "game.h"

enum {
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

typedef struct pmenuhnd_s {
	struct pmenu_s *entries;
	int cur;
	int num;
} pmenuhnd_t;

typedef struct pmenu_s {
	char *text;
	int align;
	void *arg;
	void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry, int choice);
} pmenu_t;

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num);
void PMenu_Close(edict_t *ent);
void PMenu_Update(edict_t *ent);
void PMenu_Next(edict_t *ent);
void PMenu_Prev(edict_t *ent);
void PMenu_Select(edict_t *ent);

#endif /* __P_MENU_H__ */

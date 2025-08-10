/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/u_entmgr.c,v $
 *   $Revision: 1.7 $
 *   $Date: 2002/06/04 19:49:50 $
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
 
 /*
  u_entmgr.c

  vjj

  This file contains the functions that a user would call to insert their 
  items into the itemlist[] and entities into the spawns[].

*/

#include "g_local.h"
#include "q_shared.h"


//these are the Entity functions
//These functions allow the user to insert and remove types of entities from 
//the spawns[].
//we need to be able to access the spawns[]

extern spawn_t spawns[];




//add an item to the itemlist array. Note that once you request a spot, it
//forever increases the number of items in the list,  even if you remove the 
//weapon.
//Note that most of the items in the item struct are pointers. No copy of the
//data is performed. You must maintain a copy of the data in your dll! 
//You also have to pass in a spawn_t struct that tells how to spawn your
//item.
gitem_t *InsertItem(gitem_t *it, spawn_t *spawnInfo)
{
	int i, inc_items;
	gitem_t *spot;
	spawn_t *spspot, *s;

	inc_items = 0;
	spot = NULL;

	// kernel: check if the element already exists
	spot = FindItemByClassnameInTeam(it->classname, it->dllname);

	if (spot)
	{
		// kernel: item already in list
		PrecacheItem (it); //pbowens: precache team items
		return spot;
	}

	// kernel: adds new element to the end of the array
	if (game.num_items < MAX_ITEMS - 1)
	{
		inc_items = game.num_items + 1;
		spot = &itemlist[inc_items];
	}

	if (spot)
	{
		//found a place in the item list, need to see if we can insert
		//the spawn function befrore we can insert item
		spspot = NULL;

		// kernel: look for the last element {NULL,NULL}
		for (s = spawns, i = 0; i < MAX_EDICTS && s->name; i++, s++)
			;

		// kernel: we need to make room to the new element, this will duplicate the {NULL,NULL}
		if (!s->name && i < (MAX_EDICTS - 1))
		{
			spspot = s; // former end of list
			spspot[1] = *spspot; // copies {NULL,NULL} to the new end
		}

		//OK, fill spot in with the stuff the user sent in
		if (spspot)
		{
			*spspot = *spawnInfo;
			*spot = *it;            //OK, fill spot in with the stuff the user sent in
			if (inc_items)
				game.num_items++;
		}
	}

	PrecacheItem (it); //pbowens: precache team items
	return spot;
}


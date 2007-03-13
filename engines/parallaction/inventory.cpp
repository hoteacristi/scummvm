/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "parallaction/parallaction.h"
#include "parallaction/disk.h"
#include "parallaction/zone.h"
#include "parallaction/graphics.h"
#include "parallaction/inventory.h"


namespace Parallaction {

//
//	inventory is a grid made of (at most) 30 cells, 24x24 pixels each,
//	arranged in 6 lines
//
//	inventory items are stored in cnv files in a 32x24 grid
//	but only 24x24 pixels are actually copied to graphic memory
//

#define INVENTORY_MAX_ITEMS 		30

#define INVENTORYITEM_PITCH 		32
#define INVENTORYITEM_WIDTH 		24
#define INVENTORYITEM_HEIGHT		24

#define INVENTORY_ITEMS_PER_LINE	5
#define INVENTORY_LINES 			6

#define INVENTORY_WIDTH 			(INVENTORY_ITEMS_PER_LINE*INVENTORYITEM_WIDTH)
#define INVENTORY_HEIGHT			(INVENTORY_LINES*INVENTORYITEM_HEIGHT)

static byte		*_buffer;
extern Cnv 		 _yourObjects;
uint16			 _numInvLines = 0;
static Point	 _invPosition = { 0, 0 };

InventoryItem _inventory[INVENTORY_MAX_ITEMS] = {
	{ kZoneDoor,		1 },		// open/close icon
	{ kZoneExamine, 	3 },		// examine icon
	{ kZoneGet, 		2 },		// pick up/use icon
	{ kZoneSpeak,		4 },		// speak icon
	{ 0,	0 },					// items...
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 }
};

void drawInventoryItem(uint16 pos, InventoryItem *item);


//	get inventory item index at position (x,y)
//	in screen coordinates
//
int16 Parallaction::getHoverInventoryItem(int16 x, int16 y) {

	int16 _di = x;

	int16 _si = -1;
	do {
		_si++;
	} while (_inventory[_si]._id != 0);

	_si = (_si + 4) / INVENTORY_ITEMS_PER_LINE;

	if (_invPosition._x >= _di) return -1;
	if ((_invPosition._x + INVENTORY_WIDTH) <= _di) return -1;

	if (_invPosition._y >= y) return -1;
	if ((_si * INVENTORYITEM_HEIGHT + _invPosition._y) <= y) return -1;

	return ((_di - _invPosition._x) / INVENTORYITEM_WIDTH) + (INVENTORY_ITEMS_PER_LINE * ((y - _invPosition._y) / INVENTORYITEM_HEIGHT));

}


int16 pickupItem(Zone *z) {

	uint16 _si;
	for (_si = 0; _inventory[_si]._id != 0; _si++) ;
	if (_si == INVENTORY_MAX_ITEMS)
		return -1;

	_inventory[_si]._id = MAKE_INVENTORY_ID(z->u.get->_icon);
	_inventory[_si]._index = z->u.get->_icon;

	addJob(jobRemovePickedItem, z, kPriority17 );

	if (_inventory[_si]._id == 0) return 0;

	refreshInventoryItem(_vm->_characterName, _si);

	return 0;
}


void addInventoryItem(uint16 item) {

	uint16 _si = 0;
	while (_inventory[_si]._id != 0) _si++;

	_inventory[_si]._id = MAKE_INVENTORY_ID(item);
	_inventory[_si]._index = item;

	refreshInventoryItem(_vm->_characterName, _si);

	return;
}


void dropItem(uint16 v) {

	uint16 _di = 0;
	for (uint16 _si = 0; _si < INVENTORY_MAX_ITEMS - 1; _si++) {

		if (v + 4 == _inventory[_si]._index) {
			_di = 1;
		}

		if (_di == 0) continue;

		memcpy(&_inventory[_si], &_inventory[_si+1], sizeof(InventoryItem));
	}

	refreshInventory(_vm->_characterName);

	return;
}


int16 isItemInInventory(int32 v) {

	for (uint16 _si = 0; _si < INVENTORY_MAX_ITEMS; _si++) {
		if (_inventory[_si]._id == (uint)v)
			return 1;
	}

	return 0;
}



void drawInventoryItem(uint16 pos, InventoryItem *item) {

	uint16 line = pos / INVENTORY_ITEMS_PER_LINE;
	uint16 col = pos % INVENTORY_ITEMS_PER_LINE;

	// FIXME: this will end up in a general blit function
	byte* s = _yourObjects._array[item->_index];
	byte* d = _buffer + col * INVENTORYITEM_WIDTH + line * _yourObjects._height * INVENTORY_WIDTH;
	for (uint32 i = 0; i < INVENTORYITEM_HEIGHT; i++) {
		memcpy(d, s, INVENTORYITEM_WIDTH);

		d += INVENTORY_WIDTH;
		s += INVENTORYITEM_PITCH;
	}

	return;
}

void drawBorder(const Common::Rect& r, byte *buffer, byte color) {

	byte *d = buffer + r.left + INVENTORY_WIDTH * r.top;

	memset(d, color, r.width());

	for (uint16 i = 0; i < r.height(); i++) {
		d[i * INVENTORY_WIDTH] = color;
		d[i * INVENTORY_WIDTH + r.width() - 1] = color;
	}

	d = buffer + r.left + INVENTORY_WIDTH * (r.bottom - 1);
	memset(d, color, r.width());

	return;
}

//
//	draws a color border around the specified position in the inventory
//
void highlightInventoryItem(int16 pos, byte color) {

	if (color != 12) color = 19;

	if (pos == -1) return;

	uint16 line = pos / INVENTORY_ITEMS_PER_LINE;
	uint16 col = pos % INVENTORY_ITEMS_PER_LINE;

	Common::Rect r(INVENTORYITEM_WIDTH, _yourObjects._height);
	r.moveTo(col * INVENTORYITEM_WIDTH, line * _yourObjects._height);

	drawBorder(r, _buffer, color);

	return;
}




void extractInventoryGraphics(int16 pos, byte *dst) {
//	printf("extractInventoryGraphics(%i)\n", pos);

	int16 line = pos / INVENTORY_ITEMS_PER_LINE;
	int16 col = pos % INVENTORY_ITEMS_PER_LINE;

	// FIXME: this will end up in a general blit function
	byte* d = dst;
	byte* s = _buffer + col * INVENTORYITEM_WIDTH + line * _yourObjects._height * INVENTORY_WIDTH;
	for (uint32 i = 0; i < INVENTORYITEM_HEIGHT; i++) {
		memcpy(d, s, INVENTORYITEM_WIDTH);

		s += INVENTORY_WIDTH;
		d += INVENTORYITEM_PITCH;
	}

	return;
}

void jobShowInventory(void *parm, Job *j) {
//	printf("job_showInventory()...");

	_numInvLines = 0;

	while (_inventory[_numInvLines]._id != 0) _numInvLines++;

	_numInvLines = (_numInvLines + 4) / INVENTORY_ITEMS_PER_LINE;

	_vm->_gfx->copyRect(
		Gfx::kBitBack,
		_invPosition._x,
		_invPosition._y,
		INVENTORY_WIDTH,
		_numInvLines * INVENTORYITEM_HEIGHT,
		_buffer,
		INVENTORY_WIDTH
	);

	return;
}



void jobHideInventory(void *parm, Job *j) {
//	printf("job_hideInventory()\n");

	static uint16 count = 0;

	_engineFlags |= kEngineMouse;

	count++;
	if (count == 2) {
		count = 0;
		j->_finished = 1;
		_engineFlags &= ~kEngineMouse;
	}

	_vm->_gfx->copyRect(
		Gfx::kBit2,
		_invPosition._x,
		_invPosition._y,
		Gfx::kBitBack,
		_invPosition._x,
		_invPosition._y,
		INVENTORY_WIDTH,
		_numInvLines * INVENTORYITEM_HEIGHT
	);

	return;
}



void openInventory() {
//	printf("openInventory()\n");

	uint16 _si = 0;
	_engineFlags |= kEngineInventory;

	while (_inventory[_si]._id != 0) _si++;

	uint16 _LOCALinventory_lines = (_si + 4) / INVENTORY_ITEMS_PER_LINE;

	_invPosition._x = _mousePos._x - (INVENTORY_WIDTH / 2);
	if (_invPosition._x < 0)
		_invPosition._x = 0;

	if ((_invPosition._x + INVENTORY_WIDTH) > SCREEN_WIDTH)
		_invPosition._x = SCREEN_WIDTH - INVENTORY_WIDTH;

	_invPosition._y = _mousePos._y - 2 - (_LOCALinventory_lines * INVENTORYITEM_HEIGHT);
	if (_invPosition._y < 0)
		_invPosition._y = 0;

	if (_invPosition._y > SCREEN_HEIGHT - _LOCALinventory_lines * INVENTORYITEM_HEIGHT)
		_invPosition._y = SCREEN_HEIGHT - _LOCALinventory_lines * INVENTORYITEM_HEIGHT;

	return;

}



void closeInventory() {
//	printf("closeInventory()\n");

	_engineFlags &= ~kEngineInventory;
}



// refreshes inventory view
//
void redrawInventory() {


	for (uint16 _si = 0; _si < INVENTORY_MAX_ITEMS; _si++) {
		drawInventoryItem(_si, &_inventory[_si]);
	}

}

void initInventory() {
	_buffer = (byte*)malloc(INVENTORY_WIDTH * INVENTORY_HEIGHT);	  // this buffer is also used by menu so it must stay this size
	_yourObjects._count = 0;
}

void cleanInventory() {

	for (uint16 _si = 4; _si < 30; _si++) {
		_inventory[_si]._id = 0;
		_inventory[_si]._index = 0;
	}

	return;
}




void refreshInventory(const char *character) {
	redrawInventory();

	return;
}


void refreshInventoryItem(const char *character, uint16 index) {
	drawInventoryItem(index, &_inventory[index]);

	return;
}

} // namespace Parallaction

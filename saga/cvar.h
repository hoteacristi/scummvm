/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Configuration Variable Module

#ifndef SAGA_CVAR_H_
#define SAGA_CVAR_H_

namespace Saga {

#define CVAR_HASHLEN 32

struct SUBCVAR_INT {
	cv_int_t *var_p;
	cv_int_t ubound;
	cv_int_t lbound;
};

struct SUBCVAR_UINT {
	cv_uint16_t *var_p;
	cv_uint16_t ubound;
	cv_uint16_t lbound;

};

struct SUBCVAR_FLOAT {
	cv_float_t *var_p;
	cv_float_t ubound;
	cv_float_t lbound;
};

struct SUBCVAR_STRING {
	cv_char_t *var_str;
	int ubound;
};

struct SUBCVAR_FUNC {
	cv_func_t func_p;
	const char *func_argstr;
	int min_args;
	int max_args;
};

struct CVAR {
	int type;
	const char *name;
	const char *section;
	uint16 flags;
	void *refCon;

	union {
	 SUBCVAR_INT i;
	 SUBCVAR_UINT ui;
	 SUBCVAR_FLOAT f;
	 SUBCVAR_STRING s;
	 SUBCVAR_FUNC func;
	} t;

	CVAR *next;

};

} // End of namespace Saga

#endif

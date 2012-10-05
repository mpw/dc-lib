// =====================================================================
//  dc/src/dc_message.c
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu
//  License:        See the end of this file for license information
//  Created:        January 5, 2011
//  Module:         dc

//  Last changed:   $Date: 2011/02/07 12:52:21 $
//  Changed by:     $Author: demetres $
//  Revision:       $Revision: 1.3 $


#include "dc_globals.h"


// private var
static size_t _dc_indent_s = 0;


// ---------------------------------------------------------------------
//  _dc_add_to_message_indent
// ---------------------------------------------------------------------
void _dc_add_to_message_indent(int delta) {
    _dc_indent_s += delta;
}


// ---------------------------------------------------------------------
//  _dc_set_message_indent
// ---------------------------------------------------------------------
void _dc_set_message_indent(size_t indent) {
    _dc_indent_s = indent;
}


// ---------------------------------------------------------------------
//  _dc_get_message_indent
// ---------------------------------------------------------------------
size_t _dc_get_message_indent() {
    return _dc_indent_s;
}


// ---------------------------------------------------------------------
//  _dc_vfmessage
// ---------------------------------------------------------------------
void _dc_vfmessage(FILE* fp, char* msg, va_list args) {
    if (fp == NULL) fp = _DC_DEFAULT_MSG_STREAM;
    fprintf(fp, "%s%*s", _DC_MSG_PREFIX, _dc_indent_s, "");
	vfprintf(fp, msg, args);
    fprintf(fp, "\n");
}


// ---------------------------------------------------------------------
//  _dc_fmessage
// ---------------------------------------------------------------------
void _dc_fmessage(FILE* fp, char* msg, ...) {
    va_list args;
    va_start(args, msg);
    _dc_vfmessage(fp, msg, args);
    va_end(args);
}


// ---------------------------------------------------------------------
//  _dc_message
// ---------------------------------------------------------------------
void _dc_message(char* msg, ...) {
    va_list args;
    va_start(args, msg);
    _dc_vfmessage(NULL, msg, args);
    va_end(args);
}


// ---------------------------------------------------------------------
//  _dc_panic
// ---------------------------------------------------------------------
void _dc_panic(char* msg, ...) {
    va_list args;
    va_start(args, msg);
    _dc_vfmessage(NULL, msg, args);
    va_end(args);
    exit(1);
}


// Copyright (C) 2011 Camil Demetrescu

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA

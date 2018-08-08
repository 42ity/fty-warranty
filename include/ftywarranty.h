/*  =========================================================================
    ftywarranty - class description

    Copyright (C) 2014 - 2018 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#ifndef FTYWARRANTY_H_INCLUDED
#define FTYWARRANTY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new ftywarranty
FTY_WARRANTY_EXPORT ftywarranty_t *
    ftywarranty_new (void);

//  Destroy the ftywarranty
FTY_WARRANTY_EXPORT void
    ftywarranty_destroy (ftywarranty_t **self_p);

//  Self test of this class
FTY_WARRANTY_EXPORT void
    ftywarranty_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif

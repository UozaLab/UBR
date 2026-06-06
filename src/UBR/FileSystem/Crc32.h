/*
 * Copyright (c) 2024-2026, UozaLab
 * This file is based on the RFC 1952 sample code.
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __CRC32__H__
#define __CRC32__H__

#include <windows.h>

class CRC32
{
protected:
    UINT32 crc_table[256];
    UINT32 polynomial_rev;

    void make_crc_table();
    UINT32 update_crc(UINT32 crc, UINT8 *buf, int len);

public:
    CRC32(UINT32 _polynomial_rev);
    // polynomial_rev
    //  0xedb88320 <=rev 0x04c11db7
    //  0x82f63b78 <=rev 0x1EDC6F41
    UINT32 calculate(UINT8 *buf, int len);
};

#endif

/*
 * Copyright (c) 2024-2026, UozaLab
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

#include "Crc32.h"

CRC32::CRC32(UINT32 _polynomial_rev)
: polynomial_rev(_polynomial_rev)
{
    make_crc_table();
}

void CRC32::make_crc_table()
{
    UINT32 c;

    for (int n = 0; n < 256; n++) {
        c = (UINT32) n;
        for (int k = 0; k < 8; k++) {
            if (c & 1) {
                c = polynomial_rev ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc_table[n] = c;
    }
}

UINT32 CRC32::update_crc(UINT32 crc, UINT8 *buf, int len)
{
    UINT32 c = crc ^ 0xffffffffL;
    int n;

    for (n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c ^ 0xffffffffL;
}


UINT32 CRC32::calculate(UINT8 *buf, int len)
{
    return update_crc(0L, buf, len);
}


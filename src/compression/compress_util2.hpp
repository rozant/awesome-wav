/*****************************************************************
* This program is free software; you can redistribute it and/or	 *
* modify it under the terms of the GNU General Public License	 *
* version 2 as published by the Free Software Foundation.		 *
*																 *
* This program is distributed in the hope that it will be		 *
* useful, but WITHOUT ANY WARRANTY; without even the implied	 *
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR		 *
* PURPOSE.  See the GNU General Public License for more details. *
*																 *
* You should have received a copy of the GNU General Public 	 *
* License along with this program; if not, write to the Free 	 *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,	 *
* USA.															 *
*****************************************************************/
#ifndef __compress_util_hpp__
#define __compress_util_hpp__

#ifndef gettext
#define gettext(Msgid) ((const char *) (Msgid))
#endif

// compress_util return codes
enum qlz_compress_util_ret_code {
	QLZ_BUFF_MEM_FAIL = -4,
	QLZ_OFILE_FAIL = -3,
	QLZ_IFILE_FAIL = -2,
	QLZ_FAIL = -1,
	QLZ_SUCCESS = 0
};

// function prototypes
int qlz_compress_file(const char *, const char *, const char);
int qlz_decompress_file(const char *, const char *);
const char *qlz_comp_err(const int);

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/


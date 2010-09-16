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
#ifndef __logger_hpp__
#define __logger_hpp__
#include "global.hpp"

/****************************************************************/
/* class: _ENTRY												*/
/* purpose: store a single log message						 	*/
/****************************************************************/
struct _ENTRY {
	char *message;
};

/****************************************************************/
/* class: logger												*/
/* purpose: store and manage log messages					 	*/
/****************************************************************/
class logger
{
	private:
		logger(void);
		~logger(void);

		bool resize(void);

		_ENTRY *entries;
		DWORD numEntries;
		DWORD maxEntries;

	public:
		friend logger& getLogger();

		bool record(const char *);
		void print();
		void clean();
};

logger& getLogger(void);

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/


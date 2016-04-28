/*logger.h*/

/*Copyright (C) 2016 Jonas Fuglsang-Petersen*/

/*chatnut is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

chatnut is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with chatnut.  If not, see <http://www.gnu.org/licenses/>.*/

#ifdef LOGGER_H_
#define LOGGER_H_

extern void error(const char *cat, const char *msg);
extern void log(const char *cat, const char *msg);
extern void warn(const char *cat, const char *msg);

extern void set_error(const char *filename);
extern void set_log(const char *filename);
extern void set_warn(const char *filename);

extern void shutdown_logger(void);

#endif			//LOGGER_H_
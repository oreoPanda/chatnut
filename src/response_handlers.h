/*response_handlers.h*/

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

#ifndef RESPONSE_HANDLERS_H
#define RESPONSE_HANDLERS_H

extern void handle_buddy_is_set(char *username);
extern void handle_lookup_success(char *contact);
extern void handle_login_success(char *username);

#endif /* RESPONSE_HANDLERS_H */


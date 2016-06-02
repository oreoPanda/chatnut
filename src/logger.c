/*logger.c*/

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

#include "logger.h"

FILE *ef = NULL, *lf = NULL, *wf = NULL;

//FIXME WsaGetLastError() isn't being evaluated
extern void error(const char *cat, const char *msg)
{
	if(ef)
	{
		fprintf(ef, "[%s Error] %s: %s.\n", cat, msg, strerror(errno));
	}
	else
	{
		fprintf(stderr, "[%s Error] %s: %s.\n", cat, msg, strerror(errno));
	}
	return;
}

extern void logg(const char *cat, const char *msg)
{
	if(lf)
	{
		fprintf(lf, "[%s] %s.\n", cat, msg);
	}
	else
	{
		fprintf(stdout, "[%s] %s.\n", cat, msg);
	}
	return;
}

extern void warn(const char *cat, const char *msg)
{
	if(wf)
	{
		fprintf(wf, "[%s Warning] %s.\n", cat, msg);
	}
	else
	{
		fprintf(stdout, "[%s Warning] %s.\n", cat, msg);
	}
	return;
}

extern void logger_init(void)
{
	ef = stderr;
	lf = stdout;
	wf = stdout;
}

extern void set_error(const char *filename)
{
	if(!(ef = fopen(filename, "a")) )
	{
		error("Logger", "Unable to open file for errors");
		ef = stderr;
	}
	
	return;
}

extern void set_log(const char *filename)
{
	if( !(lf = fopen(filename, "a")) )
	{
		error("Logger", "Unable to open file for log messages");
		lf = stdout;
	}
	
	return;
}

extern void set_warn(const char *filename)
{
	if( !(wf = fopen(filename, "a")) )
	{
		error("Logger", "Unable to open file for warnings");
		wf = stdout;
	}
	
	return;
}

extern void shutdown_logger(void)
{
	if(ef != stderr)
	{
		errno = 0;
		int s = fclose(ef);
		ef = stderr;
		if(s != 0)
		{
			error("Logger", "Encountered error while closing file for errors");
		}
	}
	if(lf != stdout)
	{
		errno = 0;
		int s = fclose(lf);
		lf = stdout;
		if(s != 0)
		{
			error("Logger", "Encountered error while closing file for logs");
		}
	}
	if(wf != stdout)
	{
		errno = 0;
		int s = fclose(wf);
		wf = stdout;
		if(s != 0)
		{
			error("Logger", "Encountered error while closing file for warnings");
		}
	}
}

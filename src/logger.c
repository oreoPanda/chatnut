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

//TODO here and in shutdown_logger() check if stderr and stdout can be a FILE*

FILE *ef = stderr, *lf = stdout, *wf = stdout;

extern void error(const char *cat, const char *msg)
{
	fprintf(ef, "[%s] %s.\n", cat, msg);
	return;
}

extern void log(const char *cat, const char *msg)
{
	fprintf(lf, "[%s] %s.\n", cat, msg);
	return;
}

extern void warn(const char *cat, const char *msg)
{
	fprintf(wf, "[%s] %s.\n", cat, msg);
	return;
}

extern void set_error(const char *filename)
{
	if(!(ef = fopen(filename)) )
	{
		error("Logger", "Unable to open file for errors");
		ef = stderr;
	}
	
	return;
}

extern void set_log(const char *filename)
{
	if( !(lf = fopen(filename)) )
	{
		error("Logger", "Unable to open file for log messages");
		lf = stdout;
	}
	
	return;
}

extern void set_warn(const char *filename)
{
	if( !(wf = fopen(filename)) )
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
		fclose(ef);
		ef = stderr;
	}
	if(lf != stdout)
	{
		fclose(lf);
		lf = stdout;
	}
	if(wf != stdout)
	{
		fclose(wf);
		wf = stdout;
	}
}
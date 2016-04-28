FILE *ef = stderr, *lf = stdout, *wf = stdout;

extern void error(const char *cat, const char *msg)
{
	fprintf(ef, "[%s] %s.\n", cat, msg);
	return;
}

extern void log(const char *cat, const char *msg)
{
	return;
}

extern void warn(const char *cat, const char *msg)
{
	return;
}

extern void set_error(const char *filename)
{
	if(!(ef = fopen(filename)) )
	{
		error("Logger", "Unable to open file for errors");
	}
	
	return;
}

extern void set_log(const char *filename)
{
	if( !(lf = fopen(filename)) )
	{
		error("Logger", "Unable to open file for log messages");
	}
	
	return;
}

extern void set_warn(const char *filename)
{
	return;
}
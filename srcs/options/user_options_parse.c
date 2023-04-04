#include "options/user_options.h"
#include "options/xgetopts.h"
#include "expr.h"
#include "xleaks.h"
#include "utils.h"
#include "log.h"
#include "xtypes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

t_user_options *user_options_parse(int ac, char *av[])
{
	t_user_options *options;
	t_xgetopts     opts;
	const char     *exec_name = av[0];
	char           c;
	
	options = __xmalloc__(sizeof(t_user_options));
	if (!options)
		return (NULL);
		
	user_options_init(options);
	xgetopts_init(&opts, ac, (cut8 **)av, "o:n:srh");

	while ((c = xgetopts_next(&opts)) != (char)-1)
	{
		switch (c)
		{
			case 'o':
				if (!expr_parse(opts.arg,
						&options->start_offset)) {
					return (__xfree__(options), NULL);
				} break ;
			
			case 'n':
				if (!expr_parse(opts.arg,
						&options->range)) {
					return (__xfree__(options), NULL);
				} break ;
			
			case 'r':
				options->mode = XDP_STREAM;
				break ;
			
			case 's':
				options->mode = XDP_STRINGS;
				break ;
			
			case '*':
				options->filename = opts.arg;
				break ;
			
			case 'h':
			case '?':
				fprintf(
					stdout,
						"Usage: %s [OPTIONS] FILE\n\n"
						"Description:\n"
						"    Display the contents of a file in hexadecimal format.\n\n"
						"Options:\n"
						"    -n   EXPR  The range of bytes to read from the file (default: unlimited).\n"
						"    -o   EXPR  Starting byte offset to read from (default: 0).\n"
						"    -s         Dump strings at least 4 characters long.\n"
						"    -r         Dump as a stream of hex characters.\n"
						"    -h         Show this help message\n\n",
					exec_name
				);
				__xfree__(options);
				exit(0);
		}
	}
	
	if (opts.fail)
	{
		__log__(fatal, "Invalid arguments");
		__xfree__(options);
		return (NULL);
	}

	return (options);
}
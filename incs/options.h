#ifndef __OPTIONS_H__
# define __OPTIONS_H__

# include <stddef.h>
# include <stdbool.h>
# include "infile.h"

typedef enum e_xdp_mode
{
	M_NORMAL,
	M_STRING,
	M_STREAM,
}	t_xdp_mode;

typedef struct s_user_options
{
	const char    *filename;
	t_xdp_mode    mode;
	size_t        start_offset;
	size_t        string_size;
	bool          colors;
	size_t        range;
}	t_user_options;

t_user_options 	*parse_user_options(int ac, char *av[]);
bool 			options_within_range(t_user_options *opts, t_infile *file);
void 			options_struct_debug_print(t_user_options *opts);

#endif /* __OPTIONS_H__ */
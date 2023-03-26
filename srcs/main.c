#include "xdp.h"
#include "xmem.h"
#include "options/user_options.h"
#include <stdlib.h>

int main(int ac, char *av[])
{
#ifdef __XMEM__
	xmem_trace_init();
#endif
	
	t_user_options *opts = user_options_parse(ac, av);

	if (opts == NULL)
		return (EXIT_FAILURE);
	
	bool success = _entry_(opts);
	__xfree__(opts);

#ifdef __XMEM__
	xmem_print_summary();
	xmem_trace_destroy();
#endif

	return (success == false);
}

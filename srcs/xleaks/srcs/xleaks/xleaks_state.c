#include "xleaks.h"	
#include <stdio.h>

void xleaks_state(void)
{
	t_active_rss *tmp = xleak_trace.list;

	if (xleak_trace.list == NULL)
		return;

	fprintf(stdout, "\n[active]\n\n");
	while (tmp)
	{
		active_rss_print(tmp);
		tmp = tmp->next;
	}
}
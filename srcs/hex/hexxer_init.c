#include "hex.h"
#include "log.h"
#include "file.h"
#include "xtypes.h"
#include "xleaks.h"
#include "file.h"
#include "options/user_options.h"
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

static  bool options_within_file_size(t_user_options *opts, size_t file_size)
{
    if (opts->start_offset < file_size) {
        if (opts->start_offset + opts->range > file_size)
        {
            __log__(error,  "range exceeds the maximum offset (%zx)", file_size);
            return (false);
        }
        if (!opts->range)
            opts->range = file_size - opts->start_offset; 
        return (true);
    }

    __log__(error,  
        "start offset (%zx) exceeds the maximum offset (%zx)",
        opts->start_offset,
        file_size
    );
    return (false);
}

static bool file_mmap_recommended(t_file *file, size_t range_size)
{
    size_t page_size = sysconf(_SC_PAGE_SIZE);
    size_t block_size = file->st.st_blksize;

    if (file->size < page_size)
        return (false);

    if (range_size < block_size) 
        return (false);

    if (range_size >= 2 * page_size
        && range_size >= block_size)
        return (true);

    return (false);
}

static bool file_mmap_from_offset(int fd, t_hexxer *hexxer)
{
	size_t aligned_offset = hexxer->start_offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

	hexxer->start_offset = (hexxer->start_offset - aligned_offset);
	hexxer->data.cap = hexxer->max_size + hexxer->start_offset;
	hexxer->data.size = hexxer->max_size;

	hexxer->data.ptr = mmap(
			NULL,
			hexxer->max_size,
	 		PROT_READ,
	 		MAP_PRIVATE,
	 		fd,
			aligned_offset);
	

	if (hexxer->data.ptr == MAP_FAILED)
	{
		__log__(fatal,  "mmap: %s", strerror(errno));
		return (false);
	}
	hexxer->mapped = true;
	return (true);
}

static bool init_screen(t_file *file, t_hexxer *hexxer)
{
	size_t best_size = file->st.st_blksize / 16 * (16 * 3 + 28);
	
	hexxer->screen.ptr = __xmalloc__(best_size * 8);
	if (!hexxer->screen.ptr)
		return (false);
	
	hexxer->screen.size = best_size * 8;
	return (true);
}

static bool init_special_file_hexxer(t_user_options *opts, t_hexxer *hexxer)
{
	hexxer->start_offset = opts->start_offset;
	hexxer->max_size = opts->range;
	return (true);
}

static bool init_regular_file_hexxer(int fd, t_file *file, t_user_options *opts, t_hexxer *hexxer)
{
	if (!options_within_file_size(opts, file->size))
		return (false);
	
	hexxer->start_offset = opts->start_offset;
	hexxer->max_size = opts->range;

	if (file_mmap_recommended(file, hexxer->max_size))
	{
		if (!file_mmap_from_offset(fd, hexxer))
			return (false);
	}
	return (true);
}

t_hexxer *hexxer_init(int fd, t_file *file, t_user_options *opts)
{
	t_hexxer *hexxer = __xmalloc__(sizeof(t_hexxer));
	if (hexxer == NULL)
		return (NULL);

	bzero(hexxer, sizeof(t_hexxer));
	hexxer->start_offset = opts->start_offset;
	hexxer->max_size = opts->range;

	switch (file->type)
	{
		case FILE_TYPE_DIRECTORY:
		case FILE_TYPE_SOCKET:
		case FILE_TYPE_UNKNOWN:
		case FILE_TYPE_SYMBOLIC_LINK:
			__log__(error, "No handler available");
			goto prison;

		case FILE_TYPE_REGULAR:
			if (!init_regular_file_hexxer(fd, file, opts, hexxer))
				goto prison;
			break;

		case FILE_TYPE_PIPE:
		case FILE_TYPE_BLOCK_DEVICE:
		case FILE_TYPE_CHARACTER_DEVICE:
			hexxer->read_size = file->st.st_blksize;
			if (!init_special_file_hexxer(opts, hexxer))
				goto prison;
			break;
	}

	if (init_screen(file, hexxer))
		return (hexxer);

prison:
	hexxer_destroy(hexxer);
	__xfree__(hexxer);
	return (NULL);
}
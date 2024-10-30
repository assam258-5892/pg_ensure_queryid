#ifndef __PG_INZENT_HELPER_H__
#define __PG_INZENT_HELPER_H__

#define DECLARE_HOOK(hook, ...) \
	        static hook(__VA_ARGS__);
#define HOOK(name) name
#define HOOK_STATS_SIZE 0

#endif

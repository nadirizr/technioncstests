#ifndef _VSF_UTILS_H
#define _VSF_UTILS_H 

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#define DO_SYSTEM(cmd) do { char c[256]; sprintf(c,cmd); system(c); } while (0)
#define DO_SYSTEM_WITH_PARAMS(fmt, arg) do { char c[256]; sprintf(c, fmt, arg); system(c); } while (0)

#define MKNOD(NAME,MAJOR,MINOR) mknod(NAME,S_IFCHR | 0666 ,makedev(MAJOR,MINOR))
#define RM(NAME) DO_SYSTEM_WITH_PARAMS("rm -f %s", (NAME))

#define INSMOD(max_vsf_devices) DO_SYSTEM_WITH_PARAMS("insmod vsf.o max_vsf_devices=%d", (max_vsf_devices))
#define RMMOD do { system("rmmod vsf"); } while(0)

#endif /* _VSF_UITLS_H */

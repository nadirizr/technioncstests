#ifndef _VSF_H_
#define _VSF_H_

#include <linux/ioctl.h>

struct vsf_command_parameters {
	unsigned char read_minor;
	unsigned char write_minor;
};

#define VSF_IOCTL_MAGIC (('v'+'s'+'f')%256)

#define VSF_CREATE _IOW(VSF_IOCTL_MAGIC,0,struct vsf_command_parameters)
//argument: struct vsf_command_parameters* parameters

#define VSF_FREE _IOW(VSF_IOCTL_MAGIC,1,struct vsf_command_parameters)
//argument: struct vsf_command_parameters* parameters

#endif

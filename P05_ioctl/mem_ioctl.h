#ifndef MEM_IOCTL_H

#define MEM_IOCTL_H

#include <linux/ioctl.h>

#define MEM_SET_STORE_SIZE _IOW('m', 1, int)
#define MEM_GET_STORE_SIZE _IOR('m', 2, int *)

#endif

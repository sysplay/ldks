#ifndef RTC_IOCTL_H

#define RTC_IOCTL_H

#include <linux/ioctl.h>

#define RTC_GET_SECOND _IOR('c', 1, int *)
#define RTC_SET_SECOND _IOW('c', 2, int)
#define RTC_GET_MINUTE _IOR('c', 3, int *)
#define RTC_SET_MINUTE _IOW('c', 4, int)
#define RTC_GET_HOUR _IOR('c', 5, int *)
#define RTC_SET_HOUR _IOW('c', 6, int)
#define RTC_GET_DAY _IOR('c', 7, int *)
#define RTC_SET_DAY _IOW('c', 8, int)
#define RTC_GET_DATE _IOR('c', 9, int *)
#define RTC_SET_DATE _IOW('c', 10, int)
#define RTC_GET_MONTH _IOR('c', 11, int *)
#define RTC_SET_MONTH _IOW('c', 12, int)
#define RTC_GET_YEAR _IOR('c', 13, int *)
#define RTC_SET_YEAR _IOW('c', 14, int)

#endif

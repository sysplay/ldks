#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "rtc.h"

int main(int argc, char *argv[])
{
	char *file_name = "/dev/clock";
	int fd;
	int choice;
	int i;
	unsigned char reg;
	char *day_str[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int y, mo, d, dy, h, m, s;

	if (argc > 2)
	{
		fprintf(stderr, "Usage: %s [<device file name>]\n", argv[0]);
		return 1;
	}
	else if (argc == 2)
	{
		file_name = argv[1];
	}

	fd = open(file_name, O_RDWR);
	if (fd == -1)
	{
		perror("rtc open");
		return 2;
	}

	do
	{
		printf("1: Show CMOS-RTC Register Contents\n");
		printf("2: Get Date\n");
		printf("3: Get Time\n");
		printf("4: Set Date\n");
		printf("5: Set Time\n");
		printf("0: Exit\n");
		printf("Enter choice: ");
		scanf("%d%*c", &choice);
		switch (choice)
		{
			case 1:
				if (lseek(fd, 0, SEEK_SET) == -1)
				{
					perror("rtc lseek");
					break;
				}
				printf("CMOS-RTC I/O Port Contents:\n");
				i = 0;
				while (read(fd, &reg, 1) == 1)
				{
					printf("%02X: %02X\n", i, reg);
					i++;
				}
				break;
			case 2:
				if (ioctl(fd, RTC_GET_YEAR, &y) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_GET_MONTH, &mo) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_GET_DATE, &d) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_GET_DAY, &dy) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				printf("Current Date: %02d.%02d.%d%02d %s\n", d, mo, (y < 70) ? 20 : 19, y, day_str[dy]);
				break;
			case 3:
				if (ioctl(fd, RTC_GET_HOUR, &h) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_GET_MINUTE, &m) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_GET_SECOND, &s) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				printf("Current Time: %02d:%02d:%02d\n", h, m, s);
				break;
			case 4:
				printf("Enter the Date to Set in DD.MM.YY D format (D == 1 => Sun): ");
				scanf("%d.%d.%d %d%*c", &d, &mo, &y, &dy);
				if (ioctl(fd, RTC_SET_DATE, d) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_SET_MONTH, mo) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_SET_YEAR, y) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_SET_DAY, dy) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				break;
			case 5:
				printf("Enter the Time to Set in HH:MM:SS format: ");
				scanf("%d:%d:%d%*c", &h, &m, &s);
				if (ioctl(fd, RTC_SET_SECOND, s) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_SET_MINUTE, m) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				if (ioctl(fd, RTC_SET_HOUR, h) == -1)
				{
					perror("rtc ioctl");
					break;
				}
				break;
		}
	} while (choice != 0);

	close (fd);

	return 0;
}

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

//#define MEM_IOCTL 1
#define BUF_SIZE 128

#ifdef MEM_IOCTL
#include "mem_ioctl.h"
#endif

int main(int argc, char *argv[])
{
	char *file_name;
	int fd;
	int choice;
	unsigned char data[BUF_SIZE];
	int cnt, i;
	off_t seek;
#ifdef MEM_IOCTL
	int size;
#endif

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <device file name>\n", argv[0]);
		return 1;
	}
	else
	{
		file_name = argv[1];
		fd = -1;
	}

	do
	{
		printf("1: Open\n");
		printf("2: Read\n");
		printf("3: Write\n");
		printf("4: Close\n");
		printf("5: Seek\n");
#ifdef MEM_IOCTL
		printf("6: Store Size Get\n");
		printf("7: Store Size Set\n");
#endif
		printf("0: Exit\n");
		printf("Enter choice: ");
		scanf("%d%*c", &choice);
		switch (choice)
		{
			case 1:
				fd = open(file_name, O_RDWR);
				if (fd == -1)
				{
					perror("file_ops open");
				}
				break;
			case 2:
				if (fd == -1)
				{
					printf("File %s not open\n", file_name);
					break;
				}
				printf("Bytes to read (< %d): ", BUF_SIZE);
				scanf("%d%*c", &cnt);
				cnt = read(fd, data, cnt);
				if (cnt == -1)
				{
					perror("file_ops read");
					break;
				}
				printf("Read %d bytes @ %p: ", cnt, data);
				for (i = 0; i < cnt; i++)
				{
					printf("%c (%02X) ", data[i], data[i]);
				}
				printf("\n");
				break;
			case 3:
				if (fd == -1)
				{
					printf("File %s not open\n", file_name);
					break;
				}
				printf("Enter your string of length < %d bytes: ", BUF_SIZE);
				scanf("%[^\n]%*c", data);
				cnt = write(fd, data, strlen(data));
				if (cnt == -1)
				{
					perror("file_ops write");
					break;
				}
				printf("Wrote %d bytes from %p\n", cnt, data);
				break;
			case 4:
				if (fd == -1)
				{
					printf("File %s not open\n", file_name);
					break;
				}
				close (fd);
				fd = -1;
				break;
			case 5:
				if (fd == -1)
				{
					printf("File %s not open\n", file_name);
					break;
				}
				printf("Enter the seek from start of device file: ");
				scanf("%lld%*c", &seek);
				if (lseek(fd, seek, SEEK_SET) == -1)
				{
					perror("file_ops lseek");
					break;
				}
				break;
#ifdef MEM_IOCTL
			case 6:
				if (fd == -1)
				{
					printf("File %s not open\n", file_name);
					break;
				}
				if (ioctl(fd, 0 /* TODO 1: Fill in the command for getting the store size */, &size) == -1)
				{
					perror("file_ops ioctl");
					break;
				}
				printf("Current store size: %d\n", size);
				break;
			case 7:
				if (fd == -1)
				{
					printf("File %s not open\n", file_name);
					break;
				}
				printf("Enter the store size: ");
				scanf("%d%*c", &size);
				if (ioctl(fd, 0 /* TODO 2: Fill in the command for setting the store size */, size) == -1)
				{
					perror("file_ops ioctl");
					break;
				}
				break;
#endif
		}
	} while (choice != 0);

	return 0;
}

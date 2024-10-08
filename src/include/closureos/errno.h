/* SPDX-License-Identifier: GPL-2.0 */

#ifndef CLOSUREOS_ERRNO_H
#define CLOSUREOS_ERRNO_H

/* as 0xFFFFFFFFFFFFF000 should not be a valid address, use last 0xFFF as err */
#define MAX_ERRNO   4095

#define EPERM       1   /* operation not permitted */
#define ENOENT      2   /* no such file or directory */
#define ESRCH       3   /* no such process */
#define EINTR       4   /* interuppted system call */
#define EIO         5   /* I/O error */
#define ENXIO       6   /* no such device/address */
#define E2BIG       7   /* argument list too big */
#define ENOEXEC     8   /* executable format error */
#define EBADF       9   /* bad file numer */
#define ECHILD      10  /* no child process */
#define EAGAIN      11  /* try again */
#define ENOMEM      12  /* no memory available */
#define EACCES      13  /* permission denied */
#define EFAULT      14  /* invalid address */
#define ENOTBLK     15  /* not a block device */
#define EBUSY       16  /* device/resource busy */
#define EEXIST      17  /* file existed */
#define EXDEV       18  /* cross-device link */
#define ENODEV      19  /* no such device */
#define ENOTDIR     20  /* not a directory */
#define EISDIR      21  /* is a directory */
#define EINVAL      22  /* invalid argument */
#define ENFILE      23  /* file table overflow */
#define EMFILE      24  /* too many opened files */
#define ENOTTTY     25  /* not a tty */
#define ETXTBUSY    26  /* text file busy */
#define EFBIG       27  /* file too big */
#define ENOSPEC     28  /* no space left on the device */
#define ESPIPE      29  /* illegal seek */
#define EROFS       30  /* read-only file system */
#define EMLINK      31  /* too many link */
#define EPIPE       32  /* broken pipe */
#define EDOM        33  /* math argument out of the domain of the func */
#define ERANGE      34  /* math result is out of representable range */

#endif // CLOSUREOS_ERRNO_H

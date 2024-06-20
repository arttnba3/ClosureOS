#ifndef CLOSUREOS_ERRNO_H
#define CLOSUREOS_ERRNO_H

/* as 0xFFFFFFFFFFFFF000 should not be a valid address, use last 0xFFF as err */
#define MAX_ERRNO   4095

#define ENOMEM      12  /* no memory available */
#define EFAULT      14  /* invalid address */

#endif // CLOSUREOS_ERRNO_H

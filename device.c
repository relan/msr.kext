/*
 * Copyright (C) 2015  Andrew Nayenko
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <sys/conf.h>
#include <sys/fcntl.h>
#include <sys/proc.h>
#include <miscfs/devfs/devfs.h>
#include <i386/proc_reg.h>

static const char *device_name = "msr";
static int device_major;
static void *device_handle;

static int msr_open(dev_t dev, int flags, int type, struct proc *p)
{
	return KERN_SUCCESS;
}

static int msr_close(dev_t dev, int flags, int type, struct proc *p)
{
	return KERN_SUCCESS;
}

static int msr_read_safe(unsigned index, uint64_t *value)
{
	uint32_t lo, hi;

	if (rdmsr_carefully(index, &lo, &hi) != 0)
		return KERN_INVALID_ARGUMENT;
	*value = lo | ((uint64_t) hi << 32);
	return KERN_SUCCESS;
}

static int msr_read(dev_t dev, uio_t uio, int ioflag)
{
	while (uio_resid(uio) > 0) {
		user_addr_t ioaddr = uio_curriovbase(uio);
		user_size_t iosize = uio_curriovlen(uio);
		uint64_t value;

		if (iosize != sizeof(value))
			return KERN_INVALID_ARGUMENT;
		if (msr_read_safe((unsigned) uio_offset(uio), &value) != 0)
			return KERN_INVALID_ARGUMENT;
		copyout(&value, ioaddr, sizeof(value));
		uio_update(uio, sizeof(value));
	}

	return KERN_SUCCESS;
}

static struct cdevsw device_fops = {
	.d_open  = msr_open,
	.d_close = msr_close,
	.d_read  = msr_read,
};

kern_return_t msr_start(kmod_info_t *ki, void *d);
kern_return_t msr_start(kmod_info_t *ki, void *d)
{
	device_major = cdevsw_add(-1, &device_fops);
	if (device_major < 0) {
		printf("cdevsw_add failed\n");
		return KERN_FAILURE;
	}
	device_handle = devfs_make_node(makedev(device_major, 0), DEVFS_CHAR,
			0, 0, 0660, "%s", device_name);
	if (device_handle == NULL) {
		printf("devfs_make_node failed\n");
		return KERN_FAILURE;
	}
	return KERN_SUCCESS;
}

kern_return_t msr_stop(kmod_info_t *ki, void *d);
kern_return_t msr_stop(kmod_info_t *ki, void *d)
{
	devfs_remove(device_handle);
	cdevsw_remove(device_major, &device_fops);
	return KERN_SUCCESS;
}

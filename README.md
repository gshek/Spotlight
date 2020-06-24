# Spotlight

A framework that mounts compressed disk images.

Using [FUSE](https://www.kernel.org/doc/Documentation/filesystems/fuse.txt)
(Filesystem in Userspace), this framework allows for compressed disk images to
be mounted in read-only mode, without the need for full decompression. The
mounted disk image can then be traversed and files read. The aim of this
framework is to provide these features with a small performance cost.

### Supported File Systems

* [ext4](https://wiki.archlinux.org/index.php/ext4)

### Supported Compression Formats

* [gzip](https://www.gnu.org/software/gzip/)
* [blocked xz](https://tukaani.org/xz/format.html)

# Usage

## Runtime Requirements (for Linux distributions based on Debian)

* fuse

## Mount Disk Image

```
usage: ./spotlight disk mountpoint [options]

general options:
    -o opt,[opt...]        mount options
    -h   --help            print help
    -V   --version         print version

FUSE options:
    -d   -o debug          enable debug output (implies -f)
    -f                     foreground operation
    -s                     disable multi-threaded operation

    -o allow_other         allow access to other users
    -o allow_root          allow access to root
    -o auto_unmount        auto unmount on process termination
    -o nonempty            allow mounts over non-empty file/dir
    -o default_permissions enable permission checking by kernel
    -o fsname=NAME         set filesystem name
    -o subtype=NAME        set filesystem type
    -o large_read          issue large read requests (2.4 only)
    -o max_read=N          set maximum size of read requests

    -o hard_remove         immediate removal (don't hide files)
    -o use_ino             let filesystem set inode numbers
    -o readdir_ino         try to fill in d_ino in readdir
    -o direct_io           use direct I/O
    -o kernel_cache        cache files in kernel
    -o [no]auto_cache      enable caching based on modification times (off)
    -o umask=M             set file permissions (octal)
    -o uid=N               set file owner
    -o gid=N               set file group
    -o entry_timeout=T     cache timeout for names (1.0s)
    -o negative_timeout=T  cache timeout for deleted names (0.0s)
    -o attr_timeout=T      cache timeout for attributes (1.0s)
    -o ac_attr_timeout=T   auto cache timeout for attributes (attr_timeout)
    -o noforget            never forget cached inodes
    -o remember=T          remember cached inodes for T seconds (0s)
    -o nopath              don't supply path if not necessary
    -o intr                allow requests to be interrupted
    -o intr_signal=NUM     signal to send on interrupt (10)
    -o modules=M1[:M2...]  names of modules to push onto filesystem stack

    -o max_write=N         set maximum size of write requests
    -o max_readahead=N     set maximum readahead
    -o max_background=N    set number of maximum background requests
    -o congestion_threshold=N  set kernel's congestion threshold
    -o async_read          perform reads asynchronously (default)
    -o sync_read           perform reads synchronously
    -o atomic_o_trunc      enable atomic open+truncate support
    -o big_writes          enable larger than 4kB writes
    -o no_remote_lock      disable remote file locking
    -o no_remote_flock     disable remote file locking (BSD)
    -o no_remote_posix_lock disable remove file locking (POSIX)
    -o [no_]splice_write   use splice to write to the fuse device
    -o [no_]splice_move    move data while splicing to the fuse device
    -o [no_]splice_read    use splice to read from the fuse device

Module options:

[iconv]
    -o from_code=CHARSET   original encoding of file names (default: UTF-8)
    -o to_code=CHARSET	    new encoding of the file names (default: UTF-8)

[subdir]
    -o subdir=DIR	    prepend this directory to all paths (mandatory)
    -o [no]rellinks	    transform absolute symlinks to relative
```

## Unmount Disk Image

Use `fusermount` and the `-u` flag to unmount disk images.

```
fusermount: [options] mountpoint
Options:
 -h		    print help
 -V		    print version
 -o opt[,opt...]   mount options
 -u		    unmount
 -q		    quiet
 -z		    lazy unmount
```

## Examples

```bash
# Suppose disk image and mount directory exists
disk_image="disk.img"
mount_point="mountpoint/"

# Mount, and spotlight will fork and exit
./spotlight "$disk_image" "$mount_point"

# Unmount
fusermount -u "$mount_point"


# Mount, and print debug log to stdout while spotlight blocks
./spotlight "$disk_image" "$mount_point" -o debug

# Unmount
fusermount -u "$mount_point"
```

# Getting Started with Development

These instructions will get you a copy of the framework up and running on your
machine.

## Build Requirements (for Linux distributions based on Debian)

* build-essential
* pkg-config
* libfuse-dev
* liblzma-dev

## Building

Traverse into `src` directory
```bash
cd src
```

Then build `spotlight` and `compression-reader`
```bash
make
```

### Building with debug flags

```bash
make debug
```

# Running Tests

Traverse into `tests` directory
```bash
cd tests
```

Then run tests
```bash
make
```

# Built With

* [ext4fuse](https://github.com/gerard/ext4fuse) - ext4 FUSE implementation.
* [zlib-ng](https://github.com/zlib-ng/zlib-ng) - zlib replacement with
optimisations.

# License

This project is licenses under the GPL-3.0 License - see the [LICENSE](LICENSE)
file for details.

# Acknowledgements

* [nbdkit](https://github.com/libguestfs/nbdkit) - for blocked xz reading.
* [README-Template.md](https://gist.github.com/PurpleBooth/109311bb0361f32d87a2) -
for README template.
* [zran.c](https://github.com/madler/zlib/blob/master/examples/zran.c) - for
gzip reading.

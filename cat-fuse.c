/*
 * =====================================================================================
 *
 *       Filename:  fuse-concat.c
 *
 *        Version:  0.0.1
 *    Description:  concat regular files dynamically into one virtual file
 *        Created:  08/19/2018 02:06:57 AM
 *       Compiler:  gcc
 *
 *         Author:  Jakub Nowak (MrQubo)
 *
 * =====================================================================================
 */

#define FUSE_USE_VERSION 30

#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


static char const * const * files;
static size_t files_size;


#define RETURN_ERRNO do {  \
    int const err = errno; \
    perror("\t");          \
    return err;            \
} while(0)

#define CLOSE_AND_RETURN_ERRNO do { \
    int const err = errno;          \
    perror("\t");                   \
    if (close(fd) < 0) {            \
        perror("\t");               \
    }                               \
    return -err;                    \
} while(0)


static int do_getattr(char const * const path, struct stat * const st) {
    printf("[getattr] Called\n");
    printf("\tAttributes of '%s' requested\n", path);


    if (strcmp(path, "/") != 0) {
        errno = ENOENT;
        RETURN_ERRNO;
    }


    time_t atime = 0;
    time_t mtime = 0;
    time_t ctime = 0;
    off_t size = 0;

    for (size_t i = 0; i < files_size; i += 1) {
        struct stat fst;
        stat(files[i], &fst);
        if (fst.st_atime > atime) { atime = fst.st_atime; }
        if (fst.st_mtime > mtime) { mtime = fst.st_mtime; }
        if (fst.st_ctime > ctime) { ctime = fst.st_ctime; }
        size += fst.st_size;
    }


    st->st_atime = atime;
    st->st_mtime = mtime;
    st->st_ctime = ctime;
    st->st_size = size;

    st->st_uid = getuid();
    st->st_gid = getgid();

    st->st_mode = S_IFREG | 0444;
    st->st_nlink = files_size;


    return 0;
}


static int read_concat(size_t const file_no, char * const buffer, size_t const size, off_t const offset) {
    if (size == 0) {
        return 0;
    }
    if (file_no >= files_size) {
        printf("\tReached EOF.\n");
        return 0;
    }


    printf("\tfile: '%s', size: %lu, offset: %ld\n", files[file_no], size, offset);


    int const fd = open(files[file_no], O_RDONLY);
    if (fd < 0) { RETURN_ERRNO; }

    off_t file_sz = -1;

    if (offset > 0) {
        file_sz = lseek(fd, 0L, SEEK_END);
        if (file_sz < 0) { CLOSE_AND_RETURN_ERRNO; }
    }

    ssize_t bytes_read = 0;

    if (offset == 0 || file_sz > offset) {
        if (lseek(fd, offset, SEEK_SET) < 0) { CLOSE_AND_RETURN_ERRNO; }

        bytes_read = read(fd, buffer, size);
        if (bytes_read < 0) { CLOSE_AND_RETURN_ERRNO; }
        if (bytes_read > INT_MAX) {
            errno = EOVERFLOW;
            CLOSE_AND_RETURN_ERRNO;
        }
    }

    if (close(fd) < 0) { RETURN_ERRNO; }


    int bytes_read_next = 0;

    if ((size_t)bytes_read < size) {
        bytes_read_next = read_concat(file_no + 1, buffer + bytes_read, size - (size_t)bytes_read, file_sz > offset ? offset-file_sz : 0L);
        if (bytes_read_next < 0) { return bytes_read_next; }
    }

    ssize_t const bytes_read_sum = bytes_read + bytes_read_next;

    if (bytes_read_sum > INT_MAX) {
        errno = EOVERFLOW;
        RETURN_ERRNO;
    }

    return (int)bytes_read_sum;
}

static int do_read(
    char const * const path,
    char * const buffer,
    size_t const size,
    off_t const offset,
    struct fuse_file_info * const fi __attribute__((unused))
) {
    printf("[read] Called\n");
    printf("\tTrying to read '%s', size: %lu, offset: %ld\n", path, size, offset);


    if (offset < 0) {
        errno = EINVAL;
        RETURN_ERRNO;
    }

    if (strcmp(path, "/") != 0) {
        errno = ENOENT;
        RETURN_ERRNO;
    }


    return read_concat(0L, buffer, size, offset);
}

static struct fuse_operations operations = {
    .getattr = do_getattr,
    .read    = do_read,
};

static void usage(char const * const argv0, char const * const msg) {
    printf("%s\n\nUsage: %s [FILES] -- [FUSE_OPTIONS]\n\nAll flags (except '-i') and everything after '--' is passed to fuse.\nUse '-i' if you want to include filename starting with dash.\n", msg, argv0);
}

__attribute__((const))
int main(int argc, char * const argv[const]) {
    int new_argc = 1;
    char * * new_argv;
    files_size = 0;
    char const * * _files;

    for (int i = 1; i < argc; i += 1) {
        if (strcmp(argv[i], "-i") == 0) {
            files_size += 1;
            i += 1;
        } else if (strcmp(argv[i], "--") == 0) {
            new_argc += argc - i - 1;
            break;
        } else if (argv[i][0] == '-') {
            new_argc += 1;
        } else {
            files_size += 1;
        }
    }

    if (files_size < 2) {
        usage(argv[0], "You should include at least 2 files.");
        exit(1);
    }


    _files = malloc(files_size * sizeof(char *));
    if (_files == NULL) { RETURN_ERRNO; }

    new_argv = malloc((size_t)new_argc * sizeof(char *));
    if (new_argv == NULL) { RETURN_ERRNO; }

    new_argv[0] = argv[0];


    int ii = 1, fo = 0, ao = 1;
    for (; ii < argc; ii += 1) {
        if (strcmp(argv[ii], "-i") == 0) {
            if (ii+1 >= argc) {
                usage(argv[0], "Missing argument to '-i'.");
            }
            _files[fo++] = argv[ii += 1];
        } else if (strcmp(argv[ii], "--") == 0) {
            ii += 1;
            break;
        } else if (argv[ii][0] == '-') {
            new_argv[ao++] = argv[ii];
        } else {
            _files[fo++] = argv[ii];
        }
    }

    for (; ii < argc; ii += 1) {
        new_argv[ao++] = argv[ii];
    }


    // resolve absolute paths
    for (size_t i = 0; i < files_size; i += 1) {
        _files[i] = (char const *)realpath(_files[i], NULL);
        if (_files[i] == NULL) { RETURN_ERRNO; }
    }


    files = _files;


    int const res = fuse_main(new_argc, new_argv, &operations, NULL);


    return res;
}

# cat-fuse
Concat regular files dynamically into one virtual file.

## Build
This project uses CMake as it's build system.
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Install
To install in standard CMake location:
```bash
make install
```

If you want to install in different location change the value of `CMAKE_INSTALL_PREFIX`, e.g.
```bash
cmake -DCMAKE_INSTALL_PREFIX="~/.local" ..
make install
```

### Uninstall
To uninstall remove files listed in `build/install_manifest.txt`.

## Usage
```
cat-fuse [FILES] [-i FILE] [FUSE_FLAGS] -- FUSE
```

All flags before `--` (except `-i`) are passed to fuse.
If you want to include file starting with dash use `-i` flag.
All arguments after `--` are passed to fuse.

Use `fusermount -u` to unmount the file.

The content of included files is read per request so changing content of any of the files causes the content of virtual file to change as well.

### Example usage
```bash
echo Hello Fuse! >file1
echo 0123456789  >file2
echo 9876543210  >file3
touch file1+2+3
cat-fuse file1 file2 file3 -- file1+2+3
cat file1+2+3
```

## Todo
Cache content and size of files and invalidate the cache if mtime has changed.

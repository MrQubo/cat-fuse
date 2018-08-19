cmake_minimum_required(VERSION 3.0)
project(cat-fuse C)


find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE REQUIRED fuse)


set(WARNING_C_FLAGS ${WARNING_C_FLAGS}
  -Werror
  -fmax-errors=8
  -pedantic
  -pedantic-errors
  -Wall
  -Walloc-zero
  -Walloca-larger-than=256
  -Warray-bounds=2
  -Wattribute-alias
  -Wcast-align
  -Wcast-qual
  -Wconversion
  -Wdate-time
  -Wdisabled-optimization
  -Wdouble-promotion
  -Wduplicated-branches
  -Wduplicated-cond
  -Wextra
  -Wfloat-equal
  -Wformat-overflow=2
  -Wformat-signedness
  -Wformat-truncation=2
  -Wformat=2
  -Whsa
  -Wimplicit-fallthrough=5
  -Wimport
  -Winit-self
  -Winvalid-pch
  -Wlogical-op
  -Wmissing-declarations
  -Wmissing-field-initializers
  -Wmissing-format-attribute
  -Wmissing-include-dirs
  -Wmissing-noreturn
  -Wno-multichar
  -Wno-restrict
  -Wnull-dereference
  -Wpacked
  -Wpadded
  -Wpragmas
  -Wredundant-decls
  -Wshadow
  -Wshift-negative-value
  -Wshift-overflow=2
  -Wsign-conversion
  -Wsizeof-array-argument
  -Wstack-protector
  -Wstrict-aliasing=1
  -Wstrict-overflow=5
  -Wstringop-overflow=3
  -Wstringop-overflow=4
  -Wstringop-truncation
  -Wsuggest-attribute=cold
  -Wsuggest-attribute=const
  -Wsuggest-attribute=format
  -Wsuggest-attribute=malloc
  -Wsuggest-attribute=noreturn
  -Wsuggest-attribute=pure
  -Wsuggest-final-methods
  -Wsuggest-final-types
  -Wswitch-default
  -Wsync-nand
  -Wtrampolines
  -Wundef
  -Wuninitialized
  -Wunknown-pragmas
  -Wunreachable-code
  -Wunused
  -Wunused-macros
  -Wunused-parameter
  -Wvla
  -Wvla-larger-than=256
  -Wwrite-strings
  -Wbad-function-cast
  -Wjump-misses-init
  -Wmissing-prototypes
  -Wnested-externs
  -Wold-style-declaration
  -Wold-style-definition
  -Wstrict-prototypes
  -Wtraditional-conversion
  -Wunsuffixed-float-constants)

set(WARNING_C_FLAGS ${WARNING_C_FLAGS}
  -Werror
  -pedantic-errors)


add_executable(cat-fuse cat-fuse.c)
target_include_directories(cat-fuse SYSTEM PUBLIC ${FUSE_INCLUDE_DIRS})
target_link_libraries(cat-fuse ${FUSE_LIBRARIES})
target_compile_options(cat-fuse PUBLIC ${FUSE_CFLAGS_OTHER} ${WARNING_C_FLAGS})


install(TARGETS cat-fuse
  RUNTIME DESTINATION bin)

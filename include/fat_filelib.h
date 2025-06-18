#ifndef __FAT_FILELIB_H__
#define __FAT_FILELIB_H__

#include "kernel.h"
#include "syscall.h"



//-------------------------------------------------------------
// Configuration
//-------------------------------------------------------------

// Is the processor little endian (1) or big endian (0)
#ifndef FATFS_IS_LITTLE_ENDIAN
    #define FATFS_IS_LITTLE_ENDIAN          1
#endif

// Max filename Length
#ifndef FATFS_MAX_LONG_FILENAME
    #define FATFS_MAX_LONG_FILENAME         64
#endif

// Max open files (reduce to lower memory requirements)
#ifndef FATFS_MAX_OPEN_FILES
    #define FATFS_MAX_OPEN_FILES            3
#endif

// Number of sectors per FAT_BUFFER (min 1)
#ifndef FAT_BUFFER_SECTORS
    #define FAT_BUFFER_SECTORS              1
#endif

// Max FAT sectors to buffer (min 1)
// (mem used is FAT_BUFFERS * FAT_BUFFER_SECTORS * FAT_SECTOR_SIZE)
#ifndef FAT_BUFFERS
    #define FAT_BUFFERS                     1
#endif

// Size of cluster chain cache (can be undefined)
// Mem used = FAT_CLUSTER_CACHE_ENTRIES * 4 * 2
// Improves access speed considerably
//#define FAT_CLUSTER_CACHE_ENTRIES         128

// Include support for writing files (1 / 0)?
#ifndef FATFS_INC_WRITE_SUPPORT
    #define FATFS_INC_WRITE_SUPPORT         1
#endif

// Support long filenames (1 / 0)?
// (if not (0) only 8.3 format is supported)
#ifndef FATFS_INC_LFN_SUPPORT
    #define FATFS_INC_LFN_SUPPORT           1
#endif

// Support directory listing (1 / 0)?
#ifndef FATFS_DIR_LIST_SUPPORT
    #define FATFS_DIR_LIST_SUPPORT          1
#endif

// Support time/date (1 / 0)?
#ifndef FATFS_INC_TIME_DATE_SUPPORT
    #define FATFS_INC_TIME_DATE_SUPPORT     0
#endif

// Include support for formatting disks (1 / 0)?
#ifndef FATFS_INC_FORMAT_SUPPORT
    #define FATFS_INC_FORMAT_SUPPORT        1
#endif

// Sector size used
#define FAT_SECTOR_SIZE                     512


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifndef SEEK_CUR
    #define SEEK_CUR    1
#endif

#ifndef SEEK_END
    #define SEEK_END    2
#endif

#ifndef SEEK_SET
    #define SEEK_SET    0
#endif

#ifndef EOF
    #define EOF         (-1)
#endif

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct sFL_FILE;

struct cluster_lookup
{
    uint32 ClusterIdx;
    uint32 CurrentCluster;
};


struct fat_node
{
    struct fat_node    *previous;
    struct fat_node    *next;
};

typedef struct sFL_FILE
{
    uint32                  parentcluster;
    uint32                  startcluster;
    uint32                  bytenum;
    uint32                  filelength;
    int                     filelength_changed;
    char                    path[FATFS_MAX_LONG_FILENAME];
    char                    filename[FATFS_MAX_LONG_FILENAME];
    uint8                   shortfilename[11];

#ifdef FAT_CLUSTER_CACHE_ENTRIES
    uint32                  cluster_cache_idx[FAT_CLUSTER_CACHE_ENTRIES];
    uint32                  cluster_cache_data[FAT_CLUSTER_CACHE_ENTRIES];
#endif

    // Cluster Lookup
    struct cluster_lookup   last_fat_lookup;

    // Read/Write sector buffer
    uint8                   file_data_sector[FAT_SECTOR_SIZE];
    uint32                  file_data_address;
    int                     file_data_dirty;

    // File fopen flags
    uint8                   flags;
#define FILE_READ           (1 << 0)
#define FILE_WRITE          (1 << 1)
#define FILE_APPEND         (1 << 2)
#define FILE_BINARY         (1 << 3)
#define FILE_ERASE          (1 << 4)
#define FILE_CREATE         (1 << 5)

    struct fat_node         list_node;
} FL_FILE;




//-----------------------------------------------------------------------------
// Stdio file I/O names
//-----------------------------------------------------------------------------
//#ifdef USE_FILELIB_STDIO_COMPAT_NAMES

#define FILE            FL_FILE

#define fopen(a,b)      __syscall(0xcafe0001,a,b)
#define fclose(a)       __syscall(0xcafe0002,a)
#define fflush(a)       __syscall(0xcafe0003,a)
#define fgetc(a)        __syscall(0xcafe0004,a)
#define fgets(a,b,c)    __syscall(0xcafe0005,a,b,c)
#define fputc(a,b)      __syscall(0xcafe0006,a,b)
#define fputs(a,b)      __syscall(0xcafe0007,a,b)
#define fwrite(a,b,c,d) __syscall(0xcafe0008,a,b,c,d)
#define fread(a,b,c,d)  __syscall(0xcafe0009,a,b,c,d)
#define fseek(a,b,c)    __syscall(0xcafe0010,a,b,c)
#define fgetpos(a,b)    __syscall(0xcafe0011,a,b)
#define ftell(a)        __syscall(0xcafe0012,a)
#define feof(a)         __syscall(0xcafe0013,a)
#define remove(a)       __syscall(0xcafe0014,a)
#define mkdir(a)        __syscall(0xcafe0015,a)
#define rmdir(a)        0

//#endif


enum {
    SYS_FOPEN = 0xcafe0001,
    SYS_FCLOSE = 0xcafe0002,
    SYS_FFLUSH = 0xcafe0003,
    SYS_FGETC = 0xcafe0004,
    SYS_FGETS = 0xcafe0005,
    SYS_FPUTC = 0xcafe0006,
    SYS_FPUTS = 0xcafe0007,
    SYS_FWRITE = 0xcafe0008,
    SYS_FREAD = 0xcafe0009,
    SYS_FSEEK = 0xcafe0010,
    SYS_FGETPOS = 0xcafe0011,
    SYS_FTELL = 0xcafe0012,
    SYS_FEOF = 0xcafe0013,
    SYS_REMOVE = 0xcafe0014,
    SYS_MKDIR = 0xcafe0015
};


#endif

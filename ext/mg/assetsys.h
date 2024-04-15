/*
------------------------------------------------------------------------------
          Licensing information can be found at the end of the file.
------------------------------------------------------------------------------

assetsys.h - v1.5 - File system abstraction to read from zip-files, for C/C++.

Do this:
    #define ASSETSYS_IMPLEMENTATION
before you include this file in *one* C/C++ file to create the implementation.

If you are using miniz.c for other things, and have included it in your 
project, you need to define ASSETSYS_NO_MINIZ as well, to avoid duplicate 
definitions. 

Dependencies: 
    strpool.h
*/

#ifndef assetsys_h
#define assetsys_h

// OpenJazz Modifications
#include <miniz.h>
#include <cstdint>
#define ASSETSYS_U64 uint64_t
// End of Openjazz Modifications

#ifndef ASSETSYS_U64
    #define ASSETSYS_U64 unsigned long long
#endif

typedef enum assetsys_error_t
    {
    ASSETSYS_SUCCESS = 0,
    ASSETSYS_ERROR_INVALID_PATH = -1,
    ASSETSYS_ERROR_INVALID_MOUNT = -2, 
    ASSETSYS_ERROR_FAILED_TO_READ_ZIP = -3,
    ASSETSYS_ERROR_FAILED_TO_CLOSE_ZIP = -4,
    ASSETSYS_ERROR_FAILED_TO_READ_FILE = -5,
    ASSETSYS_ERROR_FILE_NOT_FOUND = -6,
    ASSETSYS_ERROR_DIR_NOT_FOUND = -7, 
    ASSETSYS_ERROR_INVALID_PARAMETER = -8,
    ASSETSYS_ERROR_BUFFER_TOO_SMALL = -9,
    } assetsys_error_t;

typedef struct assetsys_t assetsys_t;

assetsys_t* assetsys_create( void* memctx );
void assetsys_destroy( assetsys_t* sys );

assetsys_error_t assetsys_mount( assetsys_t* sys, char const* path, char const* mount_as );
assetsys_error_t assetsys_mount_from_memory( assetsys_t* sys, void const* data, int size, char const* mount_as);
assetsys_error_t assetsys_dismount( assetsys_t* sys, char const* path, char const* mounted_as );

typedef struct assetsys_file_t { ASSETSYS_U64 mount; ASSETSYS_U64 path; int index; } assetsys_file_t;

assetsys_error_t assetsys_file( assetsys_t* sys, char const* path, assetsys_file_t* file );
assetsys_error_t assetsys_file_load( assetsys_t* sys, assetsys_file_t file, int* size, void* buffer, int capacity );
int assetsys_file_size( assetsys_t* sys, assetsys_file_t file );

int assetsys_file_count( assetsys_t* sys, char const* path );
char const* assetsys_file_name( assetsys_t* sys, char const* path, int index );
char const* assetsys_file_path( assetsys_t* sys, char const* path, int index );

int assetsys_subdir_count( assetsys_t* sys, char const* path );
char const* assetsys_subdir_name( assetsys_t* sys, char const* path, int index );
char const* assetsys_subdir_path( assetsys_t* sys, char const* path, int index );

// OpenJazz addition
const char *assetsys_file_to_path(assetsys_t* sys, assetsys_file_t file);

#endif /* assetsys_h */

/**

assetsys.h
==========

File system abstraction to read from zip-files, for C/C++.


Example
-------

    #define ASSETSYS_IMPLEMENTATION
    #include "assetsys.h"

    #define STRPOOL_IMPLEMENTATION
    #include "strpool.h"

    #include <stdio.h> // for printf

    void list_assets( assetsys_t* assetsys, char const* path, int indent ) {
        // Print folder names and recursively list assets
        for( int i = 0; i < assetsys_subdir_count( assetsys, path ); ++i ) {
            char const* subdir_name = assetsys_subdir_name( assetsys, path, i );
            for( int j = 0; j < indent; ++j ) printf( "  " );
            printf( "%s/\n", subdir_name );

            char const* subdir_path = assetsys_subdir_path( assetsys, path, i );
            list_assets( assetsys, subdir_path, indent + 1 );
        }

        // Print file names
        for( int i = 0; i < assetsys_file_count( assetsys, path ); ++i ) {
            char const* file_name = assetsys_file_name( assetsys, path, i );
            for( int j = 0; j < indent; ++j ) printf( "  " );
            printf( "%s\n", file_name );
        }
    }

    int main( void ) {
        assetsys_t* assetsys = assetsys_create( 0 );
    
        // Mount current working folder as a virtual "/data" path
        assetsys_mount( assetsys, ".", "/data" );

        // Print all files and subfolders
        list_assets( assetsys, "/", 0 ); // Start at root 

        // Load a file
        assetsys_file_t file;
        assetsys_file( assetsys, "/data/readme.txt", &file );
        int size = assetsys_file_size( assetsys, file );
        char* content = (char*) malloc( size + 1 ); // extra space for '\0'
        int loaded_size = 0;
        assetsys_file_load( assetsys, file, &loaded_size, content, size );
        content[ size ] = '\0'; // zero terminate the text file
        printf( "%s\n", content );
        free( content );

        assetsys_destroy( assetsys );
    }
    

API Documentation
-----------------

assetsys.h is a system for loading binary assets into your game. It allows you to mount directories and archive files 
(bundles of files; assetsys.h supports using standard zip files for this) assign them a virtual path. You then load 
assets through assetsys using the virtual path (which can stay the same even if the mounts change). assetsys.h is 
case-insensitive regardless of platform, and only accepts forward slash "/" as path separator, to ensure consistent 
behavior. It allows you to mount several paths or archive files to the same virtual path, even if they contain files of 
the same name, and a later mount will take precedence over a previous one. This allows you to, for example, have a 
`data` archive file and an `update` archive file, where `update` contains new versions of only some of the files.

assetsys.h is a single-header library, and does not need any .lib files or other binaries, or any build scripts. To use 
it, you just include assetsys.h to get the API declarations. To get the definitions, you must include assetsys.h from 
*one* single C or C++ file, and #define the symbol `ASSETSYS_IMPLEMENTATION` before you do. 

assetsys.h has a dependency om another single-header library, strpool.h, which is used for efficient storage and 
comparison of the filename and path strings. assetsys.h automatically includes strpool.h, so it must reside in the same
path. It does not specify the STRPOOL_IMPLEMENTATION define, on the assumption that you might be including strpool.h in
some other part of your program. If you are not, you can make assetsys.h include the strpool implemention by doing:

    #define ASSETSYS_IMPLEMENTATION
    #define STRPOOL_IMPLEMENTATION
    #include "assetsys.h"


### Customization

There are a few different things in assetsys.h which are configurable by #defines. Most of the API use the `int` data 
type, for integer values where the exact size is not important. However, for some functions, it specifically makes use 
of an unsigned 64 bit data types. It default to using `unsigned long long`, but can be redefined by #defining 
ASSETSYS_U64, before including assetsys.h. This is useful if you, for example, use the types from `<stdint.h>` in the 
rest of your program, and you want assetsys.h to use compatible types. In this case, you would include assetsys.h using 
the following code:

    #define ASSETSYS_U64 uint64_t
    #include "assetsys.h"

Note that when customizing the data type, you need to use the same definition in every place where you include 
assetsys.h, as they affect the declarations as well as the definitions.

The rest of the customizations only affect the implementation, so will only need to be defined in the file where you
have the #define ASSETSYS_IMPLEMENTATION.


#### Custom memory allocators

To store the internal data structures, ini.h needs to do dynamic allocation by calling `malloc`. Programs might want to 
keep track of allocations done, or use custom defined pools to allocate memory from. assetsys.h allows for specifying 
custom memory allocation functions for `malloc` and `free`. This is done with the following code:

    #define ASSETSYS_IMPLEMENTATION
    #define ASSETSYS_MALLOC( ctx, size ) ( my_custom_malloc( ctx, size ) )
    #define ASSETSYS_FREE( ctx, ptr ) ( my_custom_free( ctx, ptr ) )
    #include "assetsys.h"

where `my_custom_malloc` and `my_custom_free` are your own memory allocation/deallocation functions. The `ctx` parameter
is an optional parameter of type `void*`. When `assetsys_init` is called, you can set the `memctx` field of the `config`
parameter, to a pointer to anything you like, and which will be passed through as the `ctx` parameter to every 
`ASSETSYS_MALLOC`/`ASSETSYS_FREE` call. For example, if you are doing memory tracking, you can pass a pointer to your 
tracking data as `memctx`, and in your custom allocation/deallocation function, you can cast the `ctx` param back to the 
right type, and access the tracking data.

If no custom allocator is defined, assetsys.h will default to `malloc` and `free` from the C runtime library.


#### Custom assert

assetsys.h makes use of asserts to report usage errors and code errors. By default, it makes use of the C runtime 
library `assert` macro, which only executes in debug builds. However, it allows for substituting with your own assert 
function or macro using the following code:

    #define ASSETSYS_IMPLEMENTATION
    #define ASSETSYS_ASSERT( condition ) ( my_custom_assert( condition ) )
    #include "assetsys.h"

Note that if you only want the asserts to trigger in debug builds, you must add a check for this in your custom assert.


#### miniz implementation

assetsys.h makes use of the miniz library for parsing and decompressing zip files. It includes the entire miniz source
code inside assetsys.h, so normally you don't have to worry about it. However, in the case where you might already be
using miniz in some other part of the program, you can tell assetsys.h to not include the implementation for miniz. It 
will still include the miniz definitions, and if you don't include the miniz implementation elsewhere, you will get a 
linker error. To exclude the miniz implementation, simply define `ASSETSYS_NO_MINIZ` before including assetsys.h, like 
this:

    #define ASSETSYS_IMPLEMENTATION
    #define ASSETSYS_NO_MINIZ
    #include "assetsys.h"

    
assetsys_create
---------------

    assetsys_t* assetsys_create( void* memctx )

Creates a new assetsys instance. assetsys.h does not use any global variables, all data it needs is accessed through
the instance created by calling assetsys_create. Different instances can be used safely from different threads, but if
using the same instance from multiple threads, it is up to the user to make sure functions are not called concurrently,
for example by adding a mutex lock around each call.


assetsys_destroy
----------------

    void assetsys_destroy( assetsys_t* sys )

Destroys an assetsys instance, releasing all the resources used by it. 


assetsys_mount
--------------

    assetsys_error_t assetsys_mount( assetsys_t* sys, char const* path, char const* mount_as )

Mounts the data source `path`, making all its files accessible through this assetsys instance. The data source can be
either a folder or an archive file (a standard .zip file, with or without compression). `path` must use forward slash 
`/` as path separator, never backslash, regardless of platform. It must not end with a path separator. The string
`mount_as` will be prepended to all mounted files, and can be passed as "/" to mount as root.  `mount_as` may 
not contain the characters backslash `\` or colon `:`. It must not end with a path separator (unless consisting of 
a single path separator only, "/"). `assetsys_mount` will  return `ASSETSYS_ERROR_INVALID_PARAMETER` if either `path` 
or `mount_as` is NULL. It will return `ASSETSYS_ERROR_INVALID_PATH` if the conditions detailed above are not met, or 
if the file or folder specified by `path` could not be found. If `path` indicates a file, and it is not a valid archive
file, `assetsys_mount` returns `ASSETSYS_ERROR_FAILED_TO_READ_ZIP`.

If multiple mounts contains the same file and it is accessible through the same full path (whether because of the 
`mount_as` prefix or not), the last mounted data source will be used when loading that file.


assetsys_mount_from_memory
--------------------------

    assetsys_error_t assetsys_mount_from_memory( assetsys_t* sys, void const* data, int size, char const* mount_as )

Same as `assetsys_mount()`, but takes a data buffer of an archived *.zip* file, along with the size of the file.


assetsys_dismount
-----------------

    assetsys_error_t assetsys_dismount( assetsys_t* sys, char const* path, char const* mounted_as )

Removes a data source which was mounted by calling `assetsys_mount`. `path` and `mounted_as` must be the same as was
used when mounting. If `path` is NULL, `assetsys_dismount` returns `ASSETSYS_ERROR_INVALID_PARAMETER`. If `mounted_as`
is NULL, or no matching mount could be found, it returns `ASSETSYS_ERROR_INVALID_MOUNT`. 


assetsys_file
-------------

    assetsys_error_t assetsys_file( assetsys_t* sys, char const* path, assetsys_file_t* file )

Retrieves a handle for the file specified by `path`. `path` needs to be an absolute path, including the `mount_as` 
prefix specified when the data source was mounted, and matching is case insensitive. The mounts are searched in reverse
order they were added, and if a file with the specified path could not be found, `assetsys_file` returns
`ASSETSYS_ERROR_FILE_NOT_FOUND`. The handle is written to `file`, which must be a pointer to a `assetsys_file_t`
variable declared by the caller. The handle is used in calls to `assetsys_file_load` and `assetsys_file_size`. The 
handle is only valid until any mounts are modified by calling `assetsys_mount` or `assetsys_dismount`.


assetsys_file_load
------------------

    assetsys_error_t assetsys_file_load( assetsys_t* sys, assetsys_file_t file, int* size, void* buffer, int capacity )

Load the data from the file specified by the handle `file` (initialized by calling `assetsys_file`) and writes it into
the memory indicated by `buffer`. This memory buffer must be large enough to fit the entire file, and the `capacity`
parameter must indicate its size. To find out how large the buffer needs to be, call `assetsys_file_size`. The size of
the file will also be reported in the `size` parameter, unless it is passed in as NULL. Note that the two sizes can be
reported with different values if the file was updated on disk between the call to `assetsys_file_size` and the call to
`assetsys_file_load`. 
If the file could not be loaded, `assetsys_file_load` returns `ASSETSYS_ERROR_FAILED_TO_READ_FILE`. If the `capacity`
parameter is too small to hold the file data, `assetsys_file_load` returns `ASSETSYS_ERROR_BUFFER_TOO_SMALL`.


assetsys_file_size
------------------

    int assetsys_file_size( assetsys_t* sys, assetsys_file_t file )

Returns the size, in bytes, of the file specified by the handle `file` (initialized by calling `assetsys_file`). If the
file handle is not valid, `assetsys_file_size` returns 0. If the file is found in a directory mount, the size will be
re-queried on each call to `assetsys_file_size` (to support the case where a file have been re-saved to disk since the
last call). In the case where the file resides in an archive mount, `assetsys_file_size` will return its initial value.


assetsys_file_count
-------------------

    int assetsys_file_count( assetsys_t* sys, char const* path )

Returns the number of files in the directory with the specified path, or 0 if the path is invalid. `path` needs to be an 
absolute path, including the `mount_as` prefix specified when the data source was mounted, and matching is case 
insensitive. `assetsys_file_count` returns the total number of files from all mounts, which fall under the path.


assetsys_file_name
------------------
    
    char const* assetsys_file_name( assetsys_t* sys, char const* path, int index )

Returns the filename and extension (but not the full path) of one of the files in the specified path. `path` needs to be 
an absolute path, including the `mount_as` prefix specified when the data source was mounted, and matching is case 
insensitive. `index` needs to be between 0 and one less than the count returned by calling `assetsys_file_count` with
the same path. If the path is invalid or index is out of range, `assetsys_file_name` returns NULL.


assetsys_file_path
------------------

    char const* assetsys_file_path( assetsys_t* sys, char const* path, int index )

Returns the full path, including filename and extension, of one of the files in the specified path. `path` needs to be 
an absolute path, including the `mount_as` prefix specified when the data source was mounted, and matching is case 
insensitive. `index` needs to be between 0 and one less than the count returned by calling `assetsys_file_count` with
the same path. If the path is invalid or index is out of range, `assetsys_file_path` returns NULL.


assetsys_subdir_count
---------------------

    int assetsys_subdir_count( assetsys_t* sys, char const* path )

Returns the number of subdirectories in the directory with the specified path, or 0 if the path is invalid. `path` needs 
to be an absolute path, including the `mount_as` prefix specified when the data source was mounted, and matching is case 
insensitive. `assetsys_subdir_count` returns the total number of directories from all mounts, which fall under the path.


assetsys_subdir_name
--------------------

    char const* assetsys_subdir_name( assetsys_t* sys, char const* path, int index )

Returns the name (but not the full path) of one of the subdirectories in the specified path. `path` needs to be an 
absolute path, including the `mount_as` prefix specified when the data source was mounted, and matching is case 
insensitive. `index` needs to be between 0 and one less than the count returned by calling `assetsys_subdir_count` with
the same path. If the path is invalid or index is out of range, `assetsys_subdir_name` returns NULL.


assetsys_subdir_path
--------------------

    char const* assetsys_subdir_path( assetsys_t* sys, char const* path, int index )

Returns the name, including the full path, of one of the files in the specified path. `path` needs to be an absolute 
path, including the `mount_as` prefix specified when the data source was mounted, and matching is case insensitive. 
`index` needs to be between 0 and one less than the count returned by calling `assetsys_subdir_count` with the same 
path. If the path is invalid or index is out of range, `assetsys_subdir_path` returns NULL.


*/


// If we are running tests on windows
#if defined( ASSETSYS_RUN_TESTS ) && defined( _WIN32 ) && !defined( __TINYC__ )
    // To get file names/line numbers with meory leak detection, we need to include crtdbg.h before all other files
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif


/*
----------------------
    IMPLEMENTATION
----------------------
*/

#ifdef ASSETSYS_IMPLEMENTATION
#undef ASSETSYS_IMPLEMENTATION

#define _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS

// OpenJazz Modification
//#ifdef ASSETSYS_NO_MINIZ
    //#define MINIZ_HEADER_FILE_ONLY
//#endif /* ASSETSYS_NO_MINIZ */

#ifdef ASSETSYS_ASSERT
    #define MZ_ASSERT( x ) ASSETSYS_ASSERT( x, "Miniz assert" )
#endif /* ASSETSYS_ASSERT */

#ifdef _WIN32
    #pragma warning( push )
    #pragma warning( disable: 4619 ) // pragma warning : there is no warning number 'number'
    #pragma warning( disable: 4244 ) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
    #pragma warning( disable: 4365 ) // 'action' conversion from 'type_1' to 'type_2', signed/unsigned mismatch
    #pragma warning( disable: 4548 ) // expression before comma has no effect; expected expression with side-effect
    #pragma warning( disable: 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#endif

// OpenJazz Modification
// removed whole miniz copy

#ifdef _WIN32
    #pragma warning( pop )
#endif

#define _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS
#include <sys/stat.h>
#include <stdio.h> // FILE, fopen, fseet, SEEK_END, SEEK_SET, ftell, fread

#include "strpool.h"

#ifndef ASSETSYS_ASSERT
    #define _CRT_NONSTDC_NO_DEPRECATE 
    #define _CRT_SECURE_NO_WARNINGS
    #include <assert.h>
    #define ASSETSYS_ASSERT( expression, message ) assert( ( expression ) && ( message ) )
#endif

#ifndef ASSETSYS_MALLOC
    #define _CRT_NONSTDC_NO_DEPRECATE 
    #define _CRT_SECURE_NO_WARNINGS
    #include <stdlib.h>
    #define ASSETSYS_MALLOC( ctx, size ) ( malloc( size ) )
    #define ASSETSYS_FREE( ctx, ptr ) ( free( ptr ) )
#endif


#if defined( _WIN32 )
    #define _CRT_NONSTDC_NO_DEPRECATE 
    #define _CRT_SECURE_NO_WARNINGS
	#if !defined( _WIN32_WINNT ) || _WIN32_WINNT < 0x0501 
    #undef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501 // requires Windows XP minimum
    // 0x0400=Windows NT 4.0, 0x0500=Windows 2000, 0x0501=Windows XP, 0x0502=Windows Server 2003, 0x0600=Windows Vista, 
    // 0x0601=Windows 7, 0x0602=Windows 8, 0x0603=Windows 8.1, 0x0A00=Windows 10
#endif
    #define _WINSOCKAPI_
    #pragma warning( push )
    #pragma warning( disable: 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
    #pragma warning( disable: 4768 ) // __declspec attributes before linkage specification are ignored	
    #pragma warning( disable: 4255 ) // 'function' : no function prototype given: converting '()' to '(void)'
    #include <windows.h>
    #pragma warning( pop )

    struct assetsys_internal_dir_entry_t 
        {
        char name[ MAX_PATH ];
        BOOL is_folder;
        };


    struct assetsys_internal_dir_t
        {
        HANDLE handle;
        WIN32_FIND_DATAA data;
        struct assetsys_internal_dir_entry_t entry;
        };


    static void assetsys_internal_dir_open( struct assetsys_internal_dir_t* dir, char const* path )
        {
        size_t path_len = strlen( path );
        BOOL trailing_path_separator = path[ path_len - 1 ] == '\\' || path[ path_len - 1 ] == '/';
        const char* string_to_append = "*.*";
        if( path_len + strlen( string_to_append ) + ( trailing_path_separator ? 0 : 1 ) >= MAX_PATH ) return;
        char search_pattern[ MAX_PATH ];
        strcpy( search_pattern, path );
        if( !trailing_path_separator ) strcat( search_pattern, "\\" );
        strcat( search_pattern, string_to_append );

        WIN32_FIND_DATAA data;
        HANDLE handle = FindFirstFileA( search_pattern, &data );
        if( handle == INVALID_HANDLE_VALUE ) return;

        dir->handle = handle;
        dir->data = data;
        }


    static void assetsys_internal_dir_close( struct assetsys_internal_dir_t* dir )
        {
        if( dir->handle != INVALID_HANDLE_VALUE ) FindClose( dir->handle );
        }


    static struct assetsys_internal_dir_entry_t* assetsys_internal_dir_read( struct assetsys_internal_dir_t* dir )
        {
        if( dir->handle == INVALID_HANDLE_VALUE ) return NULL;

        strcpy( dir->entry.name, dir->data.cFileName );
        dir->entry.is_folder = ( dir->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;

        BOOL result = FindNextFileA( dir->handle, &dir->data );
        if( !result )
            {
            FindClose( dir->handle );
            dir->handle = INVALID_HANDLE_VALUE;      
            }

        return &dir->entry;    
        }


    static char const* assetsys_internal_dir_name( struct assetsys_internal_dir_entry_t* entry )
        {
        return entry->name;
        }


    static int assetsys_internal_dir_is_file( struct assetsys_internal_dir_entry_t* entry )
        {
        return entry->is_folder == FALSE;
        }


    static int assetsys_internal_dir_is_folder( struct assetsys_internal_dir_entry_t* entry )
        {
        return entry->is_folder == TRUE;
        }


#else

    #include <dirent.h>
    #include <cstring>

    struct assetsys_internal_dir_t
        {
        DIR* dir;
        };


    typedef struct assetsys_internal_dir_entry_t assetsys_internal_dir_entry_t;


    static void assetsys_internal_dir_open( struct assetsys_internal_dir_t* dir, char const* path )
        {
        dir->dir = opendir( path );
        }


    static void assetsys_internal_dir_close( struct assetsys_internal_dir_t* dir )
        {
        closedir( dir->dir );
        }


    static assetsys_internal_dir_entry_t* assetsys_internal_dir_read( struct assetsys_internal_dir_t* dir )
        {
        return (assetsys_internal_dir_entry_t*)readdir( dir->dir );
        }


    static char const* assetsys_internal_dir_name( assetsys_internal_dir_entry_t* entry )
        {
        return ( (struct dirent*)entry )->d_name;
        }


    static int assetsys_internal_dir_is_file( assetsys_internal_dir_entry_t* entry )
        {
        return ( (struct dirent*)entry )->d_type == DT_REG;
        }


    static int assetsys_internal_dir_is_folder( assetsys_internal_dir_entry_t* entry )
        {
        return ( (struct dirent*)entry )->d_type == DT_DIR;
        }

#endif 


static void* assetsys_internal_mz_alloc( void* memctx, size_t items, size_t size ) 
    { 
    (void) memctx; (void) items; (void) size; 
    ASSETSYS_U64* p = (ASSETSYS_U64*) ASSETSYS_MALLOC( memctx, ( items * size ) + sizeof( ASSETSYS_U64 ) );
    *p = ( items * size );
    return p + 1; 
    }


static void assetsys_internal_mz_free( void* memctx, void* ptr ) 
    { 
    (void) memctx; (void) ptr; 
    if( !ptr ) return;
    ASSETSYS_U64* p = ( (ASSETSYS_U64*) ptr ) - 1;
    ASSETSYS_FREE( memctx, p ); 
    }


static void* assetsys_internal_mz_realloc( void* memctx, void* ptr, size_t items, size_t size ) 
    { 
    (void) memctx; (void) ptr; (void) items; (void) size; 
    if( !ptr ) return assetsys_internal_mz_alloc( memctx, items, size );
   
    ASSETSYS_U64* p = ( (ASSETSYS_U64*) ptr ) - 1;
    ASSETSYS_U64 prev_size = *p;
    if( prev_size >= ( items * size ) ) return ptr;
    
    ASSETSYS_U64* new_ptr = (ASSETSYS_U64*) ASSETSYS_MALLOC( memctx, ( items * size ) + sizeof( ASSETSYS_U64 ) ); 
    *new_ptr = ( items * size );
    ++new_ptr;
    memcpy( new_ptr, ptr, (size_t) prev_size );
    ASSETSYS_FREE( memctx, p );
    return new_ptr;
    }


static char* assetsys_internal_dirname( char const* path );

struct assetsys_internal_file_t
    {
    int size;
    int zip_index;
    int collated_index;
    };

struct assetsys_internal_folder_t
    {
    int collated_index;
    };

enum assetsys_internal_mount_type_t
    {
    ASSETSYS_INTERNAL_MOUNT_TYPE_DIR,
    ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP,
    };

struct assetsys_internal_mount_t
    {
    ASSETSYS_U64 path;
    ASSETSYS_U64 mounted_as;
    int mount_len;
    enum assetsys_internal_mount_type_t type;
    mz_zip_archive zip;

    struct assetsys_internal_file_t* files;
    int files_count;
    int files_capacity;

    struct assetsys_internal_folder_t* dirs;
    int dirs_count;
    int dirs_capacity;
    };

struct assetsys_internal_collated_t
    {
    ASSETSYS_U64 path;
    int parent;
    int ref_count;
    int is_file;
    };


struct assetsys_t
    {
    void* memctx;
    strpool_t strpool;

    struct assetsys_internal_mount_t* mounts;
    int mounts_count;
    int mounts_capacity;

    struct assetsys_internal_collated_t* collated;
    int collated_count;
    int collated_capacity;

    char temp[ 260 ];
    };


static ASSETSYS_U64 assetsys_internal_add_string( assetsys_t* sys, char const* const str )
    {
    ASSETSYS_U64 h = strpool_inject( &sys->strpool, str, (int) strlen( str ) );
    strpool_incref( &sys->strpool, h );
    return h;
    }


static char const* assetsys_internal_get_string( assetsys_t* sys, ASSETSYS_U64 const handle )
    {
    return strpool_cstr( &sys->strpool, handle );
    }


assetsys_t* assetsys_create( void* memctx )
    {
    assetsys_t* sys = (assetsys_t*) ASSETSYS_MALLOC( memctx, sizeof( assetsys_t ) );
    sys->memctx = memctx;

    strpool_config_t config = strpool_default_config;
    config.memctx = memctx;
    // OpenJazz modification
    config.ignore_case = 1;
    strpool_init( &sys->strpool, &config );

    sys->mounts_count = 0;
    sys->mounts_capacity = 16;
    sys->mounts = (struct assetsys_internal_mount_t*) ASSETSYS_MALLOC( memctx, 
        sizeof( *sys->mounts ) * sys->mounts_capacity );

    sys->collated_count = 0;
    sys->collated_capacity = 16384;
    sys->collated = (struct assetsys_internal_collated_t*) ASSETSYS_MALLOC( memctx, 
        sizeof( *sys->collated ) * sys->collated_capacity );
    return sys;
    }


void assetsys_destroy( assetsys_t* sys )
    {
    while( sys->mounts_count > 0 )
        {
        assetsys_dismount( sys, assetsys_internal_get_string( sys, sys->mounts[ 0 ].path ), 
            assetsys_internal_get_string( sys, sys->mounts[ 0 ].mounted_as ) );
        }
    ASSETSYS_FREE( sys->memctx, sys->collated );
    ASSETSYS_FREE( sys->memctx, sys->mounts );
    strpool_term( &sys->strpool );
    ASSETSYS_FREE( sys->memctx, sys );
    }


static int assetsys_internal_register_collated( assetsys_t* sys, char const* path, int const is_file )
    {
    if( path[ 0 ] == '/' && path[ 1 ] == '/' ) ++path;

    ASSETSYS_U64 handle = strpool_inject( &sys->strpool, path, (int) strlen( path ) );

    int first_free = -1;
    for( int i = 0; i < sys->collated_count; ++i )
        {
        if( sys->collated[ i ].ref_count > 0 && sys->collated[ i ].path == handle )
            {
            ASSETSYS_ASSERT( is_file == sys->collated[ i ].is_file, "Entry type mismatch" );
            ++sys->collated[ i ].ref_count;
            strpool_discard( &sys->strpool, handle );
            return i;
            }
        if( sys->collated[ i ].ref_count == 0 ) first_free = i;
        }

    if( first_free < 0)
        {
        if( sys->collated_count >= sys->collated_capacity ) 
            {
            sys->collated_capacity *= 2;
            struct assetsys_internal_collated_t* new_collated = (struct assetsys_internal_collated_t*) ASSETSYS_MALLOC( 
                sys->memctx, sizeof( *sys->collated ) * sys->collated_capacity );
            memcpy( new_collated, sys->collated, sizeof( *sys->collated ) * sys->collated_count );
            ASSETSYS_FREE( sys->memctx, sys->collated );
            sys->collated = new_collated;
            }
        first_free = sys->collated_count++;
        }


    struct assetsys_internal_collated_t* dir = &sys->collated[ first_free ];
    dir->path = handle;
    strpool_incref( &sys->strpool, handle );
    dir->parent = -1;
    dir->ref_count = 1;
    dir->is_file = is_file;
    return first_free;
    }


static void assetsys_internal_collate_directories( assetsys_t* sys, struct assetsys_internal_mount_t* const mount )
    {
    for( int i = 0; i < mount->dirs_count; ++i )
        {
        struct assetsys_internal_collated_t* subdir = &sys->collated[ mount->dirs[ i ].collated_index ];
        if( subdir->parent < 0 )
            {
            char const* a = assetsys_internal_get_string( sys, subdir->path ); (void) a;
            char* sub_path = assetsys_internal_dirname( assetsys_internal_get_string( sys, subdir->path ) ) ;
            ASSETSYS_U64 handle = strpool_inject( &sys->strpool, sub_path, (int) strlen( sub_path ) - 1 );
            for( int j = 0; j < sys->collated_count; ++j )
                {
                struct assetsys_internal_collated_t* dir = &sys->collated[ j ];
                char const* b = assetsys_internal_get_string( sys, dir->path ); (void) b;
                if( dir->path == handle )
                    {
                    subdir->parent = j;
                    break;
                    }
                }
            if( subdir->parent < 0 ) strpool_discard( &sys->strpool, handle );
            }
        }

    for( int i = 0; i < mount->files_count; ++i )
        {
        struct assetsys_internal_collated_t* file = &sys->collated[ mount->files[ i ].collated_index ];
        if( file->parent < 0 )
            {
            char* file_path = assetsys_internal_dirname( assetsys_internal_get_string( sys, file->path ) ) ;
            ASSETSYS_U64 handle = strpool_inject( &sys->strpool, file_path, file_path[0] == '/' && file_path[1] == '\0' ? 1 : (int) strlen( file_path ) - 1 );
            for( int j = 0; j < sys->collated_count; ++j )
                {
                struct assetsys_internal_collated_t* dir = &sys->collated[ j ];
                if( dir->path == handle )
                    {
                    file->parent = j;
                    break;
                    }
                }
            if( file->parent < 0 ) strpool_discard( &sys->strpool, handle );
            }
        }
    }


static void assetsys_internal_recurse_directories( assetsys_t* sys, int const collated_index, 
    struct assetsys_internal_mount_t* const mount )
    {
    char const* path = assetsys_internal_get_string( sys, sys->collated[ collated_index ].path );
    path += mount->mount_len;
    if( *path == '/' ) ++path;

    strcpy( sys->temp, assetsys_internal_get_string( sys, mount->path ) );
    strcat( sys->temp, ( *path == '\0' || *sys->temp == '\0' ) ? "" : "/" );
    strcat( sys->temp, path );

    struct assetsys_internal_dir_t dir;
    assetsys_internal_dir_open( &dir, *sys->temp == '\0' ? "." : sys->temp );
        
    struct assetsys_internal_dir_entry_t* dirent;
    for( dirent = assetsys_internal_dir_read( &dir ); dirent != NULL; dirent = assetsys_internal_dir_read( &dir ) )
        {
        char const* name = assetsys_internal_dir_name( dirent );
        if( !name || *name == '\0' || strcmp( name, "." ) == 0 || strcmp( name, ".." ) == 0 ) continue;
        int is_file = assetsys_internal_dir_is_file( dirent );
        int is_folder = assetsys_internal_dir_is_folder( dirent );
        if( is_file )
            {
            char const* file_path = assetsys_internal_get_string( sys, sys->collated[ collated_index ].path );
            file_path += mount->mount_len;
            if( *file_path == '/' ) ++file_path;

            strcpy( sys->temp, assetsys_internal_get_string( sys, mount->path ) );
            strcat( sys->temp, ( *file_path == '\0' || *sys->temp == '\0' ) ? "" : "/" );
            strcat( sys->temp, file_path );
            strcat( sys->temp, *sys->temp == '\0' ? "" : "/" );
            strcat( sys->temp, name );

            struct stat s;
            if( stat( sys->temp, &s ) == 0 )
                {
                strcpy( sys->temp, assetsys_internal_get_string( sys, mount->mounted_as ) );
                if( *sys->temp && sys->temp[ strlen( sys->temp ) - 1 ] != '/' ) strcat( sys->temp, "/" );
                strcat( sys->temp, file_path );
                strcat( sys->temp, *file_path == '\0' ? "" : "/" );
                strcat( sys->temp, name );

                if( mount->files_count >= mount->files_capacity )
                    {
                    mount->files_capacity *= 2;
                    struct assetsys_internal_file_t* new_files = (struct assetsys_internal_file_t*) ASSETSYS_MALLOC( 
                        sys->memctx, sizeof( *(mount->files) ) * mount->files_capacity );
                    memcpy( new_files, mount->files, sizeof( *(mount->files) ) * mount->files_count );
                    ASSETSYS_FREE( sys->memctx, mount->files );
                    mount->files = new_files;
                    }

                struct assetsys_internal_file_t* file = &mount->files[ mount->files_count++ ];
                file->size = (int) s.st_size;
                file->zip_index = -1;
                file->collated_index = assetsys_internal_register_collated( sys, sys->temp, 1 );
                }
            }
        else if( is_folder )
            {
            char const* folder_path = assetsys_internal_get_string( sys, sys->collated[ collated_index ].path );
            folder_path += mount->mount_len;
            if( *folder_path == '/' ) ++folder_path;

            strcpy( sys->temp, assetsys_internal_get_string( sys, mount->path ) );
            strcat( sys->temp, ( *folder_path == '\0' || *sys->temp == '\0' ) ? "" : "/" );
            strcat( sys->temp, folder_path );
            strcat( sys->temp, *sys->temp == '\0' ? "" : "/" );
            strcat( sys->temp, name );

            struct stat s;
            if( stat( sys->temp, &s ) == 0 )
                {
                strcpy( sys->temp, assetsys_internal_get_string( sys, mount->mounted_as ) );
                if( *sys->temp && sys->temp[ strlen( sys->temp ) - 1 ] != '/' ) strcat( sys->temp, "/" );
                strcat( sys->temp, folder_path );
                strcat( sys->temp, *folder_path == '\0' ? "" : "/" );
                strcat( sys->temp, name );

                if( mount->dirs_count >= mount->dirs_capacity )
                    {
                    mount->dirs_capacity *= 2;
                    struct assetsys_internal_folder_t* new_dirs = (struct assetsys_internal_folder_t*) ASSETSYS_MALLOC( 
                        sys->memctx, sizeof( *(mount->dirs) ) * mount->dirs_capacity );
                    memcpy( new_dirs, mount->dirs, sizeof( *(mount->dirs) ) * mount->dirs_count );
                    ASSETSYS_FREE( sys->memctx, mount->dirs );
                    mount->dirs = new_dirs;
                    }
// OpenJazz modification
#ifdef OJ_RECURSE
                struct assetsys_internal_folder_t* as_dir = &mount->dirs[ mount->dirs_count++ ];
                as_dir->collated_index = assetsys_internal_register_collated( sys, sys->temp, 0 );
                assetsys_internal_recurse_directories( sys, as_dir->collated_index, mount );
#endif
                }
            }        
        }
    assetsys_internal_dir_close( &dir );
    }

/**
 * Mount the given internal mount data into the assetsys instance.
 * 
 * @internal
 */
assetsys_error_t assetsys_internal_mount_files( assetsys_t* sys, struct assetsys_internal_mount_t* mount )
    {
    int count = (int) mz_zip_reader_get_num_files( &mount->zip );

    for( int i = 0; i < count; ++i )
        {
        if( mz_zip_reader_is_file_a_directory( &mount->zip, (mz_uint) i ) )
            {
            if( mount->dirs_count >= mount->dirs_capacity )
                {
                mount->dirs_capacity *= 2;
                struct assetsys_internal_folder_t* new_dirs = (struct assetsys_internal_folder_t*) ASSETSYS_MALLOC( 
                    sys->memctx, sizeof( *(mount->dirs) ) * mount->dirs_capacity );
                memcpy( new_dirs, mount->dirs, sizeof( *(mount->dirs) ) * mount->dirs_count );
                ASSETSYS_FREE( sys->memctx, mount->dirs );
                mount->dirs = new_dirs;
                }

            char filename[ 1024 ];                
            mz_zip_reader_get_filename( &mount->zip, (mz_uint) i, filename, sizeof( filename ) );

            struct assetsys_internal_folder_t* as_dir = &mount->dirs[ mount->dirs_count++ ];
            strcpy( sys->temp, assetsys_internal_get_string( sys, mount->mounted_as ) );
            strcat( sys->temp, "/" );
            strcat( sys->temp, filename );
            sys->temp[ strlen( sys->temp )  - 1 ] = '\0';
            as_dir->collated_index = assetsys_internal_register_collated( sys, sys->temp, 0 );
            }
        }

    for( int i = 0; i < count; ++i )
        {
        if( !mz_zip_reader_is_file_a_directory( &mount->zip, (mz_uint) i ) )
            {
            if( mount->files_count >= mount->files_capacity )
                {
                mount->files_capacity *= 2;
                struct assetsys_internal_file_t* new_files = (struct assetsys_internal_file_t*) ASSETSYS_MALLOC( 
                    sys->memctx, sizeof( *(mount->files) ) * mount->files_capacity );
                memcpy( new_files, mount->files, sizeof( *(mount->files) ) * mount->files_count );
                ASSETSYS_FREE( sys->memctx, mount->files );
                mount->files = new_files;
                }

            mz_zip_archive_file_stat stat;
            mz_bool result = mz_zip_reader_file_stat( &mount->zip, (mz_uint) i, &stat);
            if( !result )
                {
                mz_zip_reader_end( &mount->zip );
                ASSETSYS_FREE( sys->memctx, mount->dirs );
                ASSETSYS_FREE( sys->memctx, mount->files );
                return ASSETSYS_ERROR_FAILED_TO_READ_ZIP;
                }

            struct assetsys_internal_file_t* file = &mount->files[ mount->files_count++ ];
            strcpy( sys->temp, assetsys_internal_get_string( sys, mount->mounted_as ) );
            strcat( sys->temp, "/" );
            strcat( sys->temp, stat.m_filename );
            file->collated_index = assetsys_internal_register_collated( sys, sys->temp, 1 );
            file->size = (int) stat.m_uncomp_size;
            file->zip_index = i;

            char* dir_path = assetsys_internal_dirname( sys->temp );
            ASSETSYS_U64 handle = strpool_inject( &sys->strpool, dir_path, (int) strlen( dir_path ) - 1 );               
            int found = 0;
            for( int j = 0; j < mount->dirs_count; ++j )
                {
                if( handle == sys->collated[ mount->dirs[ j ].collated_index ].path )
                    found = 1;
                }
            if( !found ) 
                {
                struct assetsys_internal_folder_t* as_dir = &mount->dirs[ mount->dirs_count++ ];
                as_dir->collated_index = assetsys_internal_register_collated( sys, 
                    assetsys_internal_get_string( sys, handle ), 0 );
                }
            }
        }

        return ASSETSYS_SUCCESS;
    }

/**
 * Creates an internal mount object for use by assetsys.
 * 
 * @internal
 */
struct assetsys_internal_mount_t* assetsys_internal_create_mount( assetsys_t* sys, enum assetsys_internal_mount_type_t type, char const* path, char const* mount_as )
    {
    if( sys->mounts_count >= sys->mounts_capacity )
        {
        sys->mounts_capacity *= 2;
        struct assetsys_internal_mount_t* new_mounts = (struct assetsys_internal_mount_t*) ASSETSYS_MALLOC( sys->memctx, 
            sizeof( *sys->mounts ) * sys->mounts_capacity );
        memcpy( new_mounts, sys->mounts, sizeof( *sys->mounts ) * sys->mounts_count );
        ASSETSYS_FREE( sys->memctx, sys->mounts );
        sys->mounts = new_mounts;
        }

    struct assetsys_internal_mount_t* mount = &sys->mounts[ sys->mounts_count ];

    mount->mounted_as = assetsys_internal_add_string( sys, mount_as ? mount_as : "" );
    mount->mount_len = mount_as ? (int) strlen( mount_as ) : 0;
    mount->path = assetsys_internal_add_string( sys, path );
    mount->type = type;
        
    mount->files_count = 0;
    mount->files_capacity = 4096;
    mount->files = (struct assetsys_internal_file_t*) ASSETSYS_MALLOC( sys->memctx, 
        sizeof( *(mount->files) ) * mount->files_capacity );

    mount->dirs_count = 0;
    mount->dirs_capacity = 1024;
    mount->dirs = (struct assetsys_internal_folder_t*) ASSETSYS_MALLOC( sys->memctx, 
        sizeof( *(mount->dirs) ) * mount->dirs_capacity );

    if( type == ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP )
        {
        memset( &mount->zip, 0, sizeof( mount->zip ) );
        mount->zip.m_pAlloc = assetsys_internal_mz_alloc;
        mount->zip.m_pRealloc = assetsys_internal_mz_realloc;
        mount->zip.m_pFree = assetsys_internal_mz_free;
        mount->zip.m_pAlloc_opaque = sys->memctx;
        }
    
    return mount;
    }

assetsys_error_t assetsys_mount_from_memory( assetsys_t* sys, void const* data, int size, char const* mount_as)
    {
    if (!data) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if( !mount_as ) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if( strchr( mount_as, '\\' ) ) return ASSETSYS_ERROR_INVALID_PATH;
    int mount_len = (int) strlen( mount_as );
    if( mount_len == 0 || mount_as[ 0 ] != '/' || ( mount_len > 1 && mount_as[ mount_len - 1 ] == '/' ) ) 
        return ASSETSYS_ERROR_INVALID_PATH;

    struct assetsys_internal_mount_t* mount = assetsys_internal_create_mount(sys, ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP, "data", mount_as);

    mz_bool status = mz_zip_reader_init_mem( &mount->zip, data, size, 0 );
    if( !status )
        {
        ASSETSYS_FREE( sys->memctx, mount->dirs );
        ASSETSYS_FREE( sys->memctx, mount->files );
        return ASSETSYS_ERROR_FAILED_TO_READ_ZIP;
        }

    assetsys_error_t result = assetsys_internal_mount_files( sys, mount );
    if( result != ASSETSYS_SUCCESS )
        return result;

    assetsys_internal_collate_directories( sys, mount );

    ++sys->mounts_count;
    return ASSETSYS_SUCCESS;
    }

assetsys_error_t assetsys_mount( assetsys_t* sys, char const* path, char const* mount_as )
    {
    if( !path ) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if( !mount_as ) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if( strchr( path, '\\' ) ) return ASSETSYS_ERROR_INVALID_PATH;
    if( strchr( mount_as, '\\' ) ) return ASSETSYS_ERROR_INVALID_PATH;
    int len = (int) strlen( path );
    if( len > 1 && path[ len - 1 ] == '/' ) return ASSETSYS_ERROR_INVALID_PATH;     
    int mount_len = (int) strlen( mount_as );
    if( mount_len == 0 || mount_as[ 0 ] != '/' || ( mount_len > 1 && mount_as[ mount_len - 1 ] == '/' ) ) 
        return ASSETSYS_ERROR_INVALID_PATH;     
    
    enum assetsys_internal_mount_type_t type;

    #if defined( _MSC_VER ) && _MSC_VER >= 1400
        struct _stat64 s;
        int res = __stat64( *path == '\0' ? "." : path, &s );
    #else
        struct stat s;
        int res = stat( *path == '\0' ? "." : path, &s );
    #endif
    if( res == 0 )
        {
        if( s.st_mode & S_IFDIR ) type = ASSETSYS_INTERNAL_MOUNT_TYPE_DIR;
        else if( s.st_mode & S_IFREG ) type = ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP;
        else return ASSETSYS_ERROR_INVALID_PATH;
        }
    else
        {
        return ASSETSYS_ERROR_INVALID_PATH;
        }

    struct assetsys_internal_mount_t* mount = assetsys_internal_create_mount(sys, type, path, mount_as);

    if( type == ASSETSYS_INTERNAL_MOUNT_TYPE_DIR )
        {
        struct assetsys_internal_folder_t* dir = &mount->dirs[ mount->dirs_count++ ];
        dir->collated_index = assetsys_internal_register_collated( sys, mount_as, 0 );
        assetsys_internal_recurse_directories( sys, dir->collated_index, mount );
        }
    else if( type == ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP )
        {
#ifdef MINIZ_NO_STDIO
        // If we explicitly disable stdio.
        return ASSETSYS_ERROR_FAILED_TO_READ_ZIP;
#else
        mz_bool status = mz_zip_reader_init_file( &mount->zip, path, 0 );
        if( !status )
            {
            ASSETSYS_FREE( sys->memctx, mount->dirs );
            ASSETSYS_FREE( sys->memctx, mount->files );
            return ASSETSYS_ERROR_FAILED_TO_READ_ZIP;
            }

        assetsys_error_t result = assetsys_internal_mount_files( sys, mount );
        if( result != ASSETSYS_SUCCESS )
            return result;
#endif
        }

    assetsys_internal_collate_directories( sys, mount );

    ++sys->mounts_count;
    return ASSETSYS_SUCCESS;
    }


static void assetsys_internal_remove_collated( assetsys_t* sys, int const index )
    {
    struct assetsys_internal_collated_t* coll = &sys->collated[ index ];
    ASSETSYS_ASSERT( coll->ref_count > 0, "Invalid ref count" );
    --coll->ref_count;
    if( coll->ref_count == 0 )
        {
        strpool_decref( &sys->strpool, coll->path );
        strpool_discard( &sys->strpool, coll->path );
        }
    }


assetsys_error_t assetsys_dismount( assetsys_t* sys, char const* path, char const* mounted_as )
    {
    if( !path ) return ASSETSYS_ERROR_INVALID_PARAMETER;
    if( !mounted_as ) return ASSETSYS_ERROR_INVALID_MOUNT;

    ASSETSYS_U64 path_handle = strpool_inject( &sys->strpool, path, (int) strlen( path ) );
    ASSETSYS_U64 mount_handle = strpool_inject( &sys->strpool, mounted_as, (int) strlen( mounted_as ) );

    for( int i = 0; i < sys->mounts_count; ++i )
        {
        struct assetsys_internal_mount_t* mount = &sys->mounts[ i ];
        if( mount->mounted_as == mount_handle && mount->path == path_handle )
            {
            mz_bool result = 1;
            if( mount->type == ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP ) result = mz_zip_reader_end( &mount->zip );

            strpool_decref( &sys->strpool, mount->mounted_as );
            strpool_decref( &sys->strpool, mount->path );
            strpool_discard( &sys->strpool, mount_handle );
            strpool_discard( &sys->strpool, path_handle );
            
            for( int j = 0; j < mount->dirs_count; ++j )
                assetsys_internal_remove_collated( sys, mount->dirs[ j ].collated_index );

            for( int j = 0; j < mount->files_count; ++j )
                assetsys_internal_remove_collated( sys, mount->files[ j ].collated_index );

            ASSETSYS_FREE( sys->memctx, mount->dirs );
            ASSETSYS_FREE( sys->memctx, mount->files );

            int count = sys->mounts_count - i;
            if( count > 0 ) memcpy( &sys->mounts[ i ], &sys->mounts[ i + 1 ], sizeof( *sys->mounts ) * count );
            --sys->mounts_count;

            return !result ? ASSETSYS_ERROR_FAILED_TO_CLOSE_ZIP : ASSETSYS_SUCCESS;
            }
        }

    strpool_discard( &sys->strpool, mount_handle );
    strpool_discard( &sys->strpool, path_handle );
    return ASSETSYS_ERROR_INVALID_MOUNT;
    }


assetsys_error_t assetsys_file( assetsys_t* sys, char const* path, assetsys_file_t* file )
    {
    if( !file || !path ) return ASSETSYS_ERROR_INVALID_PARAMETER;

    ASSETSYS_U64 handle = strpool_inject( &sys->strpool, path, (int) strlen( path ) );

    int m = sys->mounts_count;
    while( m > 0)
        {
        --m;
        struct assetsys_internal_mount_t* mount = &sys->mounts[ m ];
        for( int i = 0; i < mount->files_count; ++i )
            {
            ASSETSYS_U64 h = sys->collated[ mount->files[ i ].collated_index ].path;
            if( handle == h )
                {
                file->mount = mount->mounted_as;
                file->path = mount->path;
                file->index = i;
                return ASSETSYS_SUCCESS;
                }
            }
        }

    strpool_discard( &sys->strpool, handle );
    return ASSETSYS_ERROR_FILE_NOT_FOUND;
    }


static int assetsys_internal_find_mount_index( assetsys_t* sys, ASSETSYS_U64 const mount, ASSETSYS_U64 const path )
    {
    for( int i = 0; i < sys->mounts_count; ++i )
        {
        if( sys->mounts[ i ].mounted_as == mount && sys->mounts[ i ].path == path )
            return i;
        }
    return -1;
    }


assetsys_error_t assetsys_file_load( assetsys_t* sys, assetsys_file_t f, int* size, void* buffer, int capacity )
    {
    int mount_index = assetsys_internal_find_mount_index( sys, f.mount, f.path );
    if( mount_index < 0 ) return ASSETSYS_ERROR_INVALID_MOUNT;

    struct assetsys_internal_mount_t* mount = &sys->mounts[ mount_index ];
    struct assetsys_internal_file_t* file = &mount->files[ f.index ];
    if( mount->type == ASSETSYS_INTERNAL_MOUNT_TYPE_ZIP )
        {
        if( size ) *size = (int) file->size;
        if( file->size > capacity ) return ASSETSYS_ERROR_BUFFER_TOO_SMALL;

        mz_bool result = mz_zip_reader_extract_to_mem_no_alloc( &mount->zip, (mz_uint) file->zip_index, buffer, 
            (size_t) file->size, 0, 0, 0 ); 
        return result ? ASSETSYS_SUCCESS : ASSETSYS_ERROR_FAILED_TO_READ_FILE;
        }
    else
        {
        if( size ) *size = file->size;
        strcpy( sys->temp, assetsys_internal_get_string( sys, mount->path ) );
        strcat( sys->temp, *sys->temp == '\0' ? "" : "/" );
        strcat( sys->temp, assetsys_internal_get_string( sys, 
            sys->collated[ file->collated_index ].path ) + 
            ( strcmp( assetsys_internal_get_string( sys, mount->mounted_as ), "/" ) == 0 ? 0 : mount->mount_len + 1 ) );
        FILE* fp = fopen( sys->temp, "rb" );
        if( !fp ) return ASSETSYS_ERROR_FAILED_TO_READ_FILE;
        
        fseek( fp, 0, SEEK_END );
        int file_size = (int) ftell( fp );
        fseek( fp, 0, SEEK_SET );
        if( size ) *size = file_size;

        if( file_size > capacity ) { fclose( fp ); return ASSETSYS_ERROR_BUFFER_TOO_SMALL; }

        int size_read = (int) fread( buffer, 1, (size_t) file_size, fp );
        fclose( fp );
        if( size_read != file_size ) return ASSETSYS_ERROR_FAILED_TO_READ_FILE;

        return ASSETSYS_SUCCESS;
        }
    }


int assetsys_file_size( assetsys_t* sys, assetsys_file_t file )
    {
    int mount_index = assetsys_internal_find_mount_index( sys, file.mount, file.path );
    if( mount_index < 0 ) return 0;

    struct assetsys_internal_mount_t* mount = &sys->mounts[ mount_index ];
    if( mount->type == ASSETSYS_INTERNAL_MOUNT_TYPE_DIR )
        {
        strcpy( sys->temp, assetsys_internal_get_string( sys, mount->path ) );
        strcat( sys->temp, *sys->temp == '\0' ? "" : "/" );
        strcat( sys->temp, assetsys_internal_get_string( sys, 
            sys->collated[ mount->files[ file.index ].collated_index ].path ) + mount->mount_len + 1 );
        struct stat s;
        if( stat( sys->temp, &s ) == 0 )
            mount->files[ file.index ].size = (int) s.st_size;
        }
        
    return mount->files[ file.index ].size;
    }


static int assetsys_internal_find_collated( assetsys_t* sys, char const* const path )
    {
    ASSETSYS_U64 handle = strpool_inject( &sys->strpool, path, (int) strlen( path ) );

    for( int i = 0; i < sys->collated_count; ++i )
        {
        if( sys->collated[ i ].path == handle )
            {
            return i;
            }
        }

    strpool_discard( &sys->strpool, handle );
    return -1;
    }


int assetsys_file_count( assetsys_t* sys, char const* path )
    {
    if( !path ) return 0;
    int dir = assetsys_internal_find_collated( sys, path );
    int count = 0;
    for( int i = 0; i < sys->collated_count; ++i )
        {
        if( sys->collated[ i ].is_file && sys->collated[ i ].parent == dir )
            {
            ++count;
            }
        }
    return count;
    }


char const* assetsys_file_name( assetsys_t* sys, char const* path, int index )
    {
    char const* file_path = assetsys_file_path( sys, path, index );
    if( file_path ) 
        {
        char const* name = strrchr( file_path, '/' );
        if( !name ) return file_path;
        return name + 1;
        }

    return NULL;
    }


char const* assetsys_file_path( assetsys_t* sys, char const* path, int index )
    {
    if( !path ) return 0;
    int dir = assetsys_internal_find_collated( sys, path );
    int count = 0;
    for( int i = 0; i < sys->collated_count; ++i )
        {
        if( sys->collated[ i ].is_file &&  sys->collated[ i ].parent == dir )
            {
            if( count == index ) return assetsys_internal_get_string( sys, sys->collated[ i ].path );
            ++count;
            }
        }
    return NULL;
    }


int assetsys_subdir_count( assetsys_t* sys, char const* path )
    {
    if( !path ) return 0;
    int dir = assetsys_internal_find_collated( sys, path );
    int count = 0;
    for( int i = 0; i < sys->collated_count; ++i )
        {
        if( !sys->collated[ i ].is_file && sys->collated[ i ].parent == dir )
            {
            ++count;
            }
        }
    return count;
    }


char const* assetsys_subdir_name( assetsys_t* sys, char const* path, int index )
    {
    char const* subdir_path = assetsys_subdir_path( sys, path, index );
    if( subdir_path ) 
        {
        char const* name = strrchr( subdir_path, '/' );
        if( !name ) return subdir_path;
        return name + 1;
        }

    return NULL;
    }


char const* assetsys_subdir_path( assetsys_t* sys, char const* path, int index )
    {
    if( !path ) return 0;
    int dir = assetsys_internal_find_collated( sys, path );
    int count = 0;
    for( int i = 0; i < sys->collated_count; ++i )
        {
        if( !sys->collated[ i ].is_file &&  sys->collated[ i ].parent == dir )
            {
            if( count == index ) return assetsys_internal_get_string( sys, sys->collated[ i ].path );
            ++count;
            }
        }
    return NULL;
    }


const char *assetsys_file_to_path(assetsys_t* sys, assetsys_file_t file) {
    int mount_index = assetsys_internal_find_mount_index(sys, file.mount, file.path);
    if(mount_index < 0) return "";

    sys->temp[0] = '\0';

    struct assetsys_internal_mount_t* mount = &sys->mounts[mount_index];
    strcpy(sys->temp, assetsys_internal_get_string(sys, mount->path));
    strcat(sys->temp, *sys->temp == '\0' ? "" : "/" );
    strcat(sys->temp, assetsys_internal_get_string(sys,
        sys->collated[mount->files[file.index].collated_index].path) + mount->mount_len + 1);

    return sys->temp;
}

static char* assetsys_internal_dirname( char const* path )
    {
    static char result[ 260 ];
    strncpy( result, path, sizeof( result ) );
      
    char* lastForwardSlash = strrchr( result, '/' );

    if( lastForwardSlash ) *(lastForwardSlash + 1 ) = '\0';
    else *result = '\0';

    return result;
    }


#endif /* ASSETSYS_IMPLEMENTATION */


/*
----------------------
    TESTS
----------------------
*/


#ifdef ASSETSYS_RUN_TESTS

#include "testfw.h"


void test_assetsys( void ) {
    TESTFW_TEST_BEGIN( "Test mounting path and loading file" );

    // Create the asset system
    assetsys_t* assetsys = assetsys_create( 0 );
    TESTFW_EXPECTED( assetsys != NULL );

    // Mount current working folder as a virtual "/data" path
    TESTFW_EXPECTED( assetsys_mount( assetsys, ".", "/data" ) == ASSETSYS_SUCCESS );

    // Load a file
    assetsys_file_t file;
    TESTFW_EXPECTED(  assetsys_file( assetsys, "/data/README.md", &file ) == ASSETSYS_SUCCESS );

    // Find the size of the file
    int size = assetsys_file_size( assetsys, file );
    TESTFW_EXPECTED( size > 30 );

    // Load the file
    char* content = (char*) malloc( size + 1 ); // extra space for '\0'
    int loaded_size = 0;
    TESTFW_EXPECTED( assetsys_file_load( assetsys, file, &loaded_size, content, size ) == ASSETSYS_SUCCESS );
    content[ size ] = '\0'; // zero terminate the text file

    // Clean up
    free( content );
    assetsys_destroy( assetsys );

    TESTFW_TEST_END();

    TESTFW_TEST_BEGIN( "Test mounting data and loading file" );
        {
        // Zip file with a test.txt file containing "Hello, World!"
        const unsigned char data[]  = {
            0x50, 0x4b, 0x03, 0x04, 0x0a, 0x03, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x9e, 0x23, 0x57, 0x84, 0x9e, 
            0xe8, 0xb4, 0x0e, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x74, 0x65, 
            0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x57, 0x6f, 0x72, 
            0x6c, 0x64, 0x21, 0x0a, 0x50, 0x4b, 0x01, 0x02, 0x3f, 0x03, 0x0a, 0x03, 0x00, 0x00, 0x00, 0x00, 
            0xfc, 0x9e, 0x23, 0x57, 0x84, 0x9e, 0xe8, 0xb4, 0x0e, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 
            0x08, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x80, 0xb4, 0x81, 0x00, 0x00, 
            0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x0a, 0x00, 0x20, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x0e, 0xf2, 0x2d, 0xc2, 0xde, 0xd9, 0x01, 0x00, 0x0e, 
            0xf2, 0x2d, 0xc2, 0xde, 0xd9, 0x01, 0x00, 0x0e, 0xf2, 0x2d, 0xc2, 0xde, 0xd9, 0x01, 0x50, 0x4b, 
            0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x5a, 0x00, 0x00, 0x00, 0x34, 0x00, 
            0x00, 0x00, 0x00, 0x00
        };
        const int data_size = 164;

        // Create the asset system
        assetsys_t* assetsys = assetsys_create( 0 );
        TESTFW_EXPECTED( assetsys != NULL );

        // Mount current working folder as a virtual "/data" path
        TESTFW_EXPECTED( assetsys_mount_from_memory( assetsys, data, data_size, "/data" ) == ASSETSYS_SUCCESS );

        // Load a file
        assetsys_file_t file;
        TESTFW_EXPECTED(  assetsys_file( assetsys, "/data/test.txt", &file ) == ASSETSYS_SUCCESS );

        // Find the size of the file
        int size = assetsys_file_size( assetsys, file );
        TESTFW_EXPECTED( size > 10 );

        // Load the file
        char* content = (char*) malloc( size + 1 ); // extra space for '\0'
        int loaded_size = 0;
        TESTFW_EXPECTED( assetsys_file_load( assetsys, file, &loaded_size, content, size ) == ASSETSYS_SUCCESS );
        content[ size ] = '\0'; // zero terminate the text file

        TESTFW_EXPECTED( content[0] == 'H' );
        TESTFW_EXPECTED( content[1] == 'e' );
        TESTFW_EXPECTED( content[2] == 'l' );
        TESTFW_EXPECTED( content[3] == 'l' );
        TESTFW_EXPECTED( content[4] == 'o' );

        // Clean up
        free( content );
        assetsys_destroy( assetsys );
        }
    TESTFW_TEST_END();
}


int main( int argc, char** argv ) {
    (void) argc, argv;

    TESTFW_INIT();
    
    test_assetsys();

    return TESTFW_SUMMARY();
}


// pass-through so the program will build with either /SUBSYSTEM:WINDOWS or /SUBSYSTEM:CONSOLE
#if defined( _WIN32 ) && !defined( __TINYC__ )
    #include <stdlib.h>
    #ifdef __cplusplus 
        extern "C" int __stdcall WinMain( struct HINSTANCE__*, struct HINSTANCE__*, char*, int ) { 
            return main( __argc, __argv ); 
        }
    #else
        struct HINSTANCE__;
        int __stdcall WinMain( struct HINSTANCE__* a, struct HINSTANCE__* b, char* c, int d ) { 
            (void) a, b, c, d; return main( __argc, __argv ); 
        }
    #endif
#endif

#define TESTFW_IMPLEMENTATION
#include "testfw.h"

#endif /* ASSETSYS_RUN_TESTS */


/*

contributors:
    Randy Gaul (hotloading support)
    Rob Loach (assetsys_mount_from_memory)

revision history:
    1.5     fix issue where mount as root "/" didn't work when mounting folder
    1.4     allow mounting from memory
    1.3     allow absolute paths when mounting, update docs for mount as root
    1.2     asserts with message, eliminated a frequent small allocation
    1.1     changes to support loading assets being re-saved during execution
    1.0     first released version  

*/


/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.
Uses public domain code for "miniz" zip file support - original license can be 
found in the miniz section of this file.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2015 Mattias Gustavsson

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/
    

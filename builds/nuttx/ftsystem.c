/****************************************************************************
 *
 * ftsystem.c
 *
 *   ANSI-specific FreeType low-level system interface (body).
 *
 * Copyright (C) 2022 by Xiaomi Inc.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftstream.h>
#include <freetype/ftsystem.h>
#include <freetype/fterrors.h>
#include <freetype/fttypes.h>
#include <nuttx/config.h>
#include <nuttx/trace.h>
#include <nuttx/mm/mm.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#define FT_TRACE_BEGIN fs_trace_begin()
#define FT_TRACE_END fs_trace_end()

  /**************************************************************************
   *
   *                      MEMORY MANAGEMENT INTERFACE
   *
   */

  /**************************************************************************
   *
   * It is not necessary to do any error checking for the
   * allocation-related functions.  This will be done by the higher level
   * routines like ft_mem_alloc() or ft_mem_realloc().
   *
   */


  /**************************************************************************
   *
   * @struct:
   *   FT_MemoryEx
   *
   * @description:
   *   A structure used to describe a custom memory.
   *
   * @fields:
   *   FT_MemoryRec_ ::
   *     A structure used to describe a given memory manager to FreeType~2.
   *
   *   buffer ::
   *     A pointer to custom buffer.
   *
   */
  typedef struct
  {
    struct FT_MemoryRec_ memory;
    void*                buffer;
  } FT_MemoryEx;


  /**************************************************************************
   *
   * @Function:
   *   ft_alloc
   *
   * @Description:
   *   The memory allocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   size ::
   *     The requested size in bytes.
   *
   * @Return:
   *   The address of newly allocated block.
   */
  FT_CALLBACK_DEF( void* )
  ft_alloc( FT_Memory  memory,
            long       size )
  {
#if CONFIG_LIB_FREETYPE_MEMSIZE
    struct mm_heap_s* heap = memory->user;
    return mm_malloc( heap, (size_t)size );
#else
    FT_UNUSED( memory );
    return malloc( (size_t)size );
#endif
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_realloc
   *
   * @Description:
   *   The memory reallocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   cur_size ::
   *     The current size of the allocated memory block.
   *
   *   new_size ::
   *     The newly requested size in bytes.
   *
   *   block ::
   *     The current address of the block in memory.
   *
   * @Return:
   *   The address of the reallocated memory block.
   */
  FT_CALLBACK_DEF( void* )
  ft_realloc( FT_Memory  memory,
              long       cur_size,
              long       new_size,
              void*      block )
  {
    FT_UNUSED( cur_size );

#if CONFIG_LIB_FREETYPE_MEMSIZE
    struct mm_heap_s* heap = memory->user;
    return mm_realloc( heap, block, (size_t)new_size );
#else
    FT_UNUSED( memory );
    return realloc( block, (size_t)new_size );
#endif
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_free
   *
   * @Description:
   *   The memory release function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   block ::
   *     The address of block in memory to be freed.
   */
  FT_CALLBACK_DEF( void )
  ft_free( FT_Memory  memory,
           void*      block )
  {
#if CONFIG_LIB_FREETYPE_MEMSIZE
    struct mm_heap_s* heap = memory->user;
    mm_free( heap, block );
#else
    FT_UNUSED( memory );
    free( block );
#endif
  }


  /**************************************************************************
   *
   *                    RESOURCE MANAGEMENT INTERFACE
   *
   */

#ifndef FT_CONFIG_OPTION_DISABLE_STREAM_SUPPORT

  /**************************************************************************
   *
   * The macro FT_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_COMPONENT
#define FT_COMPONENT  io

  /* We use the macro STREAM_PTR for convenience to extract the           */
  /* system-specific stream handle from a given FreeType stream object.   */
  /* The reason for 'fd + 1' is because open() may return a legal fd with */
  /* a value of 0, preventing it from being judged as NULL when converted */
  /* to a pointer type                                                    */
#define STREAM_PTR_TO_FD( stream )  ( (int)(intptr_t)stream->descriptor.pointer - 1 )
#define FD_TO_STREAM_PTR( fd )      ( (void *)(intptr_t)( fd + 1 ) )


  /**************************************************************************
   *
   * @Function:
   *   ft_posix_stream_close
   *
   * @Description:
   *   The function to close a stream.
   *
   * @Input:
   *   stream ::
   *     A pointer to the stream object.
   */
  FT_CALLBACK_DEF( void )
  ft_posix_stream_close( FT_Stream  stream )
  {
    FT_TRACE_BEGIN;
    close( STREAM_PTR_TO_FD( stream ) );

    stream->descriptor.pointer = NULL;
    stream->size               = 0;
    stream->base               = NULL;
    FT_TRACE_END;
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_posix_stream_io
   *
   * @Description:
   *   The function to open a stream.
   *
   * @Input:
   *   stream ::
   *     A pointer to the stream object.
   *
   *   offset ::
   *     The position in the data stream to start reading.
   *
   *   buffer ::
   *     The address of buffer to store the read data.
   *
   *   count ::
   *     The number of bytes to read from the stream.
   *
   * @Return:
   *   The number of bytes actually read.  If `count' is zero (this is,
   *   the function is used for seeking), a non-zero return value
   *   indicates an error.
   */
  FT_CALLBACK_DEF( unsigned long )
  ft_posix_stream_io( FT_Stream       stream,
                      unsigned long   offset,
                      unsigned char*  buffer,
                      unsigned long   count )
  {
    int  file;
    unsigned long total = 0;
    FT_TRACE_BEGIN;

    if ( !count && offset > stream->size )
    {
      FT_TRACE_END;
      return 1;
    }

    file = STREAM_PTR_TO_FD( stream );

    if ( stream->pos != offset )
      lseek( file, (off_t)offset, SEEK_SET );

    if ( count == 0 )
    {
      FT_TRACE_END;
      return 0;
    }

    while ( total < count )
    {
      size_t remain = count - total;
      ssize_t rd = read( file, buffer + total, remain );

      if ( rd <= 0 )
        break;

      total += (unsigned long)rd;
    }

    FT_TRACE_END;
    return total;
  }


  /* documentation is in ftstream.h */

  FT_BASE_DEF( FT_Error )
  FT_Stream_Open( FT_Stream    stream,
                  const char*  filepathname )
  {
    int  file;


    if ( !stream )
      return FT_THROW( Invalid_Stream_Handle );

    stream->descriptor.pointer = NULL;
    stream->pathname.pointer   = (char*)filepathname;
    stream->base               = NULL;
    stream->pos                = 0;
    stream->read               = NULL;
    stream->close              = NULL;

    FT_TRACE_BEGIN;
    file = open( filepathname, O_RDONLY | O_CLOEXEC );
    FT_TRACE_END;

    if ( file < 0 )
    {
      FT_ERROR(( "FT_Stream_Open:"
                 " could not open `%s'\n", filepathname ));

      return FT_THROW( Cannot_Open_Resource );
    }

    lseek( file, 0, SEEK_END );
    stream->size = (unsigned long)lseek( file, 0, SEEK_CUR );
    if ( !stream->size )
    {
      FT_ERROR(( "FT_Stream_Open:" ));
      FT_ERROR(( " opened `%s' but zero-sized\n", filepathname ));
      close( file );
      return FT_THROW( Cannot_Open_Stream );
    }
    lseek( file, 0, SEEK_SET );

    stream->descriptor.pointer = FD_TO_STREAM_PTR( file );
    stream->read  = ft_posix_stream_io;
    stream->close = ft_posix_stream_close;

    FT_TRACE1(( "FT_Stream_Open:" ));
    FT_TRACE1(( " opened `%s' (%ld bytes) successfully\n",
                filepathname, stream->size ));

    return FT_Err_Ok;
  }

#endif /* !FT_CONFIG_OPTION_DISABLE_STREAM_SUPPORT */

#ifdef FT_DEBUG_MEMORY

  extern FT_Int
  ft_mem_debug_init( FT_Memory  memory );

  extern void
  ft_mem_debug_done( FT_Memory  memory );

#endif


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( FT_Memory )
  FT_New_Memory( void )
  {
    FT_Memory  memory;

#if CONFIG_LIB_FREETYPE_MEMSIZE
    struct mm_heap_s* heap;
    FT_MemoryEx* memex;

    const size_t bufsize = CONFIG_LIB_FREETYPE_MEMSIZE * 1024;
    void* buffer = malloc( bufsize );
    DEBUGASSERT( buffer != NULL );

    heap = mm_initialize( "freetype", buffer, bufsize );
    DEBUGASSERT( heap != NULL );

    memex = mm_malloc( heap, sizeof ( FT_MemoryEx ) );
    DEBUGASSERT( memex != NULL );

    memex->buffer = buffer;

    memory = (FT_Memory)memex;

    memory->user = heap;
#else
    memory = (FT_Memory)malloc( sizeof ( *memory ) );
    DEBUGASSERT( memory != NULL );

    memory->user = NULL;
#endif

    memory->alloc   = ft_alloc;
    memory->realloc = ft_realloc;
    memory->free    = ft_free;
#ifdef FT_DEBUG_MEMORY
    ft_mem_debug_init( memory );
#endif

    return memory;
  }


  /* documentation is in ftobjs.h */

  FT_BASE_DEF( void )
  FT_Done_Memory( FT_Memory  memory )
  {
#ifdef FT_DEBUG_MEMORY
    ft_mem_debug_done( memory );
#endif

#if CONFIG_LIB_FREETYPE_MEMSIZE
    FT_MemoryEx* memex = (FT_MemoryEx*)memory;
    struct mm_heap_s* heap = memory->user;
    void* buffer = memex->buffer;

    mm_uninitialize( heap );
    free( buffer );
#else
    free( memory );
#endif
  }


/* END */

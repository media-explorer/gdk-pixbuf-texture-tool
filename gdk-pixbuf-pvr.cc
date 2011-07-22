/*
 * gdk-pixbuf-texture-tool - Play with texture files
 *
 * Copyright © 2011 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>
 *
 * Authors: Damien Lespiau <damien.lespiau@intel.com>
 *
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define GDK_PIXBUF_ENABLE_BACKEND

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gdk-pixbuf-pvr.h"
#include "PVRTexLib.h"
using namespace pvrtexlib;

typedef struct
{
  CPVRTexture *decompressed;
} PvrContext;

static const gchar *
standard_pixel_type_to_string (PixelType pixel_type)
{
  switch (pixel_type)
    {
    case eInt8StandardPixelType:
      return "R8G8B8A8";
    case eInt16StandardPixelType:
      return "A16B16G16R16";
    case eInt32StandardPixelType:
      return "R32G32B32A32";
    case eFloatStandardPixelType:
      return "R32G32B32A32 (float)";

    /* should not happen as we should only be given standard pixel types */
    default:
        return "Other";
    }
}

static void
on_pixbuf_destroyed (guchar *pixels,
                     gpointer data)
{
  PvrContext *context = (PvrContext *) data;

  g_message ("destroyed");
  delete context->decompressed;
  g_free (context);
}

static GdkPixbuf *
pvrtexlib_gdk_pixbuf_new_from_memory (const guchar  *data,
                                      GError       **error)
{
  CPVRTexture *decompressed;
  PvrContext *context;
  GdkPixbuf *pixbuf;

  PVRTRY
    {
      PVRTextureUtilities utils;
      PixelType pixel_type;

      CPVRTexture compressed (data);

      decompressed = new CPVRTexture();
      utils.DecompressPVR (compressed, *decompressed);

      pixel_type = decompressed->getPixelType ();
      if (pixel_type != eInt8StandardPixelType)
        {
          const gchar *type;

          type = standard_pixel_type_to_string (pixel_type);
          g_set_error (error,
                       GDK_PIXBUF_ERROR,
                       GDK_PIXBUF_ERROR_FAILED,
                       "Image type currently not supported (%s)", type);

          return NULL;

        }

      CPVRTextureData& data = decompressed->getData();

      /* FIXME: The data resulting of the decompression will always have alpha.
       * might worth repacking the pixbuf to RGB if the original texture did
       * not have any alpha before handing the pixbuf back to the user */

      context = g_new0 (PvrContext, 1);
      context->decompressed = decompressed;

      pixbuf = gdk_pixbuf_new_from_data (data.getData(),
                                         GDK_COLORSPACE_RGB,
                                         TRUE,
                                         8,
                                         decompressed->getWidth (),
                                         decompressed->getHeight (),
                                         decompressed->getWidth () * 4,
                                         on_pixbuf_destroyed,
                                         context);

      if (compressed.isFlipped ())
        {
          GdkPixbuf *flipped;

          flipped = gdk_pixbuf_flip (pixbuf, FALSE);
          g_object_unref (pixbuf);
          pixbuf = flipped;
        }
    }
  PVRCATCH(aaaahhh)
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_FAILED,
                           aaaahhh.what());

      return NULL;
    }

  return pixbuf;
}

static GdkPixbuf *
gdk_pixbuf__pvr_image_load (FILE    *f,
                            GError **error)
{
  GdkPixbuf *pixbuf;
  guchar *content;
  GError *decompress_error = NULL;
  struct stat st;
  int fd;

  fd = fileno (f);
  if (fd == -1)
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_FAILED,
                           "Invalid FILE object");
      return NULL;
    }

  if (fstat (fd, &st) == -1)
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_FAILED,
                           "Failed to get attributes");
      return NULL;

    }

  if (st.st_size == 0 || st.st_size > G_MAXSIZE)
    {
      content = NULL;
    }
  else
    {
      content = (unsigned char *) mmap (NULL, st.st_size, PROT_READ,
                                        MAP_PRIVATE, fd, 0);
    }

  if (content == NULL || content == MAP_FAILED)
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_FAILED,
                           "Failed to map file");
      return NULL;
    }

  pixbuf = pvrtexlib_gdk_pixbuf_new_from_memory (content,
                                                 &decompress_error);
  if (decompress_error)
    {
      g_propagate_error (error, decompress_error);
      return NULL;
    }

  return pixbuf;
}

static bool
is_p2 (unsigned int x)
{
    return ((x != 0) && !(x & (x - 1)));
}

static gboolean
parse_format (const gchar *format,
              PixelType   *type)
{
  if (g_strcmp0 (format, "PVRTC2") == 0)
    {
      *type = OGL_PVRTC2;
      return TRUE;
    }
  if (g_strcmp0 (format, "PVRTC4") == 0)
    {
      *type = OGL_PVRTC4;
      return TRUE;
    }
  if (g_strcmp0 (format, "ETC1") == 0)
    {
      *type = ETC_RGB_4BPP;
      return TRUE;
    }

  *type = ETC_RGB_4BPP;
  return FALSE;
}

static gboolean
validate_pixbuf_pvrtc (GdkPixbuf  *pixbuf,
                       GError    **error)
{
  int width, height;

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  if (!is_p2 (width))
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_FAILED,
                           "Width needs to be a power of 2");
      return FALSE;
    }

  if (!is_p2 (height))
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_FAILED,
                           "Height needs to be a power of 2");
      return FALSE;
    }

  return TRUE;
}

static gboolean
gdk_pixbuf__pvr_image_save (FILE       *f,
                            GdkPixbuf  *pixbuf,
                            gchar     **param_keys,
                            gchar     **param_values,
                            GError    **error_out)
{
  GdkPixbuf *with_alpha = NULL;
  PixelType opt_format = ETC_RGB_4BPP;
  gboolean valid;
  GError *error = NULL;

  /* parse the parameters */
  if (param_keys)
    {
      gchar **key_p = param_keys, **value_p = param_values;

      while (*key_p)
        {
          if (g_strcmp0 (*key_p, "format") == 0)
            {
              if (!parse_format (*value_p, &opt_format))
                {
                  g_set_error (error_out,
                               GDK_PIXBUF_ERROR,
                               GDK_PIXBUF_ERROR_FAILED,
                               "Invalid format %s", *value_p);
                  return FALSE;
                }
            }
          else
            {
              g_warning ("Unknown option %s", *key_p);
            }

          key_p++;
          value_p++;
        }
    }

  /* validate the pixbuf if needed */
  switch (opt_format)
    {
    case OGL_PVRTC2:
    case OGL_PVRTC4:
      valid = validate_pixbuf_pvrtc (pixbuf, &error);
      break;
    default:
      valid = TRUE;
    }

  if (!valid)
    {
      g_propagate_error (error_out, error);
      return FALSE;
    }

  PVRTRY
    {
      PVRTextureUtilities utils;
      int width, height;
      guchar *pixels;

      width = gdk_pixbuf_get_width (pixbuf);
      height = gdk_pixbuf_get_height (pixbuf);

      /* The standard format that PVRTexLib takes is RGBA 8888 so we need
       * to add an alpha channel if the original image does not have one */
      if (!gdk_pixbuf_get_has_alpha (pixbuf))
        {
          with_alpha = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);
          pixbuf = with_alpha;
        }

      if (gdk_pixbuf_get_rowstride (pixbuf) != 4 * width)
        {
          g_set_error_literal (error_out,
                               GDK_PIXBUF_ERROR,
                               GDK_PIXBUF_ERROR_FAILED,
                               "A row stride larger than the width is not "
                               "allowed");
          if (with_alpha)
            g_object_unref (with_alpha);

          return FALSE;
        }

      /* make a CPVRTexture instance from the GdkPixbuf */
      pixels = gdk_pixbuf_get_pixels (pixbuf);
      CPVRTexture uncompressed (width,
                               height,
                               0,                       /* u32MipMapCount */
                               1,                       /* u32NumSurfaces */
                               false,                   /* bBorder */
                               false,                   /* bTwiddled */
                               false,                   /* bCubeMap */
                               false,                   /* bVolume */
                               false,                   /* bFalseMips */
                               true,                    /* bHasAlpha */
                               false,                   /* bFlipped */
                               eInt8StandardPixelType,  /* ePixelType */
                               0.0f,                    /* fNormalMap */
                               pixels);                 /* pPixelData */

      /* create texture to encode to */
      CPVRTexture compressed (uncompressed.getHeader());

      /* FIXME: Remove the alpha channel from the compressed texture is the
       * original GdkPixbuf does not have alpha (But we still need to create
       * the uncompressed texture with hasAlpha to TRUE as the pixbuf is 4
       * bytes per pixel anyway */

      /* TODO: Add support for generating the mipmaps */

      /* TODO: Add support for selection with compression we want */

      /* set required encoded pixel type */
      compressed.setPixelType (opt_format);

      /* encode texture */
      utils.CompressPVR (uncompressed, compressed);

      /* write to file */
      compressed.getHeader().writeToFile (f);
      compressed.getData().writeToFile (f);
    }
  PVRCATCH(aaaahhh)
    {
      g_set_error_literal (error_out,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_FAILED,
                           aaaahhh.what());

      if (with_alpha)
        g_object_unref (with_alpha);

      return FALSE;
    }

  if (with_alpha)
    g_object_unref (with_alpha);

  return TRUE;
}

/*
 * Note: The library provided by Imagination does not seem to provide anything
 * that would allow us to support "incremental loading". What we can do though
 * is to fake incremental loading by accumulating the data inside an array and
 * decode it at the end. This allows us to work with GdkPixbufLoader (which a
 * lot of apps use) at the very least we can signal the size of the level 0
 * mipmap after having read the header.
 */

typedef struct
{
  GdkPixbufModuleSizeFunc size_func;
  GdkPixbufModulePreparedFunc prepared_func;
  GdkPixbufModuleUpdatedFunc updated_func;
  gpointer user_data;

  GArray *buffer;

  guint got_header : 1;
} PvrIncContext;

static gpointer
gdk_pixbuf__pvr_begin_load (GdkPixbufModuleSizeFunc       size_func,
                            GdkPixbufModulePreparedFunc   prepared_func,
                            GdkPixbufModuleUpdatedFunc    updated_func,
                            gpointer                      user_data,
                            GError                      **error)
{
  PvrIncContext *context;

  context = g_new0 (PvrIncContext, 1);
  context->size_func = size_func;
  context->prepared_func = prepared_func;
  context->updated_func  = updated_func;
  context->user_data = user_data;

  /* initialized so can contain a 512x512 image with 4 bits per pixel without
   * the need to reallocate the buffer */
  context->buffer = g_array_sized_new (FALSE, FALSE, 1, 512 * 512 / 2);

  return context;
}

static gboolean
gdk_pixbuf__pvr_stop_load (gpointer   contextp,
                           GError   **error)
{
  PvrIncContext *context = (PvrIncContext *) contextp;
  GdkPixbuf *pixbuf;
  GError *decompress_error = NULL;

  pixbuf =
    pvrtexlib_gdk_pixbuf_new_from_memory ((guchar *)context->buffer->data,
                                          &decompress_error);
  if (decompress_error)
    {
      g_propagate_error (error, decompress_error);
      return FALSE;
    }

  if (context->prepared_func)
    context->prepared_func (pixbuf, NULL, context->user_data);

  g_free (context);

  return TRUE;
}

static gboolean
gdk_pixbuf__pvr_load_increment (gpointer       contextp,
                                const guchar  *buf,
                                guint          size,
                                GError       **error)
{
  PvrIncContext *context = (PvrIncContext *) contextp;

  g_array_append_vals (context->buffer, buf, size);

  if (!context->got_header && context->buffer->len >= sizeof (PVRHeader))
    {
      context->got_header = TRUE;

      if (context->size_func)
        {
          gint width, height;
          PVRHeader *header;

          header = (PVRHeader *) context->buffer->data;
          width = header->width;
          height = header->height;

          (*context->size_func) (&width, &height, context->user_data);

          if (width == 0 || height == 0)
            {
              /* used to signal we are going to stop loading the image, return
               * an error for good measure and not fall in the case we return
               * FALSE without an error set */
              g_set_error_literal (error,
                                   GDK_PIXBUF_ERROR,
                                   GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
                                   "Zero width or height requested");
              return FALSE;
            }
        }
    }

  return TRUE;
}

extern "C" {

G_MODULE_EXPORT void
fill_vtable (GdkPixbufModule *module)
{
  module->load = gdk_pixbuf__pvr_image_load;
  module->save = gdk_pixbuf__pvr_image_save;

  module->begin_load = gdk_pixbuf__pvr_begin_load;
  module->stop_load = gdk_pixbuf__pvr_stop_load;
  module->load_increment = gdk_pixbuf__pvr_load_increment;
}

G_MODULE_EXPORT void
fill_info (GdkPixbufFormat *info)
{
  static GdkPixbufModulePattern signature_new[] = {
        { NULL, NULL, 0 }
  };

  static gchar *mime_types[] =
    {
      "image/x-pvr",
      NULL
    };
  static gchar *extensions[] =
    {
      "pvr",
      NULL
    };

  info->name        = "pvr";
  info->description = "PVR image";
  info->signature = signature_new;
  info->mime_types  = mime_types;
  info->extensions  = extensions;
  info->flags       = GDK_PIXBUF_FORMAT_WRITABLE;
  info->license     = "LGPL";
}

} /* extern "C" */

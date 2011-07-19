/*
 * gdk-pixbuf-pvr - A gdk-pixbuf loader for PVR textures
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

#include "PVRTexLib.h"

using namespace pvrtexlib;

/*
 * Note: The library provided by Imagination does not seem to provide anything
 * that would allow us to support "incremental loading".
 */

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

  delete context->decompressed;
  g_free (context);
}

static GdkPixbuf *
gdk_pixbuf__pvr_image_load (FILE    *f,
                            GError **error)
{
  GdkPixbuf *pixbuf;
  unsigned char *content;
  CPVRTexture *decompressed;
  PvrContext *context;
  struct stat st;
  int fd;

  PVRTRY
    {
      PVRTextureUtilities utils;
      PixelType pixel_type;

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

      CPVRTexture compressed (content);

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

      context = g_new0 (PvrContext, 1);
      context->decompressed = decompressed;

      /* FIXME: The data resulting of the decompression will always have alpha.
       * might worth repacking the pixbuf to RGB if the original texture did
       * not have any alpha before handing the pixbuf back to the user */

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

bool is_p2 (unsigned int x)
{
    return ((x != 0) && !(x & (x - 1)));
}

static gboolean
gdk_pixbuf__pvr_image_save (FILE       *f,
                            GdkPixbuf  *pixbuf,
                            gchar     **param_keys,
                            gchar     **param_values,
                            GError    **error)
{
  GdkPixbuf *with_alpha = NULL;

  PVRTRY
    {
      PVRTextureUtilities utils;
      int width, height;
      guchar *pixels;

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

      /* The standard format that PVRTexLib takes is RGBA 8888 so we need
       * to add an alpha channel if the original image does not have one */
      if (!gdk_pixbuf_get_has_alpha (pixbuf))
        {
          with_alpha = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);
          pixbuf = with_alpha;
        }

      if (gdk_pixbuf_get_rowstride (pixbuf) != 4 * width)
        {
          g_set_error_literal (error,
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
      compressed.setPixelType (OGL_PVRTC4);

      /* encode texture */
      utils.CompressPVR (uncompressed, compressed);

      /* write to file */
      compressed.getHeader().writeToFile (f);
      compressed.getData().writeToFile (f);
    }
  PVRCATCH(aaaahhh)
    {
      g_set_error_literal (error,
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

extern "C" {

G_MODULE_EXPORT void
fill_vtable (GdkPixbufModule *module)
{
  module->load = gdk_pixbuf__pvr_image_load;
  module->save = gdk_pixbuf__pvr_image_save;
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

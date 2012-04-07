/*
 * gdk-pixbuf-texture-tool - Play with texture files
 *
 * Copyright © 2011 Intel Corporation
 *
 * This software is licensed under the BSD 3-Clause license. See the COPYING
 * file for the full text of the license.
 *
 * Authors: Damien Lespiau <damien.lespiau@intel.com>
 *
 */

#include <stdlib.h>
#include <string.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#define FORMAT_ETC1       0
#define FORMAT_PRVTC2     1
#define FORMAT_PRVTC4     2

const char *formats[] =
{
  "ETC1",
  "PVRTC2",
  "PVRTC4"
};

static gchar *opt_output = "output.pvr";
static gchar *opt_format = "ETC1";
static gboolean opt_list_formats = FALSE;
static gchar **opt_files;

static GOptionEntry entries[] =
{
  { "format", 'f', 0, G_OPTION_ARG_STRING, &opt_format,
    "Select the output format", NULL },
  { "list-formats", 0, 0, G_OPTION_ARG_NONE, &opt_list_formats,
    "List the valid formats", NULL },
  { "output", 'o', 0, G_OPTION_ARG_STRING, &opt_output,
    "Give the output file name", NULL },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &opt_files,
    "input files...", NULL },
  { NULL }
};

static gboolean
validate_format (const gchar *format)
{
  gint i;

  for (i = 0; i < G_N_ELEMENTS (formats); i++)
    {
      if (strcmp (format, formats[i]) == 0)
          return TRUE;
    }

  return FALSE;
}

static void
do_list_formats (void)
{
  guint i;

  g_print ("Known formats:\n");
  for (i = 0; i < G_N_ELEMENTS (formats); i++)
    g_print ("  %s\n", formats[i]);
}

static gboolean
do_compress_file (gchar *filename)
{
  GdkPixbuf *source;
  GError *error = NULL;
  gboolean success = TRUE;

  source = gdk_pixbuf_new_from_file (filename, &error);
  if (error)
    {
      g_print ("Could not open file %s: %s\n", filename, error->message);
      success = FALSE;
      goto open_failed;
    }

  gdk_pixbuf_save (source, opt_output, "pvr", &error,
                   "format", opt_format,
                   NULL);
  if (error)
    {
      g_print ("Could not save file %s: %s\n", opt_output, error->message);
      success = FALSE;
      goto save_failed;
    }

save_failed:
  g_object_unref (source);
open_failed:
  return success;
}

int
main(int   argc,
     char *argv[])
{
  GError *error = NULL;
  GOptionContext *context;
  gboolean success = TRUE;
  guint i;

  g_type_init ();

  context = g_option_context_new ("- A tool to manipulate textures");

  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("Failed to parse options: %s\n", error->message);
      return EXIT_FAILURE;
    }

  if (opt_list_formats)
    {
      do_list_formats ();
      return EXIT_SUCCESS;
    }

  if (!validate_format (opt_format))
    {
      g_printerr ("Invalid format '%s'\n", opt_format);
      return EXIT_FAILURE;
    }

  if (opt_files == NULL)
    {
      g_printerr ("You need to give at least one file to operate on\n");
      return EXIT_FAILURE;
    }

  for (i = 0; opt_files[i]; i++)
    {
      success = do_compress_file (opt_files[i]);
    }

  return !success;
}

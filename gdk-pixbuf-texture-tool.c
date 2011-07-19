#include <stdlib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

static gchar *opt_output = "output.pvr";

int
main(int argc, const char *argv[])
{
  GdkPixbuf *source;
  GError *error = NULL;
  const gchar *filename = argv[1];

  g_type_init ();

  source = gdk_pixbuf_new_from_file (filename, &error);
  if (error)
    {
      g_print ("Could not open file %s: %s\n", filename, error->message);
      return EXIT_FAILURE;

    }

  gdk_pixbuf_save (source, opt_output, "pvr", &error, NULL);
  if (error)
    {
      g_print ("Could not save file %s: %s\n", opt_output, error->message);
    }

  return 0;
}

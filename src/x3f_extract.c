/* X3F_EXTRACT.C - Extracting images from X3F files
 * 
 * Copyright 2010 (c) - Roland Karlsson (roland@proxel.se)
 * BSD-style - see doc/copyright.txt
 * 
 */

#include "x3f_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {RAW, TIFF, PPMP3, PPMP6, HISTOGRAM} raw_file_type_t;

static void usage(char *progname)
{
  fprintf(stderr,
          "usage: %s [-jpg]"
          " [{-raw|-tiff [-gamma <GAMMA> [-min <MIN>] [-max <MAX>]]}]"
          " <file1> ...\n"
          "   -jpg:       Dump embedded JPG. Turn off RAW dumping\n"
          "   -raw:       Dump RAW area undecoded\n"
          "   -tiff:      Dump RAW as 3x16 bit TIFF (default)\n"
          "   -ppm-ascii: Dump RAW as 3x16 bit PPM of type P3 (ascii)\n"
          "               NOTE: 16 bit PPM/P3 is not generally supported\n"
          "   -ppm:       Dump RAW as 3x16 bit PPM of type P6 (binary)\n"
          "   -histogram: Dump histogram as csv file\n"
          "   -loghist:   Dump histogram as csv file, with log exposure\n"
          "   -gamma <GAMMA>:  Gamma for scaled PPM/TIFF (def=-1.0 (off))\n"
          "   -min <MIN>:      Min for scaled PPM/TIFF (def=automatic)\n"
          "   -max <MAX>:      Max for scaled PPM/TIFF (def=automatic)\n",
          progname);
  exit(1);
}

int main(int argc, char *argv[])
{
  int extract_jpg = 0;
  int extract_raw = 1;
  int min = -1;
  int max = -1;
  double gamma = -1.0;
  raw_file_type_t file_type = TIFF;
  int files = 0;
  int log_hist = 0;

  int i;

  for (i=1; i<argc; i++)
    if (!strcmp(argv[i], "-jpg"))
      extract_raw = 0, extract_jpg = 1;
    else if (!strcmp(argv[i], "-raw"))
      extract_raw = 1, file_type = RAW;
    else if (!strcmp(argv[i], "-tiff"))
      extract_raw = 1, file_type = TIFF;
    else if (!strcmp(argv[i], "-ppm-ascii"))
      extract_raw = 1, file_type = PPMP3;
    else if (!strcmp(argv[i], "-ppm"))
      extract_raw = 1, file_type = PPMP6;
    else if (!strcmp(argv[i], "-histogram"))
      extract_raw = 1, file_type = HISTOGRAM;
    else if (!strcmp(argv[i], "-loghist"))
      extract_raw = 1, file_type = HISTOGRAM, log_hist = 1;
    else if ((!strcmp(argv[i], "-gamma")) && (i+1)<argc)
      gamma = atof(argv[++i]);
    else if ((!strcmp(argv[i], "-min")) && (i+1)<argc)
      min = atoi(argv[++i]);
    else if ((!strcmp(argv[i], "-max")) && (i+1)<argc)
      max = atoi(argv[++i]);
    else if (!strncmp(argv[i], "-", 1))
      usage(argv[0]);
    else
      break;			/* Here starts list of files */

  /* If min or max is set but no gamma - ERROR */
  if (min != -1 || max != -1)
    if (gamma <= 0.0)
      usage(argv[0]);

  /* If gamma is set but no file type that can output gamma - ERROR */
  if (gamma > 0.0)
    if (file_type != TIFF && file_type != PPMP3 && file_type != PPMP6)
      usage(argv[0]);

  for (; i<argc; i++) {
    char *infilename = argv[i];
    FILE *f_in = fopen(infilename, "rb");
    x3f_t *x3f = NULL;

    files++;

    if (f_in == NULL) {
      fprintf(stderr, "Could not open infile %s\n", infilename);
      goto clean_up;
    }

    printf("READ THE X3F FILE %s\n", infilename);
    x3f = x3f_new_from_file(f_in);

    if (x3f == NULL) {
      fprintf(stderr, "Could not read infile %s\n", infilename);
      goto clean_up;
    }

    if (extract_jpg) {
      char outfilename[1024];

      x3f_load_data(x3f, x3f_get_thumb_jpeg(x3f));

      strcpy(outfilename, infilename);
      strcat(outfilename, ".jpg");

      printf("Dump JPEG to %s\n", outfilename);
      if (X3F_OK != x3f_dump_jpeg(x3f, outfilename))
        fprintf(stderr, "Could not dump JPEG to %s\n", outfilename);
    }

    if (extract_raw) {
      char outfilename[1024];
      x3f_return_t ret_dump = X3F_OK;

      printf("Load RAW block from %s\n", infilename);
      if (file_type == RAW) {
        if (X3F_OK != x3f_load_image_block(x3f, x3f_get_raw(x3f))) {
          fprintf(stderr, "Could not load unconverted RAW from memory\n");
          goto clean_up;
        }
      } else {
        if (X3F_OK != x3f_load_data(x3f, x3f_get_raw(x3f))) {
          fprintf(stderr, "Could not load RAW from memory\n");
          goto clean_up;
        }
      }

      strcpy(outfilename, infilename);

      switch (file_type) {
      case RAW:
	strcat(outfilename, ".raw");
	printf("Dump RAW block to %s\n", outfilename);
	ret_dump = x3f_dump_raw_data(x3f, outfilename);
	break;
      case TIFF:
	strcat(outfilename, ".tif");
	printf("Dump RAW as TIFF to %s\n", outfilename);
	ret_dump = x3f_dump_raw_data_as_tiff(x3f, outfilename, gamma, min, max);
	break;
      case PPMP3:
      case PPMP6:
	strcat(outfilename, ".ppm");
	printf("Dump RAW as PPM to %s\n", outfilename);
	ret_dump = x3f_dump_raw_data_as_ppm(x3f, outfilename, gamma, min, max,
                                            file_type == PPMP6);
      case HISTOGRAM:
	strcat(outfilename, ".csv");
	printf("Dump RAW as CSV histogram to %s\n", outfilename);
	ret_dump = x3f_dump_raw_data_as_histogram(x3f, outfilename, log_hist);
	break;
      }

      if (X3F_OK != ret_dump)
        fprintf(stderr, "Could not dump RAW to %s\n", outfilename);
    }

  clean_up:

    x3f_delete(x3f);

    fclose(f_in);
  }

  if (files == 0)
    usage(argv[0]);

  return 0;
}

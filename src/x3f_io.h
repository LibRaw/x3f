/* X3F_IO.H - Library for accessing X3F Files.
 * 
 * Copyright (c) 2010 - Roland Karlsson (roland@proxel.se)
 * BSD-style - see doc/copyright.txt
 * 
 */

/* NOTE about endianess. We assume that the X3F file is little
   endian. All multi byte (eg uint32) elements are stored in the
   native endian of the machine. Byte streams are stored as found in
   the file, i.e. little endian. Multi byte streams are stored each
   multi byte element in the native endian - but the stream is stored
   little endian. */


#ifndef X3F_IO_H
#define X3F_IO_H

#include <inttypes.h>
#include <stdio.h>

#define SIZE_UNIQUE_IDENTIFIER 16
#define SIZE_WHITE_BALANCE 32
#define NUM_EXT_DATA 32

#define X3F_VERSION(MAJ,MIN) (uint32_t)(((MAJ)<<16) + MIN)
#define X3F_VERSION_2_0 X3F_VERSION(2,0)
#define X3F_VERSION_2_1 X3F_VERSION(2,1)

/* Main file identifier */
#define X3F_FOVb (uint32_t)(0x62564f46)
/* Directory identifier */
#define X3F_SECd (uint32_t)(0x64434553)
/* Property section identifiers */
#define X3F_PROP (uint32_t)(0x504f5250)
#define X3F_SECp (uint32_t)(0x70434553)
/* Image section identifiers */
#define X3F_IMAG (uint32_t)(0x46414d49)
#define X3F_IMA2 (uint32_t)(0x32414d49)
#define X3F_SECi (uint32_t)(0x69434553)
/* CAMF identifiers */
#define X3F_CAMF (uint32_t)(0x464d4143)
#define X3F_SECc (uint32_t)(0x63434553)
/* CAMF entry identifiers */
#define X3F_CMbP (uint32_t)(0x50624d43)
#define X3F_CMbT (uint32_t)(0x54624d43)
#define X3F_CMbM (uint32_t)(0x4d624d43)
#define X3F_CMb  (uint32_t)(0x00624d43)

/* TODO: bad name */
#define X3F_IMAGE_RAW_TRUE_SD1      (uint32_t)(0x0001001e)

/* TODO: bad name */
#define X3F_IMAGE_RAW_HUFFMAN_X530  (uint32_t)(0x00030005)

#define X3F_IMAGE_RAW_TRUE          (uint32_t)(0x0003001e)
#define X3F_IMAGE_RAW_HUFFMAN_10BIT (uint32_t)(0x00030006)
#define X3F_IMAGE_THUMB_PLAIN       (uint32_t)(0x00020003)
#define X3F_IMAGE_THUMB_HUFFMAN     (uint32_t)(0x0002000b)
#define X3F_IMAGE_THUMB_JPEG        (uint32_t)(0x00020012)

#define X3F_IMAGE_HEADER_SIZE 28
#define X3F_CAMF_HEADER_SIZE 28
#define X3F_PROPERTY_LIST_HEADER_SIZE 24

typedef uint16_t utf16_t;

typedef int bool_t;

typedef enum x3f_extended_types_e {
  X3F_EXT_TYPE_NONE=0,
  X3F_EXT_TYPE_EXPOSURE_ADJUST=1,
  X3F_EXT_TYPE_CONTRAST_ADJUST=2,
  X3F_EXT_TYPE_SHADOW_ADJUST=3,
  X3F_EXT_TYPE_HIGHLIGHT_ADJUST=4,
  X3F_EXT_TYPE_SATURATION_ADJUST=5,
  X3F_EXT_TYPE_SHARPNESS_ADJUST=6,
  X3F_EXT_TYPE_RED_ADJUST=7,
  X3F_EXT_TYPE_GREEN_ADJUST=8,
  X3F_EXT_TYPE_BLUE_ADJUST=9,
  X3F_EXT_TYPE_FILL_LIGHT_ADJUST=10
} x3f_extended_types_t;

typedef struct x3f_property_s {
  /* Read from file */
  uint32_t name_offset;
  uint32_t value_offset;

  /* Computed */
  utf16_t *name;		/* 0x00 terminated UTF 16 */
  utf16_t *value;               /* 0x00 terminated UTF 16 */
} x3f_property_t;

typedef struct x3f_property_table_s {
  uint32_t size;
  x3f_property_t *element;
} x3f_property_table_t;

typedef struct x3f_property_list_s {
  /* 2.0 Fields */
  uint32_t num_properties;
  uint32_t character_format;
  uint32_t reserved;
  uint32_t total_length;

  x3f_property_table_t property_table;

  void *data;

  uint32_t data_size;

} x3f_property_list_t;

typedef struct x3f_table8_s {
  uint32_t size;
  uint8_t *element;
} x3f_table8_t;

typedef struct x3f_table16_s {
  uint32_t size;
  uint16_t *element;
} x3f_table16_t;

typedef struct x3f_table32_s {
  uint32_t size;
  uint32_t *element;
} x3f_table32_t;

#define UNDEFINED_LEAF 0xffffffff

typedef struct x3f_huffnode_s {
  struct x3f_huffnode_s *branch[2];
  uint32_t leaf;
} x3f_huffnode_t;

typedef struct x3f_hufftree_s {
  uint32_t free_node_index; /* Free node index in huffman tree array */
  x3f_huffnode_t *nodes;    /* Coding tree */
} x3f_hufftree_t;

typedef struct x3f_true_huffman_element_s {
  uint8_t code_size;
  uint8_t code;
} x3f_true_huffman_element_t;

typedef struct x3f_true_huffman_s {
  uint32_t size;
  x3f_true_huffman_element_t *element;
} x3f_true_huffman_t;

/* TODO: is this a constant? */
#define TRUE_PLANES 3

typedef struct x3f_true_s {
  uint16_t seed[3];		/* Always 512,512,512 */
  uint16_t unknown;		/* Always 0 */
  x3f_true_huffman_t table;	/* Huffman table - zero
				   terminated. size is the number of
				   leaves plus 1.*/

  x3f_table32_t plane_size;	/* Size of the 3 planes */
  uint8_t *plane_address[TRUE_PLANES]; /* computed offset to the planes */
  x3f_hufftree_t tree;		/* Coding tree */
  x3f_table16_t x3rgb16;        /* 3x16 bit X3-RGB data */
} x3f_true_t;

typedef struct x3f_huffman_s {
  x3f_table16_t mapping;   /* Value Mapping = X3F lossy compression */
  x3f_table32_t table;          /* Coding Table */
  x3f_hufftree_t tree;		/* Coding tree */
  x3f_table32_t row_offsets;    /* Row offsets */
  x3f_table8_t rgb8;            /* 3x8 bit RGB data */
  x3f_table16_t x3rgb16;        /* 3x16 bit X3-RGB data */
} x3f_huffman_t;

typedef struct x3f_image_data_s {
  /* 2.0 Fields */
  /* ------------------------------------------------------------------ */
  /* Known combinations of type and format are:
     1-6, 2-3, 2-11, 2-18, 3-6 */
  uint32_t type;                /* 1 = RAW X3 (SD1)
                                   2 = thumbnail or maybe just RGB
                                   3 = RAW X3 */
  uint32_t format;              /* 3 = 3x8 bit pixmap
                                   6 = 3x10 bit huffman with map table
                                   11 = 3x8 bit huffman
                                   18 = JPEG */
  uint32_t type_format;         /* type<<16 + format */
  /* ------------------------------------------------------------------ */
  uint32_t columns;             /* width / row size in pixels */
  uint32_t rows;                /* height */
  uint32_t row_stride;          /* row size in bytes */

  x3f_huffman_t *huffman;       /* Huffman help data */

  x3f_true_t *tru;		/* TRUE coding help data */

  void *data;                   /* Take from file if NULL. Otherwise,
                                   this is the actual data bytes in
                                   the file. */
  uint32_t data_size;

} x3f_image_data_t;

typedef struct camf_entry_s {
  uint32_t id;
  uint32_t version;
  uint32_t entry_size;
  uint32_t name_offset;
  uint32_t value_offset;
  void *entry;			/* pointer into decoded data */

  /* computed values */
  uint8_t *name_address;
  void *value_address;
} camf_entry_t;

typedef struct camf_entry_table_s {
  uint32_t size;
  camf_entry_t *element;
} camf_entry_table_t;

typedef struct x3f_camf_typeN_s {
  uint32_t val0;
  uint32_t val1;
  uint32_t val2;
  uint32_t val3;
} x3f_camf_typeN_t;

typedef struct x3f_camf_type2_s {
  uint32_t reserved;
  uint32_t infotype;
  uint32_t infotype_version;
  uint32_t crypt_key;
} x3f_camf_type2_t;

typedef struct x3f_camf_type4_s {
  uint32_t reserved;
  uint32_t decode_bias;
  uint32_t block_size;
  uint32_t block_count;
} x3f_camf_type4_t;

typedef struct x3f_camf_s {

  /* Header info */
  uint32_t type;
  union {
    x3f_camf_typeN_t tN;
    x3f_camf_type2_t t2;
    x3f_camf_type4_t t4;
  };

  /* The encrypted raw data */
  void *data;
  uint32_t data_size;

  /* Help data for type 4 Huffman compression */
  x3f_true_huffman_t table;
  x3f_hufftree_t tree;
  uint8_t *decoding_start;

  /* The decrypted data */
  void *decoded_data;
  uint32_t decoded_data_size;

  /* Pointers into the decrypted data */
  camf_entry_table_t entry_table;
} x3f_camf_t;

typedef struct x3f_directory_entry_header_s {
  uint32_t identifier;        /* Should be �SECp�, "SECi", ... */
  uint32_t version;           /* 0x00020001 is version 2.1  */
  union {
    x3f_property_list_t property_list;
    x3f_image_data_t image_data;
    x3f_camf_t camf;
  } data_subsection;
} x3f_directory_entry_header_t;

typedef struct x3f_directory_entry_s {
  struct {
    uint32_t offset;
    uint32_t size;
  } input, output;

  uint32_t type;

  x3f_directory_entry_header_t header;
} x3f_directory_entry_t;

typedef struct x3f_directory_section_s {
  uint32_t identifier;          /* Should be �SECd� */
  uint32_t version;             /* 0x00020001 is version 2.1  */

  /* 2.0 Fields */
  uint32_t num_directory_entries;
  x3f_directory_entry_t *directory_entry;
} x3f_directory_section_t;

typedef struct x3f_header_s {
  /* 2.0 Fields */
  uint32_t identifier;          /* Should be �FOVb� */
  uint32_t version;             /* 0x00020001 means 2.1 */
  uint8_t unique_identifier[SIZE_UNIQUE_IDENTIFIER];
  uint32_t mark_bits;
  uint32_t columns;             /* Columns and rows ... */
  uint32_t rows;                /* ... before rotation */
  uint32_t rotation;            /* 0, 90, 180, 270 */

  /* Added for 2.1 and 2.2 */
  uint8_t white_balance[SIZE_WHITE_BALANCE];
  uint8_t extended_types[NUM_EXT_DATA]; /* x3f_extended_types_t */
  uint32_t extended_data[NUM_EXT_DATA];
} x3f_header_t;

typedef struct x3f_info_s {
  char *error;
  struct {
    FILE *file;                 /* Use if more data is needed */
  } input, output;
} x3f_info_t;

typedef struct x3f_s {
  x3f_info_t info;
  x3f_header_t header;
  x3f_directory_section_t directory_section;
} x3f_t;

typedef enum x3f_return_e {
  X3F_OK=0,
  X3F_ARGUMENT_ERROR=1,
  X3F_INFILE_ERROR=2,
  X3F_OUTFILE_ERROR=3,
  X3F_INTERNAL_ERROR=4
} x3f_return_t;

extern x3f_t *x3f_new_from_file(FILE *infile);

extern void x3f_print(x3f_t *x3f);

extern x3f_return_t x3f_write_to_file(x3f_t *x3f, FILE *outfile);

extern x3f_return_t x3f_delete(x3f_t *x3f);

extern x3f_directory_entry_t *x3f_get_raw(x3f_t *x3f);

extern x3f_directory_entry_t *x3f_get_thumb_plain(x3f_t *x3f);

extern x3f_directory_entry_t *x3f_get_thumb_huffman(x3f_t *x3f);

extern x3f_directory_entry_t *x3f_get_thumb_jpeg(x3f_t *x3f);

extern x3f_directory_entry_t *x3f_get_camf(x3f_t *x3f);

extern x3f_directory_entry_t *x3f_get_prop(x3f_t *x3f);

extern x3f_return_t x3f_load_data(x3f_t *x3f, x3f_directory_entry_t *DE);

extern x3f_return_t x3f_load_image_block(x3f_t *x3f, x3f_directory_entry_t *DE);

extern x3f_return_t x3f_swap_images(x3f_t *x3f_template, x3f_t *x3f_images);

extern x3f_return_t x3f_dump_raw_data(x3f_t *x3f, char *outfilename);

extern x3f_return_t x3f_dump_raw_data_as_ppm(x3f_t *x3f, char *outfilename,
                                             double gamma, int min, int max,
                                             int binary);

extern x3f_return_t x3f_dump_raw_data_as_tiff(x3f_t *x3f, char *outfilename,
                                              double gamma, int min, int max);

extern x3f_return_t x3f_dump_raw_data_as_histogram(x3f_t *x3f,
                                                   char *outfilename,
                                                   int log_hist);

extern x3f_return_t x3f_dump_jpeg(x3f_t *x3f, char *outfilename);


#endif /* X3F_IO_H */

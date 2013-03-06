
/* $Id$ */

typedef struct                       /**** BMP file header structure ****/
{
  uint16_t    bfType;           /* Magic number for file */
  uint32_t    bfSize;           /* Size of file */
  uint16_t    bfReserved1;      /* Reserved */
  uint16_t    bfReserved2;      /* ... */
  uint32_t    bfOffBits;        /* Offset to bitmap data */
} BITMAPFILEHEADER;


/* These are represenations of "BM" */
#ifdef WORDS_BIGENDIAN
#define BF_TYPE 0x424d
#else
#define BF_TYPE 0x4D42             
#endif


typedef struct                       /**** BMP file info structure ****/
{
  uint32_t    biSize;           /* Size of info header */
  int32_t    biWidth;          /* Width of image */
  int32_t    biHeight;         /* Height of image */
  uint16_t    biPlanes;         /* Number of color planes */
  uint16_t    biBitCount;       /* Number of bits per pixel */
  uint32_t    biCompression;    /* Type of compression to use */
  uint32_t    biSizeImage;      /* Size of image data */
  int32_t     biXPelsPerMeter;  /* X pixels per meter */
  int32_t     biYPelsPerMeter;  /* Y pixels per meter */
  uint32_t    biClrUsed;        /* Number of colors used */
  uint32_t    biClrImportant;   /* Number of important colors */
} BITMAPINFOHEADER;

/*
 * Constants for the biCompression field...
 */

#define BI_RGB       0             /* No compression - straight BGR data */
#define BI_RLE8      1             /* 8-bit run-length compression */
#define BI_RLE4      2             /* 4-bit run-length compression */
#define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */

typedef struct                       /**** Colormap entry structure ****/
{
  unsigned char  rgbBlue;          /* Blue value */
  unsigned char  rgbGreen;         /* Green value */
  unsigned char  rgbRed;           /* Red value */
  /*  unsigned char  rgbReserved; */      /* Reserved */
} __attribute__ ((packed)) RGBQUAD;

typedef struct                       /**** Bitmap information structure ****/
{
  BITMAPINFOHEADER bmiHeader;      /* Image header */
  RGBQUAD          bmiColors[256]; /* Image colormap */
} BITMAPINFO;

/*
 * gdk-pixbuf-texture-tool - Manipulate texture files
 *
 * Copyright (C) 2011 Intel Corporation.
 *
 * This software is licensed under the BSD 3-Clause license. See the COPYING
 * file for the full text of the license.
 *
 */

#ifndef __GDK_PIXBUF_PVR_H__
#define __GDK_PIXBUF_PVR_H__

typedef struct
{
  guint32 header_size;    /* size of the structure */
  guint32 height;         /* height of surface to be created */
  guint32 width;          /* width of input surface */
  guint32 mipmap_count;   /* number of mip-map levels requested */
  guint32 flags;          /* pixel format flags */
  guint32 data_size;      /* Total size in bytes */
  guint32 bit_count;      /* number of bits per pixel  */
  guint32 red_mask;       /* mask for red bit */
  guint32 green_mask;     /* mask for green bits */
  guint32 blue_mask;      /* mask for blue bits */
  guint32 alpha_mask;     /* mask for alpha channel */
  guint32 PVR;            /* magic number identifying pvr file */
  guint32 n_surfaces;     /* the number of surfaces present in the pvr */
} PVRHeader;

typedef enum
{
  PVR_MGLPT_ARGB_4444 = 0x00,
  PVR_MGLPT_ARGB_1555,
  PVR_MGLPT_RGB_565,
  PVR_MGLPT_RGB_555,
  PVR_MGLPT_RGB_888,
  PVR_MGLPT_ARGB_8888,
  PVR_MGLPT_ARGB_8332,
  PVR_MGLPT_I_8,
  PVR_MGLPT_AI_88,
  PVR_MGLPT_1_BPP,
  PVR_MGLPT_VY1UY0,
  PVR_MGLPT_Y1VY0U,
  PVR_MGLPT_PVRTC2,
  PVR_MGLPT_PVRTC4,

  /* OpenGL version of pixel types */
  PVR_OGL_RGBA_4444= 0x10,
  PVR_OGL_RGBA_5551,
  PVR_OGL_RGBA_8888,
  PVR_OGL_RGB_565,
  PVR_OGL_RGB_555,
  PVR_OGL_RGB_888,
  PVR_OGL_I_8,
  PVR_OGL_AI_88,
  PVR_OGL_PVRTC2,
  PVR_OGL_PVRTC4,
  PVR_OGL_BGRA_8888,
  PVR_OGL_A_8,
  PVR_OGL_PVRTCII4,                             /* Not in use */
  PVR_OGL_PVRTCII2,                             /* Not in use */

  /* S3TC Encoding */
  PVR_D3D_DXT1 = 0x20,
  PVR_D3D_DXT2,
  PVR_D3D_DXT3,
  PVR_D3D_DXT4,
  PVR_D3D_DXT5,

  /* RGB Formats */
  PVR_D3D_RGB_332,
  PVR_D3D_AL_44,
  PVR_D3D_LVU_655,
  PVR_D3D_XLVU_8888,
  PVR_D3D_QWVU_8888,

  /* 10 bit integer - 2 bit alpha */
  PVR_D3D_ABGR_2101010,
  PVR_D3D_ARGB_2101010,
  PVR_D3D_AWVU_2101010,

  /* 16 bit integers */
  PVR_D3D_GR_1616,
  PVR_D3D_VU_1616,
  PVR_D3D_ABGR_16161616,

  /* Float Formats */
  PVR_D3D_R16F,
  PVR_D3D_GR_1616F,
  PVR_D3D_ABGR_16161616F,

  /* 32 bits per channel */
  PVR_D3D_R32F,
  PVR_D3D_GR_3232F,
  PVR_D3D_ABGR_32323232F,

  /* Ericsson */
  PVR_ETC_RGB_4BPP,
  PVR_ETC_RGBA_EXPLICIT,                        /* unimplemented */
  PVR_ETC_RGBA_INTERPOLATED,                    /* unimplemented */

  PVR_D3D_A8 = 0x40,
  PVR_D3D_V8U8,
  PVR_D3D_L16,

  PVR_D3D_L8,
  PVR_D3D_AL_88,

  /* YUV Colourspace */
  PVR_D3D_UYVY,
  PVR_D3D_YUY2,

  /* DX10 */
  PVR_DX10_R32G32B32A32_FLOAT= 0x50,
  PVR_DX10_R32G32B32A32_UINT,
  PVR_DX10_R32G32B32A32_SINT,

  PVR_DX10_R32G32B32_FLOAT,
  PVR_DX10_R32G32B32_UINT,
  PVR_DX10_R32G32B32_SINT,

  PVR_DX10_R16G16B16A16_FLOAT,
  PVR_DX10_R16G16B16A16_UNORM,
  PVR_DX10_R16G16B16A16_UINT,
  PVR_DX10_R16G16B16A16_SNORM,
  PVR_DX10_R16G16B16A16_SINT,

  PVR_DX10_R32G32_FLOAT,
  PVR_DX10_R32G32_UINT,
  PVR_DX10_R32G32_SINT,

  PVR_DX10_R10G10B10A2_UNORM,
  PVR_DX10_R10G10B10A2_UINT,

  PVR_DX10_R11G11B10_FLOAT,			/* unimplemented */

  PVR_DX10_R8G8B8A8_UNORM,
  PVR_DX10_R8G8B8A8_UNORM_SRGB,
  PVR_DX10_R8G8B8A8_UINT,
  PVR_DX10_R8G8B8A8_SNORM,
  PVR_DX10_R8G8B8A8_SINT,

  PVR_DX10_R16G16_FLOAT,
  PVR_DX10_R16G16_UNORM,
  PVR_DX10_R16G16_UINT,
  PVR_DX10_R16G16_SNORM,
  PVR_DX10_R16G16_SINT,

  PVR_DX10_R32_FLOAT,
  PVR_DX10_R32_UINT,
  PVR_DX10_R32_SINT,

  PVR_DX10_R8G8_UNORM,
  PVR_DX10_R8G8_UINT,
  PVR_DX10_R8G8_SNORM,
  PVR_DX10_R8G8_SINT,

  PVR_DX10_R16_FLOAT,
  PVR_DX10_R16_UNORM,
  PVR_DX10_R16_UINT,
  PVR_DX10_R16_SNORM,
  PVR_DX10_R16_SINT,

  PVR_DX10_R8_UNORM,
  PVR_DX10_R8_UINT,
  PVR_DX10_R8_SNORM,
  PVR_DX10_R8_SINT,

  PVR_DX10_A8_UNORM,
  PVR_DX10_R1_UNORM,
  PVR_DX10_R9G9B9E5_SHAREDEXP,                  /* unimplemented */
  PVR_DX10_R8G8_B8G8_UNORM,                     /* unimplemented */
  PVR_DX10_G8R8_G8B8_UNORM,                     /* unimplemented */

  PVR_DX10_BC1_UNORM,
  PVR_DX10_BC1_UNORM_SRGB,

  PVR_DX10_BC2_UNORM,
  PVR_DX10_BC2_UNORM_SRGB,

  PVR_DX10_BC3_UNORM,
  PVR_DX10_BC3_UNORM_SRGB,

  PVR_DX10_BC4_UNORM,                           /* unimplemented */
  PVR_DX10_BC4_SNORM,                           /* unimplemented */

  PVR_DX10_BC5_UNORM,                           /* unimplemented */
  PVR_DX10_BC5_SNORM,                           /* unimplemented */

  /* OpenVG */

  /* RGB{A,X} channel ordering */
  PVR_VG_sRGBX_8888  = 0x90,
  PVR_VG_sRGBA_8888,
  PVR_VG_sRGBA_8888_PRE,
  PVR_VG_sRGB_565,
  PVR_VG_sRGBA_5551,
  PVR_VG_sRGBA_4444,
  PVR_VG_sL_8,
  PVR_VG_lRGBX_8888,
  PVR_VG_lRGBA_8888,
  PVR_VG_lRGBA_8888_PRE,
  PVR_VG_lL_8,
  PVR_VG_A_8,
  PVR_VG_BW_1,

  /* {A,X}RGB channel ordering */
  PVR_VG_sXRGB_8888,
  PVR_VG_sARGB_8888,
  PVR_VG_sARGB_8888_PRE,
  PVR_VG_sARGB_1555,
  PVR_VG_sARGB_4444,
  PVR_VG_lXRGB_8888,
  PVR_VG_lARGB_8888,
  PVR_VG_lARGB_8888_PRE,

  /* BGR{A,X} channel ordering */
  PVR_VG_sBGRX_8888,
  PVR_VG_sBGRA_8888,
  PVR_VG_sBGRA_8888_PRE,
  PVR_VG_sBGR_565,
  PVR_VG_sBGRA_5551,
  PVR_VG_sBGRA_4444,
  PVR_VG_lBGRX_8888,
  PVR_VG_lBGRA_8888,
  PVR_VG_lBGRA_8888_PRE,

  /* {A,X}BGR channel ordering */
  PVR_VG_sXBGR_8888,
  PVR_VG_sABGR_8888,
  PVR_VG_sABGR_8888_PRE,
  PVR_VG_sABGR_1555,
  PVR_VG_sABGR_4444,
  PVR_VG_lXBGR_8888,
  PVR_VG_lABGR_8888,
  PVR_VG_lABGR_8888_PRE,

  PVR_N_PIXEL_TYPES,

} PVRPixelType;

#define PVR_FLAG_MIPMAP           (1<<8)      /* has mip map levels */
#define PVR_FLAG_TWIDDLE          (1<<9)      /* is twiddled */
#define PVR_FLAG_BUMPMAP          (1<<10)     /* has normals encoded for a bump
                                                 map */
#define PVR_FLAG_TILING           (1<<11)     /* is bordered for tiled pvr */
#define PVR_FLAG_CUBEMAP          (1<<12)     /* is a cubemap/skybox */
#define PVR_FLAG_FALSEMIPCOL      (1<<13)     /* are there false coloured MIP
                                                 levels */
#define PVR_FLAG_VOLUME           (1<<14)     /* is this a volume texture */
#define PVR_FLAG_ALPHA            (1<<15)     /* v2.1 is there transparency
                                                 info in the texture */
#define PVR_FLAG_VERTICAL_FLIP    (1<<16)     /* v2.1 is the texture vertically
                                                 flipped */

#define PVR_FLAG_PIXELTYPE        0xff        /* pixel type is always in the
                                                 last 16bits of the flags */
#define PVR_FLAG_IDENTIFIER       0x21525650  /* the pvr identifier is the
                                                 characters 'P','V','R','!' */

#define PVR_FLAG_V1_HEADER_SIZE   44          /* old header size was 44 for
                                                 identification purposes */

#define PVR_PVRTC2_MIN_TEXWIDTH   16
#define PVR_PVRTC2_MIN_TEXHEIGHT  8
#define PVR_PVRTC4_MIN_TEXWIDTH   8
#define PVR_PVRTC4_MIN_TEXHEIGHT  8
#define PVR_ETC_MIN_TEXWIDTH      4
#define PVR_ETC_MIN_TEXHEIGHT     4
#define PVR_DXT_MIN_TEXWIDTH      4
#define PVR_DXT_MIN_TEXHEIGHT     4

#endif /* __GDK_PIXBUF_PVR_H__ */

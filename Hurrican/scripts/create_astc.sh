#!bin/sh

export DATA="data/textures"
export TEX="tc"
export TYPE=${TEX}"/astc"
export PNG32=${TEX}"/png32"

cd ../${DATA}
rm -rf ${TYPE}
rm -rf ${PNG32}
mkdir ${TEX}
mkdir ${TYPE}
mkdir ${PNG32}

for f in *.png
do
    echo "Processing $f file..."
    
    convert ${f} -alpha on PNG32:${PNG32}/${f}
    
    ${COMPRESSONATOR}/CompressonatorCLI -nomipmap -fd ASTC -BlockRate 4x4 ${PNG32}/${f} ${TYPE}/${f}.astc
done

# ------------------------------------------------
# CompressonatorCLI V3.2.535 Copyright AMD 2019
# ------------------------------------------------
# 
# Usage: CompressonatorCLI.exe [options] SourceFile DestFile
# 
# MipMap options:
# 
# -nomipmap                 Turns off Mipmap generation
# -mipsize    <size>        The size in pixels used to determine
#                           how many mip levels to generate
# -miplevels  <Level>       Sets Mips Level for output,
#                           (mipSize overides this option): default is 1
# Compression options:
# 
# -fs <format>    Optionally specifies the source texture format to use
# -fd <format>    Specifies the destination texture format to use
# -decomp <filename>   If the destination  file is compressed optionally
#                      decompress it
#                      to the specified file. Note the destination  must
#                      be compatible 
#                      with the sources format,decompress formats are typically
#                      set to ARGB_8888 or ARGB_32F
# -EncodeWith          Compression with CPU or HPC
# -draco               Enable draco compression. (only support glTF files)
# -doswizzle           Swizzle the source images Red and Blue channels
# 
# The following is a list of channel formats
# ARGB_16        ARGB format with 16-bit fixed channels
# ARGB_16F       ARGB format with 16-bit floating-point channels
# ARGB_32F       ARGB format with 32-bit floating-point channels
# ARGB_2101010   ARGB format with 10-bit fixed channels for color
#                and a 2-bit fixed channel for alpha
# ARGB_8888      ARGB format with 8-bit fixed channels
# R_8            Single component format with 8-bit fixed channels
# R_16           Single component format with 16-bit fixed channels
# R_16F          Two component format with 32-bit floating-point channels
# R_32F          Single component with 32-bit floating-point channels
# RG_8           Two component format with 8-bit fixed channels
# RG_16          Two component format with 16-bit fixed channels
# RG_16F         Two component format with 16-bit floating-point channels
# RG_32F         Two component format with 32-bit floating-point channels
# RGB_888        RGB format with 8-bit fixed channels
# 
# The following is a list of compression formats
# ASTC           Adaptive Scalable Texture Compression
# ATC_RGB                 Compressed RGB format
# ATC_RGBA_Explicit       ARGB format with explicit alpha
# ATC_RGBA_Interpolated   ARGB format with interpolated alpha
# ATI1N          Single component compression format using the same
#                technique as DXT5 alpha. Four bits per pixel
# ATI2N          Two component compression format using the same
#                technique as DXT5 alpha. Designed for compression object
#                space normal maps. Eight bits per pixel
# ATI2N_XY       Two component compression format using the same technique
#                as DXT5 alpha. The same as ATI2N but with the channels swizzled.
#                Eight bits per pixel
# ATI2N_DXT5     An ATI2N like format using DXT5. Intended for use on GPUs that
#                do not natively support ATI2N. Eight bits per pixel
# BC1            Four component opaque (or 1-bit alpha) compressed texture
#                format. Four bit per pixel
# BC2            Four component compressed texture format with explicit
#                alpha.  Eight bits per pixel
# BC3            Four component compressed texture format with interpolated
#                alpha.  Eight bits per pixel
# BC4            Single component compressed texture format for Microsoft
# BC5            Two component compressed texture format for Microsoft
# BC6H           High-Dynamic Range  compression format
# BC7            High-quality compression of RGB and RGBA data
# 
# DXT1           An opaque (or 1-bit alpha) DXTC compressed texture format.
#                Four bits per pixel
# DXT3           DXTC compressed texture format with explicit alpha.
#                Eight bits per pixel
# DXT5           DXTC compressed texture format with interpolated alpha.
#                Eight bits per pixel
# DXT5_xGBR      DXT5 with the red component swizzled into the alpha channel.
#                Eight bits per pixel
# DXT5_RxBG      Swizzled DXT5 format with the green component swizzled
#                into the alpha channel. Eight bits per pixel
# DXT5_RBxG      Swizzled DXT5 format with the green component swizzled
#                into the alpha channel & the blue component swizzled into
#                the green channel. Eight bits per pixel
# DXT5_xRBG      Swizzled DXT5 format with the green component swizzled
#                into the alpha channel & the red component swizzled into the
#                green channel. Eight bits per pixel
# DXT5_RGxB      Swizzled DXT5 format with the blue component swizzled
#                into the alpha channel. Eight bits per pixel
# DXT5_xGxR      Two-component swizzled DXT5 format with the red component
#                swizzled into the alpha channel & the green component in the
#                green channel. Eight bits per pixel
# ETC_RGB        Ericsson Texture Compression - Compressed RGB format.
# ETC2_RGB       Ericsson Texture Compression - Compressed RGB format.
# ETC2_RGBA      Ericsson Texture Compression - Compressed RGB with 8 bit Alpha.
# ETC2_RGBA1     Ericsson Texture Compression - Compressed RGB with 1 bit Alpha.
# 
# <codec options>: Reference  documentation for range of values
# 
# -UseChannelWeighting <value> Use channel weightings
# -WeightR <value>             The weighting of the Red or X Channel
# -WeightG <value>             The weighting of the Green or Y Channel
# -WeightB <value>             The weighting of the Blue or Z Channel
# -AlphaThreshold <value>      The alpha threshold to use when compressing
#                              to DXT1 & BC1 with DXT1UseAlpha
#                              Texels with an alpha value less than the threshold
#                              are treated as transparent
#                              value is in the range of 0 to 255, default is 128
# -BlockRate <value>           ASTC 2D only - sets block size or bit rate
#                              value can be a bit per pixel rate from 0.0 to 9.9
#                              or can be a combination of x and y axes with paired
#                              values of 4,5,6,8,10 or 12 from 4x4 to 12x12
# -DXT1UseAlpha <value>        Encode single-bit alpha data.
#                              Only valid when compressing to DXT1 & BC1
# -CompressionSpeed <value>    The trade-off between compression speed & quality
#                              This setting is not used in BC6H and BC7
# -NumThreads <value>          Number of threads to initialize for ASTC,BC6H,BC7
#                              encoding (Max up to 128). Default set to 8
# -Quality <value>             Sets quality of encoding for BC7
# -Performance <value>         Sets performance of encoding for BC7
# -ColourRestrict <value>      This setting is a quality tuning setting for BC7
#                              which may be necessary for convenience in some
#                              applications
# -AlphaRestrict <value>       This setting is a quality tuning setting for BC7
#                              which may be necessary for some textures
# -ModeMask <value>            Mode to set BC7 to encode blocks using any of 8
#                              different block modes in order to obtain the
#                              highest quality
# -Analysis <image1> <image2>  Generate analysis metric like SSIM, PSNR values 
#                              between 2 images with same size. Analysis_Result.xml file will be generated.
# 
# 
# -diff_image <image1> <image2> Generate difference between 2 images with same size 
#                               A .bmp file will be generated. Please use compressonator GUI to increase the contrast to view the diff pixels.
# -log                         Logs process information to a process_results.txt file containing
#                              file info, performance data, SSIM, PSNR and MSE. 
# -logfile <filename>          Logs process information to a user defined text file
# 
# 
# -imageprops <image>           Print image properties of image files specifies. 
# 
# 
# Output options:
# 
# -silent                      Disable print messages
# -performance                 Shows various performance stats
# -noprogress                  Disables showing of compression progress messages
# 
# 
# Example compression:
# 
# CompressonatorCLI.exe -fd ASTC image.bmp result.astc 
# CompressonatorCLI.exe -fd ASTC -BlockRate 0.8 image.bmp result.astc
# CompressonatorCLI.exe -fd ASTC -BlockRate 12x12 image.bmp result.astc
# CompressonatorCLI.exe -fd BC7  image.bmp result.dds 
# CompressonatorCLI.exe -fd BC7  image.bmp result.bmp
# CompressonatorCLI.exe -fd BC7  -NumTheads 16 image.bmp result.dds
# CompressonatorCLI.exe -fd BC7  -ff PNG -fx KTX ./source_dir/ ./dist_dir/
# CompressonatorCLI.exe -fd BC6H image.exr result.exr
# 
# Example decompression from compressed image using CPU:
# 
# CompressonatorCLI.exe  result.dds image.bmp
# 
# Example decompression from compressed image using GPU:
# 
# CompressonatorCLI.exe  -UseGPUDecompress result.dds image.bmp
# 
# Example compression with decompressed result (Useful for qualitative analysis):
# 
# Example draco compression usage (support glTF and OBJ file only):
# 
# CompressonatorCLI.exe -draco source.gltf dest.gltf
# CompressonatorCLI.exe -draco source.obj dest.drc
# Note: only .obj file produces compressed .drc file. glTF does not produce .drc compressed file.
# Note: You can specify .obj as compressed file format as well, but a new .drc file will be created for this case.
# 
# Example draco decompression usage (support glTF and drc file only):
# 
# CompressonatorCLI.exe source.gltf dest.gltf
# CompressonatorCLI.exe source.drc dest.obj

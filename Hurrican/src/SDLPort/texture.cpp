/**
 *
 *  Copyright (C) 2011-2015 Scott R. Smith
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *
 */

#include "Main.h"
#include "DX8Texture.h"
#include "DX8Graphics.h"
#include "texture.h"
#include "Texts.h"          //For ReplaceAll() function

using namespace std;

#if defined(USE_ETC1)
#define ETC1_HEADER_SIZE 16
#endif

#if defined(USE_ETC2)
#define ETC2_HEADER_SIZE 64
#endif

#if defined(USE_ASTC)
#define ASTC_HEADER_SIZE 16
#endif

#if defined(USE_DXT3)
#define DXT3_HEADER_SIZE 128
#endif

#if defined(USE_PVRTC)
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG			0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG			0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG			0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG			0x8C03

#define PVRTC_HEADER_SIZE 52

enum
{
    PVRTC_RGB_2BPP=0,
    PVRTC_RGBA_2BPP,
    PVRTC_RGB_4BPP,
    PVRTC_RGBA_4BPP
};
#endif

//DKS - Textures are now managed in DX8Sprite.cpp in new TexturesystemClass.
//      This function now returns a TextureHandle object, and if it fails to load the file,
//      all of the returned TextureHandle's data members will be set to 0.
//      In th.npot_scalex and th.npot_scaley, it sets a correction
//      factor to apply to each dimension to compensate for any increases in size
//      from power-of-two expansion (each will remain 1.0 if none occurred).
bool SDL_LoadTexture( const string &path, const string &filename,
                      void *buf, unsigned int buf_size, TextureHandle &th )
{
    image_t image;
    bool success = false;
    bool load_from_memory = buf_size > 0;
    string fullpath;
    string filename_sans_ext(filename);
    ReplaceAll(filename_sans_ext, ".png", "");

    if (load_from_memory && !buf) {
        Protokoll.WriteText( true, "Error in SDL_LoadTexture() loading texture from memory!\n" );
        return false;
    } else if (!load_from_memory && filename.empty()) {
        Protokoll.WriteText( true, "Error in SDL_LoadTexture(): empty filename parameter\n" );
        return false;
    }

#if defined(USE_ETC1)
    if (DirectGraphics.SupportedETC1 == true)
    {
        image_t alpha_image;

        // First, load the RGB texture:
        fullpath = path + "/tc/etc1/" + filename_sans_ext + ".pkm";
        success = loadImageETC1(image, fullpath);

        if (success) {
            // Then, load the alpha texture:
            fullpath = path + "/tc/etc1/" + filename_sans_ext + "_alpha.pkm";
            success = loadImageETC1(alpha_image, fullpath);
        }

       if (success) {
            success = load_texture( image, th.tex ) &&
                      load_texture( alpha_image, th.alphatex);
       }

       delete [] image.data;
       delete [] alpha_image.data;

       if (success)
           goto loaded;
       else
           th.tex = th.alphatex = 0;
    }
#endif

#if defined(USE_ETC2)
    if (DirectGraphics.SupportedETC2 == true)
    {
        fullpath = path + "/tc/etc2/" + filename + ".ktx";

#if defined(_DEBUG)
        Protokoll.WriteText( false, "Using ETC2 looking for %s\n", fullpath.c_str() );
#endif

        success = loadImageETC2( image, fullpath ) &&
                  load_texture ( image, th.tex );

        delete [] image.data;
        if (success)
            goto loaded;
        else
            th.tex = 0;
    }
#endif

#if defined(USE_ASTC)
    if (DirectGraphics.SupportedASTC == true)
    {
        fullpath = path + "/tc/astc/" + filename + ".astc";

#if defined(_DEBUG)
        Protokoll.WriteText( false, "Using ASTC looking for %s\n", fullpath.c_str() );
#endif

        success = loadImageASTC( image, fullpath ) &&
                  load_texture ( image, th.tex );

        delete [] image.data;
        if (success)
            goto loaded;
        else
            th.tex = 0;
    }
#endif

#if defined(USE_DXT3)
    if (DirectGraphics.SupportedDXT3 == true)
    {
        fullpath = path + "/tc/dxt3/" + filename + ".dds";

#if defined(_DEBUG)
        Protokoll.WriteText( false, "Using DXT3 looking for %s\n", fullpath.c_str() );
#endif

        success = loadImageDXT3( image, fullpath ) &&
                  load_texture ( image, th.tex );

        delete [] image.data;
        if (success)
            goto loaded;
        else
            th.tex = 0;
    }
#endif

#if defined(USE_PVRTC)
    if (DirectGraphics.SupportedPVRTC == true)
    {
        fullpath = path + "/tc/pvr/" + filename + ".pvr";

#if defined(_DEBUG)
        Protokoll.WriteText( false, "Using PVR looking for %s\n", fullpath.c_str() );
#endif

        success = loadImagePVRTC( image, fullpath ) &&
                  load_texture ( image, th.tex );

        delete [] image.data;
        if (success)
            goto loaded;
        else
            th.tex = 0;
    }
#endif

    fullpath = path + "/" + filename;
    success = loadImageSDL( image, fullpath, buf, buf_size ) &&
              load_texture( image, th.tex );
    delete [] image.data;

    if (success)
        goto loaded;
    else
        th.tex = 0;

loaded:
    if (success) {
        th.instances = 1;
        th.npot_scalex = image.npot_scalex;
        th.npot_scaley = image.npot_scaley;
    } else {
        Protokoll.WriteText( true, "Error loading texture %s in SDL_LoadTexture()\n", filename.c_str() );
    }

    return success;
}

void SDL_UnloadTexture( TextureHandle &th )
{
    glDeleteTextures( 1, &th.tex );
    th.tex = 0;
#if defined(USE_ETC1)
    glDeleteTextures( 1, &th.alphatex );
    th.alphatex = 0;
#endif
    th.instances = 0;
}

bool load_texture( image_t& image, GLuint &new_texture )
{
    GLuint texture;

    if (image.data != NULL)
    {
        // Have OpenGL generate a texture object handle for us
        glGenTextures( 1, &texture );

        // Bind the texture object
        glBindTexture( GL_TEXTURE_2D, texture );

        // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        //DKS - this fixes the lightning-beam drawing in low,med-detail modes..
        //      and from looking at its code in Player.cpp, I am convinced this is
        //      correct edge setting for textures for this game. My play-testing
        //      shows no unwanted side effects from the change to GL_REPEAT.
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        if (image.compressed == true)
        {
            glCompressedTexImage2D( GL_TEXTURE_2D, 0, image.format,
                                    image.w, image.h, 0, image.size, image.data+image.offset );
        }
        else
        {
            glTexImage2D( GL_TEXTURE_2D, 0, image.format, image.w, image.h, 0,
                          image.format, image.type, image.data );
        }

#if defined(_DEBUG)
        int error = glGetError();
        if (error != 0)
        {
            Protokoll.WriteText( false, "GL load_texture Error %X\n", error );
            Protokoll.WriteText( false, "Format %X W %d H %d S %d Data %X + %d\n", image.format, image.w, image.h, image.size, image.data, image.offset );
            return false;
        }
#endif

    }
    else
    {
        Protokoll.WriteText( false, "ERROR Image data reference is NULL\n" );
        return false;
    }

    new_texture = texture;
    return true;
}

#if defined(USE_ETC1)
bool loadImageETC1( image_t& image, const std::string &fullpath )
{
    /*
        00-03 4 bytes header "PKM "
        04-05 2 bytes version "10"
        06-07 2 bytes data type (always zero)
        08-09 2 bytes extended width
        10-11 2 bytes extended height
        12-13 2 bytes original width
        14-15 2 bytes original height
        rest is data

        compressed size = (extended width / 4) * (extended height / 4) * 8
    */

    uint32_t etc1_filesize;

    if (fullpath.empty() || !FileExists(fullpath.c_str()))
        return false;

    image.data = LoadFileToMemory( fullpath, etc1_filesize );

    if (image.data != NULL)
    {
        if ((image.data[0] == 'P') &&
            (image.data[1] == 'K') &&
            (image.data[2] == 'M') &&
            (image.data[3] == ' ') &&
            (image.data[4] == '1') &&
            (image.data[5] == '0')
           )
        {
            image.format        = GL_ETC1_RGB8_OES;
            image.h             = (image.data[14]<<8)+image.data[15];
            image.w             = (image.data[12]<<8)+image.data[13];
            image.size          = (((image.data[8]<<8)+image.data[9]) / 4) * (((image.data[10]<<8)+image.data[11]) / 4) * 8;
            image.offset        = ETC1_HEADER_SIZE;
            image.type          = 0; /* dont care */
            image.compressed    = true;

            Protokoll.WriteText( false, "Using ETC1 (%X) ... ", image.format );

#if defined(_DEBUG)
            Protokoll.WriteText( false, "Header %c%c%c%c\nVersion %X\nType %d\nExt Width %d\nExt Height %d\nWidth %d\nHeight %d\n",
                                 image.data[0], image.data[1], image.data[2], image.data[3],
                                 (image.data[4]<<8)+image.data[5], (image.data[6]<<8)+image.data[7], (image.data[8]<<8)+image.data[9],
                                 (image.data[10]<<8)+image.data[11], (image.data[12]<<8)+image.data[13], (image.data[14]<<8)+image.data[15] );
#endif
            return true;
        }
        else
        {
            Protokoll.WriteText( false, "ERROR ETC1 Unknown file type %c%c%c%c\n",  image.data[0], image.data[1], image.data[2], image.data[3] );
            delete [] image.data;
            image.data = NULL;
        }
    }

    return false;
}
#endif

#if defined(USE_ETC2)
bool loadImageETC2( image_t& image, const std::string &fullpath )
{
    /*
       00-11     Byte[12] identifier
       12-15     UInt32 endianness
       16-19     UInt32 glType
       20-23     UInt32 glTypeSize
       24-27     UInt32 glFormat
       28-31     Uint32 glInternalFormat
       32-35     Uint32 glBaseInternalFormat
       36-39     UInt32 pixelWidth
       40-43     UInt32 pixelHeight
       44-47     UInt32 pixelDepth
       48-51     UInt32 numberOfArrayElements
       52-55     UInt32 numberOfFaces
       56-59     UInt32 numberOfMipmapLevels
       60-63     UInt32 bytesOfKeyValueData

                for each keyValuePair that fits in bytesOfKeyValueData
                    UInt32   keyAndValueByteSize
                    Byte     keyAndValue[keyAndValueByteSize]
                    Byte     valuePadding[3 - ((keyAndValueByteSize + 3) % 4)]
                end

                UInt32 imageSize;
    */

    uint32_t etc2_filesize;
    uint32_t meta_size;
    uint32_t meta_offset;

    if (fullpath.empty() || !FileExists(fullpath.c_str()))
        return false;

    image.data = LoadFileToMemory( fullpath, etc2_filesize );

    if (image.data != NULL)
    {
        if ((image.data[0] == 0xAB) &&
            (image.data[1] == 'K') &&
            (image.data[2] == 'T') &&
            (image.data[3] == 'X') &&
            (image.data[4] == ' ') &&
            (image.data[5] == '1') &&
            (image.data[6] == '1') &&
            (image.data[7] == 0xBB)
           )
        {
            image.format        = (image.data[31]<<24)+(image.data[30]<<16)+(image.data[29]<<8)+image.data[28];
            image.h             = (image.data[43]<<24)+(image.data[42]<<16)+(image.data[41]<<8)+image.data[40];
            image.w             = (image.data[39]<<24)+(image.data[38]<<16)+(image.data[37]<<8)+image.data[36];
            meta_size           = (image.data[63]<<24)+(image.data[62]<<16)+(image.data[61]<<8)+image.data[60];
            meta_offset         = ETC2_HEADER_SIZE + meta_size;
            image.size          = (image.data[meta_offset+3]<<24)+(image.data[meta_offset+2]<<16)+(image.data[meta_offset+1]<<8)+image.data[meta_offset];
            image.offset        = meta_offset + 4;
            image.type          = 0; /* dont care */
            image.compressed    = true;

            Protokoll.WriteText( false, "Using ETC2 (%X) ... ", image.format );

#if defined(_DEBUG)
            Protokoll.WriteText( false, "Header %c%c%c%c\nFormat %X\nHeight %d\nWidth %d\nSize %d\nMetaSize %d\nMetaOffset %d\n",
                                 image.data[0], image.data[1], image.data[2], image.data[3],
                                 image.format, image.h, image.w, image.size, meta_size, meta_offset );
#endif
            return true;
        }
        else
        {
            Protokoll.WriteText( false, "ERROR ETC2 Unknown file type %c%c%c%c\n",  image.data[0], image.data[1], image.data[2], image.data[3] );
            delete [] image.data;
            image.data = NULL;
        }
    }

    return false;
}
#endif

#if defined(USE_ASTC)
bool loadImageASTC( image_t& image, const std::string &fullpath )
{
    /*
        ASTC header declaration.
        typedef struct
        {
    0-3      unsigned char magic[4]
    4        unsigned char blockdim_x;
    5        unsigned char blockdim_y;
    6        unsigned char blockdim_z;
    7-9      unsigned char xsize[3];
    10-12    unsigned char ysize[3];
    13-15    unsigned char zsize[3];
        } astc_header;
    */

    uint32_t astc_filesize;
    uint32_t xblocks, yblocks;

    if (fullpath.empty() || !FileExists(fullpath.c_str()))
        return false;

    image.data = LoadFileToMemory( fullpath, astc_filesize );

    if (image.data != NULL)
    {
        if ((image.data[0] == 0x13) &&
            (image.data[1] == 0xAB) &&
            (image.data[2] == 0xA1) &&
            (image.data[3] == 0x5C)
           )
        {
            image.format        = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
            image.h             = (image.data[12]<<16)+(image.data[11]<<8)+image.data[10];
            image.w             = (image.data[9]<<16)+(image.data[8]<<8)+image.data[7];

            /* Compute number of blocks in each direction. */
            xblocks = (image.w + image.data[4] - 1) / image.data[4];
            yblocks = (image.h + image.data[5] - 1) / image.data[5];
            /* Each block is encoded on 16 bytes, so calculate total compressed image data size. */
            image.size = xblocks * yblocks << 4;

            image.offset        = ASTC_HEADER_SIZE;
            image.type          = 0; /* dont care */
            image.compressed    = true;

            Protokoll.WriteText( false, "Using ASTC (%X) ... ", image.format );

#if defined(_DEBUG)
            Protokoll.WriteText( false, "ASTC Header %X%X%X%X\nHeight %X\nWidth %d\nSize %d\n",
                                 image.data[0], image.data[1], image.data[2], image.data[3],
                                 image.h, image.w, image.size );
#endif
            return true;
        }
        else
        {
            Protokoll.WriteText( false, "ERROR ASTC Unknown file type %X%X%X%X\n",  image.data[0], image.data[1], image.data[2], image.data[3] );
            delete [] image.data;
            image.data = NULL;
        }
    }

    return false;
}
#endif

#if defined(USE_DXT3)
bool loadImageDXT3( image_t& image, const std::string &fullpath )
{
    /*
      00-03    Magic bytes
             typedef struct {
      04-07    DWORD           dwSize;
      08-11    DWORD           dwFlags;
      12-15    DWORD           dwHeight;
      16-19    DWORD           dwWidth;
      20-23    DWORD           dwPitchOrLinearSize;
      24-27    DWORD           dwDepth;
      28-31    DWORD           dwMipMapCount;
      32-35    DWORD           dwReserved1[11];
      36-67    DDS_PIXELFORMAT ddspf;
      68-71    DWORD           dwCaps;
      72-75    DWORD           dwCaps2;
      76-79    DWORD           dwCaps3;
      80-83    DWORD           dwCaps4;
      84-87    DWORD           dwReserved2;
             } DDS_HEADER;
    */

    uint32_t filesize;

    if (fullpath.empty() || !FileExists(fullpath.c_str()))
        return false;

    image.data = LoadFileToMemory( fullpath, filesize );

    if (image.data != NULL)
    {
        if ((image.data[0] == 'D') &&
            (image.data[1] == 'D') &&
            (image.data[2] == 'S') &&
            (image.data[3] == ' ')
           )
        {
            image.format        = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            image.h             = (image.data[15]<<24)+(image.data[14]<<16)+(image.data[13]<<8)+image.data[12];
            image.w             = (image.data[19]<<24)+(image.data[18]<<16)+(image.data[17]<<8)+image.data[16];
            image.size          = MAX(1, ( (image.w + 3) / 4 ) ) * MAX(1, ( (image.h + 3) / 4 ) ) * 16;

            image.offset        = DXT3_HEADER_SIZE;
            image.type          = 0; /* dont care */
            image.compressed    = true;

            Protokoll.WriteText( false, "Using DXT3 (%X) ... ", image.format );

#if defined(_DEBUG)
            Protokoll.WriteText( false, "DXT3 Header %c%c%c%c\nHeight %X\nWidth %d\nSize %d\n",
                                 image.data[0], image.data[1], image.data[2], image.data[3],
                                 image.h, image.w, image.size );
#endif
            return true;
        }
        else
        {
            Protokoll.WriteText( false, "ERROR DXT3 Unknown file type %c%c%c%c\n",  image.data[0], image.data[1], image.data[2], image.data[3] );
            delete [] image.data;
            image.data = NULL;
        }
    }

    return false;
}
#endif

#if defined(USE_PVRTC)
bool loadImagePVRTC( image_t& image, const string &fullpath )
{
    uint32_t* pvrtc_buffer32 = NULL;
    uint32_t pvrtc_filesize, pvrtc_depth, pvrtc_bitperpixel;

    if (fullpath.empty() || !FileExists(fullpath.c_str()))
        return false;

    image.data = LoadFileToMemory( fullpath, pvrtc_filesize );

    if (image.data != NULL)
    {
        pvrtc_buffer32  = (uint32_t*)image.data;

        switch ( pvrtc_buffer32[2] )
        {
        case PVRTC_RGB_2BPP:
            image.format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
            pvrtc_bitperpixel = 2;
            break;
        case PVRTC_RGBA_2BPP:
            image.format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
            pvrtc_bitperpixel = 2;
            break;
        case PVRTC_RGB_4BPP:
            image.format = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
            pvrtc_bitperpixel = 4;
            break;
        case PVRTC_RGBA_4BPP:
            image.format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
            pvrtc_bitperpixel = 4;
            break;
        default:
            Protokoll.WriteText( false, "ERROR Unknown PVRTC format %X\n",  pvrtc_buffer32[2] );
            delete [] image.data;
            image.data = NULL;
            return false;
        }

        image.h             = pvrtc_buffer32[6];
        image.w             = pvrtc_buffer32[7];
        pvrtc_depth         = pvrtc_buffer32[8];
        image.size          = (image.w * image.h * pvrtc_depth * pvrtc_bitperpixel) / 8;
        image.offset        = PVRTC_HEADER_SIZE;
        image.type          = 0; /* dont care */
        image.compressed    = true;

        Protokoll.WriteText( false, "Using PVRTC (%X) ... ", image.format );

#if defined(_DEBUG)
        Protokoll.WriteText( false, "Version %X\nFlags %d\nPFormatA %d\nPFormatB %d\nColorS %d\nChanType %d\nHeight %d\nWidth %d\nDepth %d\nNumSurf %d\nNumFaces %d\nMipmap %d\nMeta %d\n", pvrtc_buffer32[0], pvrtc_buffer32[1], pvrtc_buffer32[2], pvrtc_buffer32[3],
                             pvrtc_buffer32[4], pvrtc_buffer32[5], pvrtc_buffer32[6], pvrtc_buffer32[7], pvrtc_buffer32[8],
                             pvrtc_buffer32[9], pvrtc_buffer32[10], pvrtc_buffer32[11], pvrtc_buffer32[12] );
#endif

        return true;
    }

    return false;
}
#endif

bool loadImageSDL( image_t& image, const std::string &fullpath, void *buf, unsigned int buf_size )
{
    //DKS - No longer needed, as we no longer use sprites that are colorkeyed
#if 0
#if SDL_VERSION_ATLEAST(2,0,0)
    uint32_t flags = SDL_TRUE;
#else
    uint32_t flags = SDL_SRCCOLORKEY|SDL_RLEACCEL;
#endif
#endif

    uint8_t factor;
    SDL_Rect rawDimensions;
    SDL_Surface* rawSurf = NULL;	// This surface will tell us the details of the image
    SDL_Surface* finSurf = NULL;

    // Init
    image.data          = NULL;
    image.size          = 0; /* dont care */
    image.compressed    = false;
    image.format        = GL_RGBA;
    image.npot_scalex   = 1.0;
    image.npot_scaley   = 1.0;

    if (buf_size == 0)  // Load from file
    {
        if (fullpath.empty() || !FileExists(fullpath.c_str())) {
            Protokoll.WriteText( true, "Error in loadImageSDL loading %s\n", fullpath.c_str() );
            return false;
        }

        rawSurf = IMG_Load(fullpath.c_str());
    } else              // Load from memory
    {
        SDL_RWops* sdl_rw = SDL_RWFromConstMem( (const void*)buf, buf_size );

        if (sdl_rw != NULL)
        {
            rawSurf = IMG_Load_RW( sdl_rw, 1 );
        } else
        {
            Protokoll.WriteText( true, "ERROR Texture: Failed to load texture: %s\n", SDL_GetError() );
            return false;
        }
    }

    if (rawSurf != NULL)
    {
        //  Store dimensions of original RAW surface
        rawDimensions.x = rawSurf->w;
        rawDimensions.y = rawSurf->h;

        //DKS - Allowing NPOT textures is now a command line switch:
        //      The safest thing seems to me to leave it off unless specifically requested,
        //      since some GPUs/drivers do not play nicely with it and old GL/GLES doesn't even pretend to.
        if (!CommandLineParams.AllowNPotTextureSizes) {
            //  Check if surface is PoT
            if (!isPowerOfTwo(rawSurf->w)) {
                rawDimensions.x = nextPowerOfTwo(rawSurf->w);
                image.npot_scalex = (double)rawSurf->w / (double)rawDimensions.x;
            }

            if (!isPowerOfTwo(rawSurf->h)) {
                rawDimensions.y = nextPowerOfTwo(rawSurf->h);
                image.npot_scaley = (double)rawSurf->h / (double)rawDimensions.y;
            }
        }

        finSurf = SDL_CreateRGBSurface( SDL_SWSURFACE, rawDimensions.x, rawDimensions.y, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks
                                        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
                                        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
                                      );

        //DKS - Many of the original game's textures used magenta for a colorkey, instead
        //      of using an alpha channel. All those textures have been converted to
        //      now use an alpha channel, so SDL_SetColorKey() below is no longer necessary.
#if 0
        //      Check if original image uses an alpha channel
        //if (!(rawSurf->flags & SDL_SRCALPHA))
        if (rawSurf->format->BytesPerPixel <= 3 )
        {
            // if no alpha use MAGENTA and key it out.
            SDL_SetColorKey( rawSurf, flags, SDL_MapRGB( rawSurf->format, 255, 0, 255 ) );
        }
        else
#endif
        {
#if SDL_VERSION_ATLEAST(2,0,0)
            SDL_SetSurfaceAlphaMod( rawSurf, 255 );
#else /* SDL 1.2 */
            SDL_SetAlpha( rawSurf, 0, 0 );
#endif
        }

        SDL_BlitSurface( rawSurf, 0, finSurf, 0 );
        SDL_FreeSurface( rawSurf );

        factor = 1;

        if (((finSurf->w >= CommandLineParams.TexSizeMin) || (finSurf->h >= CommandLineParams.TexSizeMin))
                && (CommandLineParams.TexFactor > 1))
        {
            factor = CommandLineParams.TexFactor;
        }

        // Blacklist of image filenames (sub-strings) that shouldn't ever be resized, because of
        // resulting graphics glitches
        if (   fullpath.find("font")           != string::npos
            //|| fullpath.find("lightmap")     != string::npos           // Lightmaps were never actually used in the game
            || fullpath.find("hurrican_rund")  != string::npos             // Menu star/nebula background (ugly)
            || fullpath.find("roboraupe")      != string::npos             // Flat spiky enemy worm-like thing (glitches)
            || fullpath.find("enemy-walker")   != string::npos             // Frog-like robotic walker (glitches)
            || fullpath.find("stelzsack")      != string::npos             // Stilt-walker enemy on elevator level
           )
        {
            factor = 1;
        }

        if (factor > 1)
            image.data = LowerResolution( finSurf, factor );

        image.w = finSurf->w / factor;
        image.h = finSurf->h / factor;

        if (image.data == NULL)
        {
            image.size = finSurf->w*finSurf->h*sizeof(uint32_t);
            image.data = new uint8_t[image.size];
            memcpy( image.data, finSurf->pixels, image.size );
            image.type = GL_UNSIGNED_BYTE;
        }

        SDL_FreeSurface( finSurf );
    } else {
        Protokoll.WriteText( true, "Error in loadImageSDL: Could not read image data into rawSurf\n" );
        return false;
    }

    Protokoll.WriteText( false, "Using RGBA (SDL) ... " );

    return true;
}

//DKS - Assumes rgba8888 input, rgba8888 output
uint8_t* LowerResolution( SDL_Surface* surface, int factor )
{
    if (factor != 2 && factor != 4) {
        Protokoll.WriteText( false, "ERROR call to LowerResolution() with factor not equal to 2 or 4\n");
        return NULL;
    }

    if (surface->format->BytesPerPixel != 4) {
        Protokoll.WriteText( false, "ERROR call to LowerResolution() with source surface bpp other than 4\n");
        return NULL;
    }

    int x, y;
    uint8_t *dataout = new uint8_t[(surface->h / factor) * (surface->w / factor) * sizeof(uint32_t)];

    if (!dataout)
        return dataout;

    uint32_t *dataout32 = (uint32_t *)dataout;
    uint32_t *datain32 = (uint32_t *)surface->pixels;

    for (y=0; y<surface->h; y+=factor) {
        datain32 = ((uint32_t *)surface->pixels) + surface->w * y;
        for (x=0; x<surface->w; x+=factor) {
            *dataout32 = *datain32;
            datain32 += factor;
            dataout32++;
        }
    }

    return dataout;
}

#if 0   //DKS - Disabled ConvertRGBA5551, as it did not affect how GL stores textures in VRAM.
#if defined(RGBA_5551)
static uint8_t* ConvertRGBA5551( SDL_Surface* surface, uint8_t factor )
{
    uint8_t r, g, b, a;
    int32_t x, y;
    uint8_t* data = NULL;
    uint16_t* data16 = NULL;

    // Create the space for the 16 bpp image
    data = new uint8_t[(surface->w / factor) * (surface->h / factor) * sizeof(uint16_t)];

    for (y=0; y<surface->h; y+=factor)
    {
        for (x=0; x<surface->w; x+=factor)
        {
            get_components( surface, x, y, r, g, b, a );

            *data16 = ((uint16_t)(r>>3)<<11) | ((uint16_t)(g>>3)<<6) | ((uint16_t)(b>>3)<<1) | ((a > 0) ? 0x1 : 0x0);
            data16++;
        }
    }

    return data;
}
#endif
#endif

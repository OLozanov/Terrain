#include "Image.h"

#include <png.h>
#include "Render/Render.h"
#include <iostream>

Image* LoadPNG(const char* filename, bool mipmaps, bool rawdata)
{
    FILE* file;

	errno_t error = fopen_s(&file, filename, "rb");

	if (error)
	{
		std::cout << "Can't open file " << filename << std::endl;
		return nullptr;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) return nullptr;

	png_infop info = png_create_info_struct(png);
	if (!info) return nullptr;

	if (setjmp(png_jmpbuf(png))) return nullptr;

	png_init_io(png, file);
	png_read_info(png, info);

	Image* image = new Image();

	image->width = png_get_image_width(png, info);
	image->height = png_get_image_height(png, info);
	png_byte colortype = png_get_color_type(png, info);
	png_byte bitdepth = png_get_bit_depth(png, info);

	bool alpha = true;

	// These color_type don't have an alpha channel then fill it with 0xff.
	if (colortype == PNG_COLOR_TYPE_RGB ||
		colortype == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
		alpha = false;
	}

	if (colortype == PNG_COLOR_TYPE_GRAY)
		alpha = false;

	if (colortype == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	if (colortype == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png);
		image->format = VK_FORMAT_R8G8B8A8_UNORM;
	}
	else if (colortype == PNG_COLOR_TYPE_GRAY)
	{
		if (bitdepth < 8)
		{
			png_set_expand_gray_1_2_4_to_8(png);
			image->format = VK_FORMAT_R8_UNORM;
		}
		else if (bitdepth == 8)
		{
			image->format = VK_FORMAT_R8_UNORM;
		}
		else
		{
			image->format = VK_FORMAT_R16_UNORM;
			png_set_swap(png);
		}
	} 
	else
		image->format = VK_FORMAT_R8G8B8A8_UNORM;

	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);

	png_read_update_info(png, info);

	image->mipmaps = mipmaps ? log2(std::min(image->width, image->height)) + 1 : 1;
	size_t dataSize = image->size();

	uint32_t width = image->width;
	uint32_t height = image->height;

	size_t memsize = image->size();

	// Read image
	Render::Buffer buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, dataSize);
	uint8_t* data = reinterpret_cast<uint8_t*>(buffer.map(dataSize));

	png_bytep* rows = new png_bytep[image->height];

	for (uint32_t y = 0; y < image->height; y++) 
	{
		rows[y] = (png_byte*)data + png_get_rowbytes(png, info) * y;
	}

	png_read_image(png, rows);

	// Calculate mipmaps
	if (mipmaps) BuildMipmaps(data, image->width, image->height, image->format, image->mipmaps);

	// Copy image to CPU memory
	if (rawdata)
	{
		size_t size = image->width * image->height * pixelsize(image->format);
		image->data = new uint8_t * [size];
		memcpy(image->data, data, size);
	}

	buffer.unmap();
	fclose(file);

	png_destroy_read_struct(&png, &info, NULL);
	delete rows;

	Render::VulkanInstance::GetInstance().createTexture(buffer, *image);

    return image;
}
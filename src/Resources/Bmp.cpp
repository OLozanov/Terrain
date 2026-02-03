#include "Image.h"

#include <windows.h>
#include <stdio.h>

#include "Render/Render.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>

Image* LoadBMP(const char* filename)
{
	FILE * file;

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	errno_t error = fopen_s(&file, filename, "rb");

	if (error)
	{
		std::cout << "Can't open file " << filename << std::endl;
		return nullptr;
	}

	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
	if(fileHeader.bfType != 0x4d42) throw std::runtime_error("LoadBMP: incorrect header type!");

	size_t infoSz = fileHeader.bfOffBits - sizeof(BITMAPFILEHEADER);

	fread(&infoHeader, std::min(infoSz, sizeof(BITMAPINFOHEADER)), 1, file);

	if (infoSz > sizeof(BITMAPINFOHEADER)) fseek(file, fileHeader.bfOffBits, SEEK_SET);

	if(infoHeader.biBitCount < 24) throw std::runtime_error("LoadBMP: incorrect bit depth!");;
	if(infoHeader.biCompression != BI_RGB) throw std::runtime_error("LoadBMP: don't support compression!");

	size_t mipmaps = log2(std::min(infoHeader.biWidth, infoHeader.biHeight));

	size_t imageSize = infoHeader.biWidth*infoHeader.biHeight*sizeof(uint32_t);

	Image* image = new Image();

	image->format = VK_FORMAT_R8G8B8A8_UNORM;
	image->width = infoHeader.biWidth;
	image->height = infoHeader.biHeight;
	image->mipmaps = mipmaps + 1;

	image->mipmaps = mipmaps ? log2(std::min(image->width, image->height)) + 1 : 1;
	size_t dataSize = image->size();

	Render::Buffer buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, dataSize);
	uint8_t* data = reinterpret_cast<uint8_t*>(buffer.map(dataSize));
	uint8_t* pixels = data;

	if (infoHeader.biBitCount == 32)
	{
		fread(pixels, imageSize, 1, file);

		for (int i = 0; i < infoHeader.biWidth * infoHeader.biHeight; i++, pixels += 4)
		{
			uint8_t blue = pixels[0];
			pixels[0] = pixels[2];
			pixels[2] = blue;
		}
	}
	else
	{
		for (int i = 0; i < infoHeader.biWidth * infoHeader.biHeight; i++, pixels += 4)
		{
			fread(pixels + 2, 1, 1, file);
			fread(pixels + 1, 1, 1, file);
			fread(pixels, 1, 1, file);
			pixels[3] = 0xFF;
		}
	}

	buffer.unmap();

	fclose(file);

	if (mipmaps) BuildMipmaps(data, image->width, image->height, image->format, image->mipmaps);

	Render::VulkanInstance::GetInstance().createTexture(buffer, *image);

	return image;
}

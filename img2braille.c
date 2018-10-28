#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char **argv)
{
	if (argc < 2) printf("please supply a filname!");

	char *filename = argv[1];
	int width, height, bytes;
	unsigned char *data = stbi_load(filename, &width, &height, &bytes, 4);

	if (!data)
	{
		printf("couldn't open %s", filename);
		return 1;
	}

	float *result = (float *)malloc((width + 4) * (height + 2) * 4);

	int totalPixels = width * height;
	for (int i = 0; i < totalPixels; ++i)
	{
		float alpha = 0.055f;
		float r = data[i*4+0] / 255.0f;
		float g = data[i*4+1] / 255.0f;
		float b = data[i*4+2] / 255.0f;
		float a = data[i*4+3] / 255.0f;
		if (a == 0.0f)
		{
			result[i+2] = 1.0f;
			continue;
		}
		
		if (r <= 0.04045) r /= 12.92f;
		else r = pow((r + alpha) / (1.0f + alpha), 2.4);
		if (g <= 0.04045) g /= 12.92f;
		else g = pow((g + alpha) / (1.0f + alpha), 2.4);
		if (b <= 0.04045) b /= 12.92f;
		else b = pow((b + alpha) / (1.0f + alpha), 2.4);
		float x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
		float y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
		float z = r * 0.0193f + g * 0.1192f + b * 0.9505f;

		// if (y <= 0.0031308) y *= 12.92f;
		// else y = (1.0f + alpha) * pow(y, 1.0f/2.4f) - alpha;
		result[i+2] = y;
	}
	stbi_image_free(data);

	for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
		float error = result[y * width + x + 2];
		if (error >= 0.5f) error = error - 1.0f;
		error *= (1.0f/42.0f);

		result[y * width + x + 3] += error * 8.0f;
		result[y * width + x + 4] += error * 4.0f;
		
		result[(y + 1) * width + x    ] += error * 2.0f;
		result[(y + 1) * width + x + 1] += error * 4.0f;
		result[(y + 1) * width + x + 2] += error * 8.0f;
		result[(y + 1) * width + x + 3] += error * 4.0f;
		result[(y + 1) * width + x + 4] += error * 2.0f;

		result[(y + 2) * width + x    ] += error * 1.0f;
		result[(y + 2) * width + x + 1] += error * 2.0f;
		result[(y + 2) * width + x + 2] += error * 4.0f;
		result[(y + 2) * width + x + 3] += error * 2.0f;
		result[(y + 2) * width + x + 4] += error * 1.0f;
	}

	unsigned char *finalResult = (unsigned char *)malloc(width * height);
	for (int i = 0; i < totalPixels; ++i)
		finalResult[i] = (result[i+2] < 0.5f) ? 0 : 255;
	free(result);

	FILE *file = fopen("result.txt", "w");

	for (int y = 0; y < (height >> 2); ++y)
	{
		for (int x = 0; x < (width >> 1); ++x)
		{
			int yy = y << 2;
			int xx = x << 1;
			unsigned char braille = 0;
			
			if (!finalResult[(yy + 0) * width + xx]) braille |= 1;
			if (!finalResult[(yy + 1) * width + xx]) braille |= 2;
			if (!finalResult[(yy + 2) * width + xx]) braille |= 4;

			if (!finalResult[(yy + 0) * width + xx + 1]) braille |= 8;
			if (!finalResult[(yy + 1) * width + xx + 1]) braille |= 16;
			if (!finalResult[(yy + 2) * width + xx + 1]) braille |= 32;
			
			if (!finalResult[(yy + 3) * width + xx])     braille |= 64;
			if (!finalResult[(yy + 3) * width + xx + 1]) braille |= 128;
			
			unsigned char utf8[3] = { 0xE2, 0xA0, 0x80 };
			utf8[1] |= braille >> 6;
			utf8[2] |= braille & 0x3F;
			fwrite(utf8, 3, 1, file);
		}
		fwrite("\n", 1, 1, file);
	}
	free(finalResult);
  	fclose (file);

    return 0;
}

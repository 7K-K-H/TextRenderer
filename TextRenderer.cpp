#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class TextRenderer {
private:
	FT_Library library;
	FT_Face face;
	int width;
	int height;
	int bytesPerPixel;

public:
	TextRenderer(int imgWidth, int imgHeight, int bpp)
		: width(imgWidth), height(imgHeight), bytesPerPixel(bpp) {
		// Initialize FreeType library
		if (FT_Init_FreeType(&library)) {
			throw std::runtime_error("Could not initialize FreeType library");
		}
	}

	~TextRenderer() {
		FT_Done_Face(face);
		FT_Done_FreeType(library);
	}

	void loadFont(const std::string& fontPath, int fontSize) {
		if (FT_New_Face(library, fontPath.c_str(), 0, &face)) {
			throw std::runtime_error("Could not load font");
		}
		FT_Set_Pixel_Sizes(face, 0, fontSize);
	}

	// Function to calculate the maximum height of the text
	int calculateTextHeight(const std::string& text) {
		int max_height = 0;
		for (char c : text) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				continue; // If error, skip this character
			}
			if (face->glyph->bitmap_top > max_height) {
				max_height = face->glyph->bitmap_top;
			}
		}
		return max_height;
	}

	void drawTextOnImage(std::vector<uint16_t>& image, const std::string& text, int startX, int startY) {
		FT_GlyphSlot slot = face->glyph;
		int penX = startX;
		int penY = startY;

		int max_height = calculateTextHeight(text);

		for (char c : text) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				continue; // If error, skip this character
			}

			FT_Bitmap bitmap = slot->bitmap;
			for (int row = 0; row < (int)bitmap.rows; ++row) {
				for (int col = 0; col < (int)bitmap.width; ++col) {
					int x = penX + col + slot->bitmap_left;
					int y = penY - (max_height - slot->bitmap_top) - row;

					// Reverse the y coordinate to flip the image vertically
					int flipped_y = height - y - 1;

					if (x >= 0 && x < width && flipped_y >= 0 && flipped_y < height) {
						image[flipped_y * width + x] = bitmap.buffer[row * bitmap.pitch + col] ? 65535 : image[flipped_y * width + x];
					}
				}
			}

			penX += slot->advance.x >> 6;
			penY += slot->advance.y >> 6;
		}
	}

	void saveImage(const std::string& outputPath, const std::vector<uint16_t>& image) {
		std::ofstream outfile(outputPath, std::ios::binary);
		outfile.write(reinterpret_cast<const char*>(image.data()), image.size() * sizeof(uint16_t));
		outfile.close();
	}

	void loadRawImage(const std::string& filePath, std::vector<uint16_t>& image) {
		std::ifstream infile(filePath, std::ios::binary);
		if (!infile) {
			throw std::runtime_error("Could not open raw image file");
		}
		infile.read(reinterpret_cast<char*>(image.data()), image.size() * sizeof(uint16_t));
		infile.close();
	}
};

int main() {
	// Define image dimensions and bytes per pixel
	const int WIDTH = 3072;
	const int HEIGHT = 3072;
	const int BYTES_PER_PIXEL = 2; // 16-bit grayscale

	// Create a TextRenderer object
	TextRenderer renderer(WIDTH, HEIGHT, BYTES_PER_PIXEL);

	// Load font and set font size
	try {
		renderer.loadFont("C:/Windows/Fonts/malgun.ttf", 48);
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	// Initialize image vector to hold the raw data
	std::vector<uint16_t> image(WIDTH * HEIGHT, 0);

	// Load raw image data from file
	try {
		renderer.loadRawImage("C:\\DR\\1717.raw", image); // Adjust the path as necessary
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	// Input text
	std::string text = "Your Text Here";

	// Calculate start positions for centering the text
	int startX = (WIDTH - text.length() * 24) / 2; // Rough estimation for centering
	int startY = HEIGHT / 2 + 24 / 2;

	// Draw the text on the image
	renderer.drawTextOnImage(image, text, startX, startY);

	// Save the image as raw data
	renderer.saveImage("output.raw", image);

	return 0;
}

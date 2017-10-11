#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <ctgmath>

const char alpha_magic[] = "RALPHA";
const int16_t alpha_version = 1;

std::vector<std::string> args;

static inline void read_palette(std::string fname, uint8_t* dest)
{
	std::ifstream fobj;
	fobj.open(fname);
	fobj.read((char*) dest, 768);
	fobj.close();
}

int main(int argc, char** argv)
{
	args = std::vector<std::string>(argv, argv + argc);
	
	if (args.size() != 5)
	{
		std::cerr << "usage: " << args[0] << " ORIGINAL_PALETTE ORIGINAL_ALPHA NEW_PALETTE OUT_ALPHA" << std::endl;
		return EXIT_FAILURE;
	}
	
	uint8_t orig_pal[768], new_pal[768];
	read_palette(args[1], orig_pal);
	read_palette(args[3], new_pal);
	
	std::ifstream original_alpha_fobj;
	original_alpha_fobj.open(args[2]);
	char header[7];
	original_alpha_fobj.read(header, 7);
	if (std::memcmp(header, alpha_magic, 7) != 0)
	{
		std::cerr << args[2] << " is not an RAlpha" << std::endl;
		return EXIT_FAILURE;
	}
	int16_t ver;
	original_alpha_fobj.read((char*) &ver, 2);
	if (ver != alpha_version)
	{
		std::cerr << "this RAlpha is version " << ver << ", expected " << alpha_version << std::endl;
		return EXIT_FAILURE;
	}
	int16_t depth;
	original_alpha_fobj.read((char*) &depth, 2);
	
	std::ofstream new_alpha_fobj;
	new_alpha_fobj.open(args[4]);
	new_alpha_fobj.write(header, 7);
	new_alpha_fobj.write((char*) &ver, 2);
	new_alpha_fobj.write((char*) &depth, 2);
	
	for (int_fast32_t i = 0; i < 256 * depth; i++)
	{
		uint8_t orig_entry, new_entry = 0;
		original_alpha_fobj.read((char*) &orig_entry, 1);
		uint8_t orig_r = orig_pal[orig_entry * 3];
		uint8_t orig_g = orig_pal[orig_entry * 3 + 1];
		uint8_t orig_b = orig_pal[orig_entry * 3 + 2];
		int_fast16_t max = 768;
		for (int_fast16_t i = 0; i < 256; i++)
		{
			uint8_t new_r = new_pal[i * 3];
			uint8_t new_g = new_pal[i * 3 + 1];
			uint8_t new_b = new_pal[i * 3 + 2];
			int_fast16_t diff = std::abs(new_r - orig_r) + std::abs(new_g - orig_g) + std::abs(new_b - orig_b);
			if (diff < max)
			{
				max = diff;
				new_entry = i;
			}
		}
		new_alpha_fobj.write((char*) &new_entry, 1);
	}
	
	original_alpha_fobj.close();
	new_alpha_fobj.close();
	return EXIT_SUCCESS;
}


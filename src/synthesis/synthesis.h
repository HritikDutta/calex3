#pragma once
#include "core/types.h"
#include "containers/string.h"
#include "core/coroutines.h"
#include "math/math.h"

namespace Synthesis
{

struct Image
{
    s32 width, height, bytes_pp;
    u8* pixels;
};

bool image_load_from_file(const String filepath, Image& image);
void image_inflate_to_rgba(Image& image);

void synthesize_image(Coroutine& co, const Image& input_image, const Vector2 output_size, const s32 kernel_size, Image& output_image);

} // namespace Synthesis

void free(Synthesis::Image& image);
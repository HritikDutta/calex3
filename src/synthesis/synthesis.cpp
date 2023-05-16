#include "synthesis.h"

#include "core/logger.h"
#include "core/types.h"
#include "containers/string.h"
#include "containers/darray.h"
#include "core/coroutines.h"
#include "math/math.h"
#include "platform/platform.h"

#include <stb_image.h>

namespace Synthesis
{

static constexpr u32 SHIFT_RED   =  0;
static constexpr u32 SHIFT_GREEN =  8;
static constexpr u32 SHIFT_BLUE  = 16;
static constexpr u32 SHIFT_ALPHA = 24;

struct PixelIndex
{
    s32 x, y;
};

enum struct PixelStage : u8
{
    EMPTY,
    IN_QUEUE,
    FILLED,

    NUM_TYPES
};

static inline constexpr u32 pack_color(u8 red, u8 green, u8 blue, u8 alpha)
{
    return (red << SHIFT_RED) | (green << SHIFT_GREEN) | (blue << SHIFT_BLUE) | (alpha << SHIFT_ALPHA);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE
static Vector4 unpack_color(u32 packed_color)
{
    return Vector4 {
        ((packed_color >> SHIFT_RED)   & 0xFF) / 255.0f,
        ((packed_color >> SHIFT_GREEN) & 0xFF) / 255.0f,
        ((packed_color >> SHIFT_BLUE)  & 0xFF) / 255.0f,
        ((packed_color >> SHIFT_ALPHA) & 0xFF) / 255.0f
    };
}

bool image_load_from_file(const String filepath, Image& image)
{
    image.pixels = stbi_load(filepath.data, &image.width, &image.height, &image.bytes_pp, 0);
    gn_assert_with_message(image.pixels, "Couldn't load image data! (filepath: \"%\")", filepath);
    return image.pixels != nullptr;
}

void image_inflate_to_rgba(Image& image)
{
    // Working with u32 to perform the operation faster
    u32* inflated = (u32*) platform_allocate(image.width * image.height * 4);
    gn_assert_with_message(inflated, "Couldn't allocate memory for inflated image!");

    switch (image.bytes_pp)
    {
        case 1:
        {
            for (s32 i = 0; i < image.width * image.height * image.bytes_pp; i++)
            {
                u32 value = image.pixels[i];
                inflated[i / image.bytes_pp] = pack_color(value, value, value, 0xFF);
            }
        } break;

        case 2:
        {
            for (s32 i = 0; i < image.width * image.height * image.bytes_pp; i += 2)
                inflated[i / image.bytes_pp] = pack_color(image.pixels[i], image.pixels[i + 1], 0x00, 0xFF);
        } break;

        case 3:
        {
            for (s32 i = 0; i < image.width * image.height * image.bytes_pp; i += 3)
                inflated[i / image.bytes_pp] = pack_color(image.pixels[i], image.pixels[i + 1], image.pixels[i + 2], 0xFF);
        } break;

        case 4:
        {
            // Skip conversion
        } break;

        default:
        {
            gn_assert_with_message(false, "Bytes per pixel value is not currently supported! (bytes per pixel: %)", image.bytes_pp);
        } break;
    }

    platform_free(image.pixels);

    image.pixels = (u8*) inflated;
    image.bytes_pp = 4;
}

struct Context
{
    Image const* input_image;
    Image* output_image;
    s32 kernel_size;

    u32* output_pixels;
    PixelStage* pixel_stages;

    f32* gaussian;
    f32* deviations;

    DynamicArray<PixelIndex> pixel_queue;

    u32 filled;
};

static inline void free(Context& context)
{
    platform_free(context.pixel_stages);
    platform_free(context.gaussian);
    platform_free(context.deviations);
    free(context.pixel_queue);
}

static inline void add_pixel_to_queue(Context& context, PixelIndex pixel_index)
{
    append(context.pixel_queue, pixel_index);
    const s32 index = (context.output_image->height - pixel_index.y - 1) * context.output_image->width + pixel_index.x;
    context.pixel_stages[index] = PixelStage::IN_QUEUE;
}

static inline PixelIndex pop_pixel_from_queue(Context& context)
{
    PixelIndex pixel_index = remove(context.pixel_queue, 0);
    const s32 index = (context.output_image->height - pixel_index.y - 1) * context.output_image->width + pixel_index.x;
    context.pixel_stages[index] = PixelStage::FILLED;

    return pixel_index;
}

static inline s32 get_filled_neighbour_count(Context& context, s32 x, s32 y)
{
    s32 filled_neighbours = 0;

    for (s32 ny = max(y - 1, 0); ny <= min(y + 1, context.output_image->height - 1); ny++)
    for (s32 nx = max(x - 1, 0); nx <= min(x + 1, context.output_image->width  - 1); nx++)
    {
        if (nx == x && ny == y)
            continue;
        
        const s32 index = (context.output_image->height - ny - 1) * context.output_image->width + nx;
        filled_neighbours += (context.pixel_stages[index] == PixelStage::FILLED);
    }

    return filled_neighbours;
}

// Returns the number of matches found
static s32 find_matches(Context& context, PixelIndex pixel_index, u32* best_matches, f32* errors, const u32 max_matches_allowed, const f32 max_error_tolerance)
{
    s32 matches_found = 0;

    u32 const* input_image_pixels_as_32 = (u32*) context.input_image->pixels;

    // Get neighbourhood window in output image
    PixelIndex output_top_left     = PixelIndex { max(pixel_index.x - context.kernel_size / 2, 0), max(pixel_index.y - context.kernel_size / 2, 0) };
    PixelIndex output_bottom_right = PixelIndex { min(pixel_index.x + context.kernel_size / 2, context.output_image->width - 1), min(pixel_index.y + context.kernel_size / 2, context.output_image->height - 1) };

    // Clear all deviations
    platform_set_memory(context.deviations, 0x00, context.input_image->width * context.input_image->height * sizeof(f32));

    f32 min_deviation = Math::infinity;

    for (s32 y = context.kernel_size / 2; y < context.input_image->height - context.kernel_size / 2; y++)
    for (s32 x = context.kernel_size / 2; x < context.input_image->width  - context.kernel_size / 2; x++)
    {
        const s32 input_pixel_index = (context.input_image->height - y - 1) * context.input_image->width + x;
        
        f32 total_weight = 0.0f;
        f32 deviation = 0.0f;

        // Check neibourhood window
        for (s32 ky = output_top_left.y; ky <= output_bottom_right.y; ky++)
        for (s32 kx = output_top_left.x; kx <= output_bottom_right.x; kx++)
        {
            const s32 output_index = (context.output_image->height - ky - 1) * context.output_image->width + kx;

            const PixelIndex offset = PixelIndex { kx - pixel_index.x, ky - pixel_index.y };
            const s32 input_index  = (context.input_image->height  - (y + offset.y) - 1) * context.input_image->width  + (x + offset.x);

            const Vector4 input_color  = unpack_color(input_image_pixels_as_32[input_index]);
            const Vector4 output_color = unpack_color(context.output_pixels[output_index]);

            // TODO: Find a way to find difference using u32 rather than Vector4
            const f32 diff = sqr_length(output_color - input_color);
            
            if (context.pixel_stages[output_index] == PixelStage::FILLED)
            {
                const u32 gaussian_index = (offset.y + context.kernel_size / 2) * context.kernel_size + (offset.x + context.kernel_size / 2);
                deviation += diff * context.gaussian[gaussian_index];
                total_weight += context.gaussian[gaussian_index];
            }
        }

        // If deviation is exactly 0 then it's sus
        deviation = (deviation == 0.0f) ? Math::infinity : deviation;
        deviation /= total_weight;

        min_deviation = min(deviation, min_deviation);
        context.deviations[input_pixel_index] = deviation;
    }
    
    f32 max_deviation = min_deviation * (1 + max_error_tolerance);
    for (s32 y = context.kernel_size / 2; y < (context.input_image->height - context.kernel_size / 2); y++)
    for (s32 x = context.kernel_size / 2; x < (context.input_image->width - context.kernel_size / 2); x++)
    {
        const s32 input_index = (context.input_image->height - y - 1) * context.input_image->width + x;

        if (context.deviations[input_index] <= max_deviation)
        {
            best_matches[matches_found] = input_image_pixels_as_32[input_index];
            errors[matches_found] = context.deviations[input_index];
            matches_found++;

            // No more matches need to be found!
            if (matches_found == max_matches_allowed)
                return matches_found;
        }
    }

    return matches_found;
}

// Only works on RGBA textures
void synthesize_image(Coroutine& co, const Image& input_image, const Vector2 output_size, const s32 kernel_size, Image& output_image)
{
    Context& context = coroutine_stack_variable<Context>(co);
    f32& max_error = coroutine_stack_variable<f32>(co);

    constexpr u32 max_matches_allowed = 256;

    coroutine_start(co);

    // Lots of checks
    gn_assert_with_message(input_image.bytes_pp == 4, "Input image should be RGBA! (input image channel count: %)", input_image.bytes_pp);

    gn_assert_with_message(kernel_size % 2 == 1, "Kernel size should be an odd number! (kernel size: %)", kernel_size);

    gn_assert_with_message(input_image.height > kernel_size, "Kernel size larger than input image height! (input height: %, kernel size: %)", input_image.height, kernel_size);

    gn_assert_with_message(input_image.width  < output_size.x, "Output image width smaller than input image! (input width: %, output width: %)", input_image.width, output_size.x);
    gn_assert_with_message(input_image.height < output_size.y, "Output image height smaller than input image! (input height: %, output height: %)", input_image.height, output_size.y);

    output_image.bytes_pp = input_image.bytes_pp;
    output_image.width  = output_size.x;
    output_image.height = output_size.y;

    // Initialize context
    context.input_image  = &input_image;
    context.output_image = &output_image;
    context.kernel_size  = kernel_size;

    // Working with u32 to perform operations faster
    context.output_pixels = (u32*) platform_allocate(output_image.width * output_image.height * output_image.bytes_pp);
    platform_set_memory(context.output_pixels, 0x22, output_image.width * output_image.height * output_image.bytes_pp);
    output_image.pixels = (u8*) context.output_pixels;

    context.pixel_stages = (PixelStage*) platform_allocate(output_image.width * output_image.height * sizeof(PixelStage));
    platform_set_memory(context.pixel_stages, (u8) PixelStage::EMPTY, output_image.width * output_image.height * sizeof(PixelStage));

    // Create gaussian kernel
    context.gaussian = (f32*) platform_allocate(kernel_size * kernel_size * sizeof(f32));

    // Allocate array for deviations
    context.deviations = (f32*) platform_allocate(context.input_image->width * context.input_image->height * sizeof(f32));

    {   // Fill gaussian kernel
        f32 max_val = 0.0f;

        const f32 sigma = (kernel_size - 1) / 6.4f;
        const f32 div1 = 1.0f / (2 * sigma * sigma);
        const f32 div2 = 1.0f / (2 * Math::PI * sigma * sigma);

        for (s32 i = 0; i < kernel_size * kernel_size; i++)
        {
            f32 x = ((f32) (i % kernel_size) - (f32) (kernel_size / 2));
            f32 y = ((f32) (i / kernel_size) - (f32) (kernel_size / 2));
            context.gaussian[i] = div2 * Math::exp(-(x * x + y * y) * div1);

            max_val += context.gaussian[i];
        }

        // Normalize values
        for (s32 i = 0; i < kernel_size * kernel_size; i++)
            context.gaussian[i] /= max_val;
    }

    // Create queue for unfilled pixels
    context.pixel_queue = make<DynamicArray<PixelIndex>>((u64) output_image.width);

    coroutine_yield(co);

    {   // Copy the source image to the center of the output image
        u32 const* input_image_pixels_as_32 = (u32*) input_image.pixels;

        s32 start_x = (output_image.width - input_image.width) / 2;
        s32 start_y = (output_image.height - input_image.height) / 2;
        
        for (s32 y = 0; y < input_image.height; y++)
        for (s32 x = 0; x < input_image.width; x++)
        {
            const s32 input_image_index  = (input_image.height  - y - 1) * input_image.width + x;
            const s32 output_image_index = (output_image.height - (y + start_y) - 1) * output_image.width + (x + start_x);

            context.output_pixels[output_image_index] = input_image_pixels_as_32[input_image_index];
            context.pixel_stages[output_image_index] = PixelStage::FILLED;

            context.filled++;
        }

        add_pixel_to_queue(context, PixelIndex { start_x + input_image.width, start_y });
        add_pixel_to_queue(context, PixelIndex { start_x, start_y + input_image.height });
    }

    coroutine_yield(co);

    u32 best_matches[max_matches_allowed];
    f32 errors[max_matches_allowed];
    s32 current_index, matches;

    max_error = 0.3f;
    while (context.pixel_queue.size > 0)
    {
        PixelIndex pixel_index = pop_pixel_from_queue(context);
        current_index = (output_image.height - pixel_index.y - 1) * output_image.width + pixel_index.x;

        // Get best matches
        matches = find_matches(context, pixel_index, best_matches, errors, max_matches_allowed, 0.05f);

        // Use a better way to add pixels to the queue
        // Subtraction from morphological dilation?
        
        {   // Fill pixel
            const s32 random_index = Math::floor((matches * Math::random()));

            if (errors[random_index] < max_error)
            {
                context.output_pixels[current_index] = best_matches[random_index];
                context.filled++;
            }
            else
            {
                add_pixel_to_queue(context, pixel_index);
                max_error *= 1.1f;
                continue;
            }
        }

        // Add neighbouring pixels to queue
        for (int y = max(0, pixel_index.y - 1); y <= min(output_image.height - 1, pixel_index.y + 1); y++)
        for (int x = max(0, pixel_index.x - 1); x <= min(output_image.width  - 1, pixel_index.x + 1); x++)
        {
            // Ignore current pixel
            if (pixel_index.x == x && pixel_index.y == y)
                continue;
            
            // Don't add pixels with less than 3 filled neighbours
            if (get_filled_neighbour_count(context, x, y) < 3)
                continue;
            
            // Only add empty pixels to queue
            s32 index = (output_image.height - y - 1) * output_image.width + x;
            if (context.pixel_stages[index] == PixelStage::EMPTY)
                add_pixel_to_queue(context, PixelIndex { x, y });
        }

        print("\r Progress: %", (f32) context.filled / (f32) (output_image.width * output_image.height));

        coroutine_yield(co);
    }

    coroutine_yield(co);

    // Free things
    free(context);

    coroutine_end(co);
}

} // namespace Synthesis

void free(Synthesis::Image& image)
{
    stbi_image_free(image.pixels);

    image.pixels = nullptr;
    image.width = image.height = image.bytes_pp = 0;
}
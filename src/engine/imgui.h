#pragma once

#include "application/application.h"
#include "core/types.h"
#include "containers/bytes.h"
#include "containers/hash_table.h"
#include "graphics/texture.h"
#include "math/math.h"
#include "serialization/json/json_document.h"

namespace Imgui
{

// Initialization and Shutdown
void init(const Application& app);
void shutdown();

// Begin and End UI Pass
void begin();
void end();

// Required for proper functioning UI
void update();

struct ID
{
    s32 primary;
    s32 secondary;
    
    inline bool operator==(const ID& other)
    {
        return primary == other.primary &&
                secondary == other.secondary;
    }

    inline bool operator!=(const ID& other)
    {
        return primary != other.primary ||
                secondary != other.secondary;
    }
};

struct Rect
{
    Vector3 top_left;
    Vector2 size;
};

using Image = Texture;

struct Font
{
    struct GlyphData
    {
        f32 advance;
        Vector4 plane_bounds;
        Vector4 atlas_bounds;
    };

    using KerningTable = HashTable<s32, f32>;

    Texture atlas;

    u32 size;
    f32 line_height;
    f32 ascender, descender;

    GlyphData glyphs[127 - ' '];
    KerningTable kerning_table = {};
};

// Deserialization
Font  font_load_from_json(const Json::Document& document, const String atlas_path);
Font  font_load_from_bytes(const Bytes& bytes);

// Utility Functions
Vector2 get_rendered_text_size(const String text, const Font& font, f32 size = -1.0f);
Vector2 get_rendered_char_size(const char ch, const Font& font, f32 size = -1.0f);

// Rendering UI
void render_rect(const Rect& rect, const Vector4& color);
void render_overlap_rect(ID id, const Rect& rect, const Vector4& color);   // Overlap rect prevents items under it from being interacted with
void render_image(const Image& image, const Vector3& top_left, const Vector2& size = Vector2(-1.0f), const Vector4& tint = Vector4(1.0f));
bool render_button(ID id, const Rect& rect, const Vector4& default_color = Vector4(0.5f, 0.5f, 0.5f, 1.0f), const Vector4& hover_color = Vector4(0.75f, 0.75f, 0.75f, 1.0f), const Vector4& pressed_color = Vector4(0.35f, 0.35f, 0.35f, 1.0f));

void render_text(const String text, const Font& font, const Vector3& top_left, f32 size = -1.0f, const Vector4& tint = Vector4(1.0f));
void render_char(const char ch, const Font& font, const Vector3& top_left, f32 size = -1.0f, const Vector4& tint = Vector4(1.0f));

} // namespace Imgui

void free(Imgui::Font& font);

#define gen_imgui_id() (Imgui::ID { __LINE__, 0 })
#define gen_imgui_id_with_secondary(sec) (Imgui::ID { __LINE__, (sec) })
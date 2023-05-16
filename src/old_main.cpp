#include "application/application.h"
#include "core/logger.h"
#include "core/coroutines.h"
#include "core/input.h"
#include "fileio./fileio.h"
#include "fileio./compression.h"
#include "engine/imgui.h"
#include "game/game.h"
#include "math/common.h"
#include "graphics/texture.h"

#include "game/game_package.h"
#include "game/game_loader.h"

void init(Application& app)
{
    GameData& data = *(GameData*) app.data;

    // Note: Used when packaging data for build

    // {   // Pack Assets
    //     Bytes bytes = Package::pack_assets();
    //     Bytes compressed = compress_bytes(bytes);

    //     file_write_bytes(ref("package.bytes"), compressed);

    //     free(compressed);
    //     free(bytes);
    // }
    
    // {   // Save Default Settings
    //     Bytes bytes = Package::pack_settings_default(app, data);
    //     Bytes compressed = compress_bytes(bytes);

    //     file_write_bytes(ref("settings.bytes"), compressed);

    //     free(compressed);
    //     free(bytes);
    // }

    // {   // Pack Shaders
    //     String contents = Package::pack_shaders();
    //     file_write_string(ref("src/engine/packed_shaders.h"), contents);
    //     free(contents);
    // }
    
    {   // Load Assets
        Bytes bytes = file_load_bytes(ref("package.bytes"));
        Bytes uncompressed = decompress_bytes(bytes);
        
        game_load_assets(uncompressed, data);

        free(uncompressed);
        free(bytes);
    }

    {   // Load settings
        Bytes bytes = file_load_bytes(ref("settings.bytes"));
        Bytes uncompressed = decompress_bytes(bytes);

        game_load_settings(uncompressed, app, data);

        free(uncompressed);
        free(bytes);
    }

    // Setup Game
    game_init(app, data);
}

void update(Application& app)
{
    GameData& data = *(GameData*) app.data;

    if (data.request_shutdown)
    {
        app.is_running = false;
        return;
    }

    if (data.save_settings)
    {
        Bytes bytes = Package::pack_settings(app, data);
        Bytes compressed = compress_bytes(bytes);

        file_write_bytes(ref("settings.bytes"), compressed);

        free(compressed);
        free(bytes);

        data.save_settings = false;
    }

    if (data.initial_loading)
        return;

    if (data.active_game_windows.size > 0)
    {
        f32 load_speed = data.current_project_difficulty.loading_speed_base;

        if (sqr_length(Input::mouse_delta_position()) >= 0.001f)
            load_speed *= 0.5f;

        load_speed *= 1.0f / (data.active_game_windows.size * data.active_game_windows.size);

#ifdef GN_DEBUG
        data.load_speed_multiplier = load_speed / data.current_project_difficulty.loading_speed_base;
#endif // GN_DEBUG

        data.loading_progress += data.started_game ? load_speed * app.delta_time : 0.0f;
    }

    if (app.time >= data.next_pop_up_time)
    {
        constexpr f32 chance_of_notification = 0.35f;

        if (!data.notification_active && Math::random() <= chance_of_notification)
        {
            game_window_register(data, game_window_notification, Vector2 {});
        }
        else
        {
            constexpr f32 chance_of_clickbait = 0.65f;
            Vector2 position = Vector2 { (Math::random() * (app.window.ref_width - 600)) + 200, (Math::random() * (app.window.ref_height - 400)) + 80 };

            if (!data.baited && Math::random() <= chance_of_clickbait)
                game_window_register(data, game_window_click_bait, position);
            else
                game_window_register(data, game_window_random_pop_up, position);
        }

        data.next_pop_up_time = game_decide_next_interruption_time(app, data);
    }

    if (data.loading_progress > 1.0f && !data.loading_finished)
    {
        data.loading_end_time = app.time;
        data.loading_finished = true;
    }

#ifdef GN_DEBUG
    if (Input::get_key_down(Key::GRAVE))
        data.is_debug = !data.is_debug;
#endif // GN_DEBUG
}

void render(Application& app)
{
    GameData& data = *(GameData*) app.data;

    Imgui::begin();

    {   // Render Wallpaper
        Imgui::render_image(data.desktop_wallpaper, Vector3 { 0.0f, 0.0f, 0.999f }, Vector2 { (f32) app.window.ref_width, (f32) app.window.ref_height });
    }

    {   // Shutdown Button
        constexpr f32 padding = 20.0f;
        Imgui::Rect rect;
        rect.size = Vector2 { 35.0f, 35.0f };
        rect.top_left = Vector3 { padding, app.window.ref_height - rect.size.y - padding, 0.991f };

        Imgui::render_image(data.shutdown_button_image, rect.top_left + Vector3 { 0.0f, 0.0f, 0.001f }, rect.size);

        const Vector4 default_color = Vector4 {};
        const Vector4 hover_color   = Vector4 { 1.0f, 1.0f, 1.0f, 0.25f };
        if (Imgui::render_button(gen_imgui_id(), rect, default_color, hover_color, hover_color))
            game_window_register(data, game_window_shutdown, Vector2 {});
    }

    game_render_shortcuts(app, data);
    game_render_active_windows(app, data);

#ifdef GN_DEBUG
    if (data.is_debug)
    {
        char buffer[128];
        sprintf(buffer, "Active windows: %llu\nLoading Speed: %.3f", data.active_game_windows.size, data.load_speed_multiplier);

        String text = ref(buffer);
        Vector2 size = Imgui::get_rendered_text_size(text, data.ui_font, 25.0f);
        
        constexpr f32 padding = 5.0f;

        Imgui::Rect rect;
        rect.size = size + Vector2 { 2 * padding, 2 * padding };
        rect.top_left = Vector3 { app.window.ref_width - rect.size.x, 0.0f, -0.9f };

        Imgui::render_rect(rect, Vector4 { 0.0f, 0.0f, 0.0f, 0.5f });
        Imgui::render_text(text, data.ui_font, Vector3 { rect.top_left.x + padding, rect.top_left.y + padding, rect.top_left.z - 0.001f }, 25.0f);
    }
#endif // GN_DEBUG

    Imgui::end();

    game_post_render(data);
}

void create_app(Application& app)
{
    app.window.x = 500;
    app.window.y = 500;
    app.window.width  = 1280;
    app.window.height = 720;
    app.window.ref_height = 900;
    app.window.name = ref("Loading Screen Game!");
    app.window.icon_path = ref("assets/art/game_icon.ico");

    app.on_init   = init;
    app.on_update = update;
    app.on_render = render;

    app.data = (void*) platform_allocate(sizeof(GameData));
    gn_assert_with_message(app.data, "Couldn't allocate game data!");
    (*(GameData*) app.data) = GameData();    // Initialize to default values
}
#pragma once

#include <imgui.h>
#include <imgui_internal.h>


struct SliderAnimationState
{
    float fill_anim_ratio;
    float anim_t;
};

struct CardScope
{
    bool open = false;
    void End() {
        if (open) ImGui::EndChild();
    }
};

struct CenterCard
{
    ImDrawList *dl = nullptr;
    ImVec2 frame_min{};
    ImVec2 frame_max{};
    float card_w = 0.0f;
    float pad_x = 7.0f;   
    float pad_y = 2.0f;  
    float rounding = 10.0f;
    ImU32 bg_col = 0;
    ImU32 border_col = 0;
    bool with_header = true;
};


namespace CustomMenu
{
    inline CenterCard BeginCenteredCard( const char *title, float card_width, bool with_header = true ) {
        CenterCard c;
        c.dl = ImGui::GetWindowDrawList();
        c.with_header = with_header;
        c.card_w = card_width;

        const ImVec4 bg = ImGui::GetStyleColorVec4( ImGuiCol_ChildBg );
        const ImVec4 br = ImGui::GetStyleColorVec4( ImGuiCol_Border );
        c.bg_col = ImGui::ColorConvertFloat4ToU32( bg );
        c.border_col = ImGui::ColorConvertFloat4ToU32( br );

        const float avail_w = ImGui::GetContentRegionAvail().x;
        float x_local = ImGui::GetCursorPosX() + (avail_w - c.card_w) * 0.5f;
        if (x_local < ImGui::GetCursorPosX()) x_local = ImGui::GetCursorPosX();

        ImGui::SetCursorPosX( x_local );
        c.frame_min = ImGui::GetCursorScreenPos();

        c.dl->ChannelsSplit( 2 );
        c.dl->ChannelsSetCurrent( 1 ); // content

        ImGui::PushID( title );

        ImGui::Dummy( ImVec2( 0, c.pad_y ) );

        ImGui::Indent( c.pad_x );

        ImGui::PushItemWidth( c.card_w - 2.0f * c.pad_x );

        ImGui::BeginGroup();

        if (with_header)
        {
            ImGui::TextUnformatted( title );
            ImGui::Separator();
            ImGui::Spacing();
        }

        return c;
    }

    inline void EndCenteredCard( CenterCard &c ) {
        ImGui::EndGroup();
        ImGui::PopItemWidth();

        ImGui::Dummy( ImVec2( 0, c.pad_y ) );

        ImGui::Unindent( c.pad_x );

        ImVec2 content_max = ImGui::GetItemRectMax();

        c.frame_max.x = c.frame_min.x + c.card_w;
        c.frame_max.y = content_max.y + c.pad_y;

        c.dl->ChannelsSetCurrent( 0 );
        c.dl->AddRectFilled( c.frame_min, c.frame_max, c.bg_col, c.rounding );
        c.dl->AddRect( c.frame_min, c.frame_max, c.border_col, c.rounding, 0, 1.0f );

        c.dl->ChannelsMerge();
        ImGui::PopID();

        ImGui::Spacing();
    }

    inline bool ModernTextField( const char *label, char *buf, size_t buf_size,
        const char *hint = "", float width = 0.0f, ImGuiInputTextFlags flags = 0 ) {
        if (width <= 0.0f) width = ImGui::GetContentRegionAvail().x;

        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.75f, 0.78f, 0.85f, 1 ) );
        ImGui::TextUnformatted( label );
        ImGui::PopStyleColor();

        ImGui::PushID( label );

        ImGui::PushItemWidth( width );
        ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 8.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1.0f );
        ImGui::PushStyleColor( ImGuiCol_FrameBg, ImGui::GetStyleColorVec4( ImGuiCol_FrameBg ) );
        ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImGui::GetStyleColorVec4( ImGuiCol_FrameBgHovered ) );
        ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImGui::GetStyleColorVec4( ImGuiCol_FrameBgActive ) );

#if IMGUI_VERSION_NUM >= 18900
        bool changed = ImGui::InputTextWithHint( "##field", hint, buf, buf_size, flags );
#else
        bool changed = ImGui::InputText( "##field", buf, buf_size, flags );
        // (Optional: draw your own faint hint text when buf is empty)
#endif

        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        bool   focused = ImGui::IsItemActive() || ImGui::IsItemFocused();

        bool cleared = false;
        if (buf[ 0 ] != '\0')
        {
            ImGui::SameLine( 0, 0 );
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10, 6 ) );
            if (ImGui::SmallButton( "×" ))
            {
                buf[ 0 ] = '\0';
                changed = true;
                cleared = true;
            }
            ImGui::PopStyleVar();
        }

        ImGui::PopStyleColor( 3 );
        ImGui::PopStyleVar( 2 );
        ImGui::PopItemWidth();

        ImDrawList *dl = ImGui::GetWindowDrawList();
        static float f = 0.f; 
        float target = focused ? 1.0f : 0.0f;
        f = ImLerp( f, target, ImGui::GetIO().DeltaTime * 12.0f );

        ImU32 border = ImGui::GetColorU32( ImGuiCol_Border );
        ImVec4 grab = ImGui::GetStyleColorVec4( ImGuiCol_SliderGrab );
        ImU32 ul = ImGui::GetColorU32( ImLerp( ImGui::ColorConvertU32ToFloat4( border ), grab, f ) );

        dl->AddLine( ImVec2( min.x + 8, max.y + 2 ), ImVec2( max.x - 8, max.y + 2 ), ul, 2.0f );

        ImGui::PopID();
        return changed || cleared;
    }


    bool CustomCheckbox( const char *label, bool *v, bool mem_write = false ) {
        ImGui::PushID( label );
        float height = ImGui::GetFrameHeight() - 2.5f;
        float width = height * 1.75f;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImGui::InvisibleButton( label, ImVec2( width, height ) );
        bool clicked = ImGui::IsItemClicked();
        if (clicked)
        {
            *v = !*v;
        }
        struct ToggleAnimData
        {
            float t; bool lastValue;
        };
        static std::unordered_map<const char *, ToggleAnimData> animStates;
        ToggleAnimData &anim = animStates[ label ];
        if (anim.lastValue != *v)
        {
            anim.lastValue = *v;
        }
        float targetT = (*v) ? 1.0f : 0.0f;
        float speed = 8.0f;
        float dt = ImGui::GetIO().DeltaTime;
        anim.t = ImLerp( anim.t, targetT, ImClamp( dt * speed, 0.0f, 1.0f ) );
        ImU32 bgColorOn = IM_COL32( 76, 152, 220, 255 );
        ImU32 bgColorOff = IM_COL32( 128, 128, 128, 255 );
        ImU32 knobColor = IM_COL32( 255, 255, 255, 255 );
        ImU32 bgColor = ImGui::ColorConvertFloat4ToU32( ImLerp(
            ImGui::ColorConvertU32ToFloat4( bgColorOff ),
            ImGui::ColorConvertU32ToFloat4( bgColorOn ),
            anim.t ) );
        float rounding = height * 0.5f;
        draw_list->AddRectFilled( p, ImVec2( p.x + width, p.y + height ), bgColor, rounding );
        float knobRadius = (height - 3.5f) * 0.435f;
        float knobX = ImLerp( p.x + knobRadius + 2.0f, p.x + width - knobRadius - 2.0f, anim.t );
        ImVec2 knobCenter( knobX, p.y + height * 0.5f );
        draw_list->AddCircleFilled( knobCenter, knobRadius, knobColor );
        ImGui::SameLine();
        if (mem_write)
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1, 1, 0, 1 ) );
        }
        ImGui::TextUnformatted( label );
        if (mem_write)
            ImGui::PopStyleColor();
        ImGui::PopID();
        return clicked;
    }

    bool CustomSliderFloat( const char *label, ImGuiDataType data_type, float *v,
        float v_min, float v_max, const char *format,
        ImU32 Theme, ImGuiSliderFlags flags,
        float specified_width, const char *labelModifier = "" ) {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext &g = *ImGui::GetCurrentContext();
        const ImGuiStyle &style = g.Style;
        const ImGuiID id = window->GetID( label );
        ImGuiStorage *storage = window->DC.StateStorage;

        const ImGuiID fill_ratio_id = ImHashStr( "fill_anim_ratio", 0, id );
        const ImGuiID anim_t_id = ImHashStr( "anim_t", 0, id );

        SliderAnimationState state;
        state.fill_anim_ratio = storage->GetFloat( fill_ratio_id, (*v - v_min) / (v_max - v_min) );
        state.anim_t = storage->GetFloat( anim_t_id, 0.0f );

        const float w = (specified_width > 0.0f) ? specified_width : ImGui::CalcItemWidth();
        const ImVec2 label_size = ImGui::CalcTextSize( label, nullptr, true );
        const ImVec2 pos = window->DC.CursorPos;
        const float sliderHeight = 6.5f;

        ImRect frame_bb( pos, ImVec2( pos.x + w, pos.y + sliderHeight ) );   // interactive rect
        ImRect total_bb = frame_bb;
        total_bb.Min.y -= (label_size.y + 12.0f);                         // space for label above

        ImGui::ItemSize( total_bb, style.FramePadding.y );
        if (!ImGui::ItemAdd( total_bb, id, &frame_bb ))
            return false;

        const bool hovered = ImGui::ItemHoverable( frame_bb, id, ImGuiItemFlags_None );
        const bool temp_input_active = ImGui::TempInputIsActive( id );

        if (!temp_input_active)
        {
            if (hovered && g.IO.MouseClicked[ ImGuiMouseButton_Left ])
            {
                ImGui::SetActiveID( id, window );
                ImGui::SetFocusID( id, window );
                ImGui::FocusWindow( window );
                g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
            }
            if (g.NavActivateId == id)
            {
                ImGui::SetActiveID( id, window );
                ImGui::SetFocusID( id, window );
                ImGui::FocusWindow( window );
            }
        }

        float ratio = ImSaturate( (*v - v_min) / (v_max - v_min) );
        state.fill_anim_ratio = ImLerp( state.fill_anim_ratio, ratio, 8.0f * g.IO.DeltaTime );

        const bool is_active = (ImGui::GetActiveID() == id);
        if (is_active)
            state.anim_t = ImMin( state.anim_t + g.IO.DeltaTime * 3.0f, 1.0f );
        else
            state.anim_t = ImMax( state.anim_t - g.IO.DeltaTime * 3.0f, 0.0f );

        storage->SetFloat( fill_ratio_id, state.fill_anim_ratio );
        storage->SetFloat( anim_t_id, state.anim_t );

        const float pulse_t = 0.5f * (1.0f + sinf( state.anim_t * IM_PI ));
        const ImU32 base_col_u = ImGui::GetColorU32( ImGuiCol_SliderGrab );
        const ImVec4 base_col = ImGui::ColorConvertU32ToFloat4( base_col_u );
        const ImVec4 final_col = ImLerp( base_col, ImVec4( 1, 1, 1, 1 ), pulse_t );
        const ImU32 final_col_u = ImGui::ColorConvertFloat4ToU32( final_col );

        window->DrawList->AddRectFilled( frame_bb.Min, frame_bb.Max,
            ImGui::GetColorU32( ImGuiCol_FrameBg ), sliderHeight * 0.5f );
        window->DrawList->AddRect( frame_bb.Min, frame_bb.Max,
            ImGui::GetColorU32( ImGuiCol_Border ), sliderHeight * 0.5f );

        ImRect fill_bb = frame_bb;
        fill_bb.Max.x = frame_bb.Min.x + state.fill_anim_ratio * frame_bb.GetWidth();
        window->DrawList->AddRectFilled( fill_bb.Min, fill_bb.Max, Theme, sliderHeight * 0.5f );

        ImRect grab_bb;
        bool value_changed = ImGui::SliderBehavior( frame_bb, id, data_type, v, &v_min, &v_max, "%.10f", flags, &grab_bb );

        *v = IM_FLOOR( *v * 1000.0f + 0.5f ) / 1000.0f;

        ratio = ImSaturate( (*v - v_min) / (v_max - v_min) );
        const ImVec2 grab_center( frame_bb.Min.x + ratio * frame_bb.GetWidth(),
            frame_bb.Min.y + sliderHeight * 0.5f );
        window->DrawList->AddCircleFilled( grab_center, 8.0f, final_col_u, 15 );

        if (label_size.x > 0.0f)
        {
            char value_buf[ 64 ];
            snprintf( value_buf, IM_ARRAYSIZE( value_buf ), format, *v );
            const ImVec2 value_size = ImGui::CalcTextSize( value_buf, nullptr, true );

            const ImVec2 label_pos( frame_bb.Min.x, frame_bb.Min.y - label_size.y - 4.0f );
            const ImVec2 value_pos( label_pos.x + label_size.x + 8.0f, label_pos.y );
            const ImVec2 modifier_pos( value_pos.x + value_size.x + 1.0f, label_pos.y );

            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.78f, 0.78f, 0.78f, 1.0f ) );
            ImGui::RenderText( label_pos, label );
            ImGui::RenderText( value_pos, value_buf );
            ImGui::RenderText( modifier_pos, labelModifier );
            ImGui::PopStyleColor();
        }

        return value_changed;
    }



}
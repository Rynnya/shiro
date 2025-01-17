/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2021-2022 Rynnya
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SHIRO_CATCH_THE_PP_CALCULATOR_HH
#define SHIRO_CATCH_THE_PP_CALCULATOR_HH

#include "../../beatmaps/beatmap.hh"
#include "../../scores/score.hh"
#include "curves.hh"

namespace shiro::pp::ctb {

    class timing_point {
    public:
        timing_point() = default;
        timing_point(double time, double spm, double raw_s, double bpm, double raw_b) :
            timestamp(time),
            spm(spm),
            raw_spm(raw_s),
            bpm(bpm),
            raw_bpm(raw_b)
        {};

        double timestamp = 0;
        double spm = 1;
        double raw_spm = -100;
        double bpm = 100;
        double raw_bpm = 600;
    };

    class slider_tick {
    public:
        slider_tick(double x, double y, double time) :
            x(x),
            y(y),
            time(time)
        {};

        double x;
        double y;
        double time;
    };

    class slider_t {
    public:
        enum class type : uint8_t {
            linear = 0,
            bezier = 1,
            catmull = 2,
            perfect = 3
        };

        slider_t() = default;
        constexpr slider_t(type slider) : value(slider) {};

        constexpr operator type() const { return value; }
        constexpr operator bool() = delete;

        constexpr bool operator==(slider_t a) const { return value == a.value; }
        constexpr bool operator==(type a) const { return value == a; }

        constexpr bool operator!=(slider_t a) const { return value != a.value; }
        constexpr bool operator!=(type a) const { return value != a; }

        constexpr bool is_linear() const { return value == type::linear; }

    private:
        type value;
    };

    class fruit {
    public:
        fruit() = default;
        fruit(double x, double y, double time, int32_t type,
            slider_t slider_type = slider_t::type::linear, std::deque<point> curve_points = {}, int32_t repeat = 1, float pixel_length = 0,
            timing_point time_point = timing_point(), double slider_multiplier = 0.0, float tick_distance = 1.0f
        ) :
            x(x),
            y(y),
            time(time),
            type(type),
            slider_type(slider_type),
            curve_points(curve_points),
            repeat(repeat),
            pixel_length(pixel_length),
            time_point(time_point),
            slider_multiplier(slider_multiplier),
            tick_distance(tick_distance) {

            if (2 & type) {
                this->curve_points.push_front({ x, y });
                this->duration = (time_point.raw_bpm * (pixel_length / (slider_multiplier * time_point.spm)) / 100.0) * repeat;

                this->calculate_slider();
            }
        };

        int32_t get_combo();
        slider_tick to_tick() noexcept;

        // Notes
        double x = 0.0;
        double y = 0.0;
        double time = 0.0;
        int32_t type = 0;

        // Sliders
        slider_t slider_type {};
        std::deque<point> curve_points {};
        int32_t repeat = 1;
        double pixel_length = 0.0;
        timing_point time_point = timing_point();
        double slider_multiplier = 0.0;
        double tick_distance = 1.0;
        double duration = 0.0;

        std::vector<slider_tick> ticks = {};
        std::vector<slider_tick> end_ticks = {};

    private:
        void calculate_slider();
    };

    class difficulty_object {
    public:
        difficulty_object() :
            empty(true),
            object(slider_tick(0, 0, 0))
        {};
        difficulty_object(slider_tick object, double player_width) :
            empty(false),
            object(object),
            player_width(player_width)
        {};

        bool empty;
        double strain = 1.0;
        double offset = 0.0;
        double last_movement = 0.0;
        slider_tick object;
        double error_margin = constants::ABSOLUTE_PLAYER_POSITIONING_ERROR;
        double player_width = 0.0;
        double scaled_position = object.x * (constants::NORMALIZED_HITOBJECT_RADIUS / player_width);
        float hyperdash_distance = 0;
        bool hyperdash = false;

        void calculate_strain(difficulty_object last, float time_rate);

        difficulty_object& operator=(const difficulty_object& other) noexcept = default;
    };

    class ctb_calculator {
    private:
        // Map based variables
        int32_t version = 0;
        double star_rate = 0.0;
        double cs = 0.0;
        double ar = 0.0;
        double od = 0.0;
        double hp = 0.0;
        double slider_multiplier = 0.0;
        double slider_tick_rate = 0.0;
        int32_t max_combo = 0;
        double time_rate = 1.0;
        double player_width = 0.0;

        // Score based variables
        uint32_t mods = 0;
        uint32_t miss_count = 0;
        uint32_t combo = 0;
        float accuracy = 0;

        std::vector<timing_point> timing_points;
        std::vector<fruit> hit_objects;

        std::vector<slider_tick> hit_object_with_ticks;
        std::vector<difficulty_object> difficulty_objects;

        float adjust_difficulty(float raw_value, float scale);
        void calculate_stars();
        void update_hyperdash_distance();
        void calculate_strain_values();
        float calculate_difficulty();
        void parse_file(std::string filename);
        void parse_timing_point(std::string line);
        void parse_hit_object(std::string line);
        timing_point get_point_by_time_all(float timestamp);

    public:
        ctb_calculator(beatmaps::beatmap beatmap, scores::score score);

        float calculate();
    };

}

#endif //SHIRO_CATCH_THE_PP_CALCULATOR_HH

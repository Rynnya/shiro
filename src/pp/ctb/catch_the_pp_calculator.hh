/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
 * Copyright (C) 2021 Rynnya
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
#include "math.hh"

namespace shiro::pp::ctb
{
    class timing_point
    {
    public:
        timing_point() {};
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

    class slider_tick
    {
    public:
        slider_tick(float x, float y, float time) : x(x), y(y), time(time) {};

        float x;
        float y;
        float time;
    };

    class fruit
    {
    public:
        fruit() {};
        fruit(float x, float y, float time, int32_t type,
            std::string slider_type = "", std::deque<std::pair<float, float>> curve_points = {}, int32_t repeat = 1, int32_t pixel_length = 0,
            timing_point time_point = timing_point(), float slider_multiplier = 0, int32_t tick_distance = 1) :
            x(x), y(y), time(time), type(type),
            slider_type(slider_type), curve_points(curve_points), repeat(repeat), pixel_length(pixel_length),
            time_point(time_point), slider_multiplier(slider_multiplier), tick_distance(tick_distance)
        {
            if (2 & type)
            {
                this->curve_points.push_front(std::make_pair(x, y));
                this->duration = (int32_t(time_point.raw_bpm) * (pixel_length / (slider_multiplier * time_point.spm)) / 100) * repeat;

                this->calculate_slider();
            }
        };

        int get_combo();
        inline slider_tick to_tick()
        {
            return slider_tick(this->x, this->y, this->time);
        }

        // Notes
        float x = 0;
        float y = 0;
        float time = 0;
        int32_t type = 0;

        // Sliders
        std::string slider_type = "";
        std::deque<std::pair<float, float>> curve_points = {};
        int32_t repeat = 1;
        float pixel_length = 0;
        timing_point time_point = timing_point();
        float slider_multiplier = 0;
        float tick_distance = 1;
        float duration = 0;

        std::vector<slider_tick> ticks = {};
        std::vector<slider_tick> end_ticks = {};

    private:
        void calculate_slider();
    };

    class difficulty_object
    {
    public:
        difficulty_object() :
            empty(true),
            object(slider_tick(0, 0, 0)),
            player_width(0)
        {};
        difficulty_object(slider_tick object, float player_width) :
            empty(false),
            object(object),
            player_width(player_width)
        {};

        bool empty;
        float strain = 1;
        float offset = 0;
        float last_movement = 0;
        slider_tick object;
        int32_t error_margin = constants::ABSOLUTE_PLAYER_POSITIONING_ERROR;
        float player_width;
        float scaled_position = object.x * (constants::NORMALIZED_HITOBJECT_RADIUS / player_width);
        float hyperdash_distance = 0;
        bool hyperdash = false;

        void calculate_strain(difficulty_object last, float time_rate);

        difficulty_object& operator=(const difficulty_object& other) noexcept
        {
            if (this == &other)
                return *this;

            empty = false;
            strain = other.strain;
            offset = other.offset;
            last_movement = other.last_movement;
            object = other.object;
            player_width = other.player_width;
            scaled_position = other.scaled_position;
            hyperdash_distance = other.hyperdash_distance;
            hyperdash = other.hyperdash;
            return *this;
        };
    };

    class ctb_calculator
    {
    private:
        // Map based variables
        int32_t version = 0;
        float star_rate = 0;
        float cs = 0;
        float ar = 0;
        float od = 0;
        float hp = 0;
        float slider_multiplier = 0;
        float slider_tick_rate = 0;
        int32_t max_combo = 0;
        float time_rate = 1;
        float player_width = 0;

        // Score based variables
        int32_t mods = 0;
        int32_t miss_count = 0;
        int32_t combo = 0;
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
        timing_point get_point_by_time_all(double timestamp);

    public:
        ctb_calculator(beatmaps::beatmap beatmap, scores::score score);
        ~ctb_calculator();

        float calculate();
    };

}

#endif //SHIRO_CATCH_THE_PP_CALCULATOR_HH

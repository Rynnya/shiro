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

#include <regex>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include "../../beatmaps/beatmap_helper.hh"
#include "../../utils/mods.hh"
#include "../../utils/string_utils.hh"
#include "catch_the_pp_calculator.hh"

shiro::pp::ctb::ctb_calculator::ctb_calculator(shiro::beatmaps::beatmap beatmap, shiro::scores::score score) {
    std::optional<std::string> beatmap_file = beatmaps::helper::get_location(beatmap.beatmap_id);

    if (!beatmap_file.has_value()) {
        return;
    }

    this->accuracy = score.accuracy;
    this->miss_count = score.count_misses;
    this->combo = score.max_combo;
    this->mods = score.mods;
    this->cs = adjust_difficulty(beatmap.cs, 1.3f);
    this->ar = adjust_difficulty(beatmap.ar, 1.4f);
    this->od = adjust_difficulty(beatmap.od, 1.4f);
    this->hp = adjust_difficulty(beatmap.hp, 1.4f);
    
    parse_file(*beatmap_file);
    calculate_stars();
}

float shiro::pp::ctb::ctb_calculator::calculate() {
    if (this->star_rate == 0) {
        return 0;
    }

    float pp = std::pow(((5.0 * this->star_rate / 0.0049) - 4.0), 2) / 100000;
    float length_bonus = 0.95 + 0.4 * std::min(1U, this->combo / 3000);

    if (this->combo > 3000) {
        length_bonus += std::log10(combo / 3000) * 0.5;
    }

    pp *= length_bonus;
    pp *= std::pow(0.97, this->miss_count);
    pp *= std::min(std::pow(this->combo, 0.8) / std::pow(this->max_combo, 0.8), 1.0);

    if (this->ar > 9) {
        pp *= 1 + 0.1 * (this->ar - 9.0);
    }

    if (this->ar < 8) {
        pp *= 1 + 0.025 * (8.0 - this->ar);
    }

    if (this->mods & shiro::utils::mods::hidden) {
        pp *= 1.05 + 0.075 * (10 - std::min(10.0, this->ar));
    }

    if (this->mods & shiro::utils::mods::flashlight) {
        pp *= 1.35 * length_bonus;
    }

    pp *= std::pow(this->accuracy, 5.5);

    if (this->mods & shiro::utils::mods::no_fail) {
        pp *= 0.9;
    }

    if (this->mods & shiro::utils::mods::spun_out) {
        pp *= 0.95;
    }

    return pp;
}

float shiro::pp::ctb::ctb_calculator::adjust_difficulty(float raw_value, float scale) {
    if (this->mods & shiro::utils::mods::easy) {
        raw_value = std::max(0.0f, raw_value / 2);
    }

    if (this->mods & shiro::utils::mods::hard_rock) {
        raw_value = std::min(10.0f, raw_value * scale);
    }

    return raw_value;
}

void shiro::pp::ctb::ctb_calculator::calculate_stars() {
    for (fruit& _fruit : this->hit_objects) {
        this->hit_object_with_ticks.push_back(_fruit.to_tick());
        if (2 & _fruit.type) {
            for (slider_tick& tick : _fruit.ticks) {
                this->hit_object_with_ticks.push_back(tick);
            }
            for (slider_tick& tick : _fruit.end_ticks) {
                this->hit_object_with_ticks.push_back(tick);
            }
        }
    }

    if (this->mods & shiro::utils::mods::double_time) {
        this->time_rate += 0.5;
    }

    if (this->mods & shiro::utils::mods::half_time) {
        this->time_rate -= 0.25;
    }

    this->player_width = 305 / 1.6 * ((102.4 * (1 - 0.7 * (this->cs - 5) / 5)) / 128) * 0.7;

    for (slider_tick& tick : this->hit_object_with_ticks) {
        this->difficulty_objects.push_back(difficulty_object{ tick, this->player_width * 0.4 });
    }

    update_hyperdash_distance();
    std::sort(this->difficulty_objects.begin(), this->difficulty_objects.end(),
        [](difficulty_object diff1, difficulty_object diff2) { return diff1.object.time < diff2.object.time; });

    calculate_strain_values();
    this->star_rate = std::sqrt(calculate_difficulty()) * constants::STAR_SCALING_FACTOR;
}

void shiro::pp::ctb::ctb_calculator::update_hyperdash_distance() {
    int32_t last_direction = 0;
    int32_t direction = 0;
    float player_width_half = this->player_width / 2 * 0.8;
    float last = player_width_half;

    difficulty_object current;
    difficulty_object next;

    for (size_t i = 0; i < this->difficulty_objects.size() - 1; i++) {
        current = this->difficulty_objects[i];
        next = this->difficulty_objects[i + 1];

        direction = next.object.x > current.object.x ? 1 : -1;

        float time_to_next = next.object.time - current.object.time - 4.166667;
        float distance_to_next = std::abs(next.object.x - current.object.x);
        distance_to_next -= last_direction == direction ? last : player_width_half;

        if (time_to_next < distance_to_next) {
            current.hyperdash = true;
            last = player_width_half;
        }
        else {
            current.hyperdash_distance = time_to_next - distance_to_next;
            last = std::clamp(current.hyperdash_distance, 0.0f, player_width_half);
        }

        last_direction = direction;
    }
}

void shiro::pp::ctb::ctb_calculator::calculate_strain_values() {
    difficulty_object current = this->difficulty_objects[0];
    difficulty_object next;

    size_t index = 1;
    while (index++ < this->difficulty_objects.size()) {
        next = this->difficulty_objects[index];
        next.calculate_strain(current, this->time_rate);
        current = next;
    }
}

float shiro::pp::ctb::ctb_calculator::calculate_difficulty() {
    float strain_step = constants::STRAIN_STEP * this->time_rate;
    std::vector<float> highest_strains;
    float interval = strain_step;
    float max_strain = 0;

    difficulty_object last = difficulty_object();

    for (difficulty_object& diff_object : this->difficulty_objects) {
        while (diff_object.object.time > interval) {
            highest_strains.push_back(max_strain);
            max_strain = (last.empty)
                ? 0
                : last.strain * std::pow(constants::DECAY_BASE, ((interval - last.object.time) / 1000));

            interval += strain_step;
        }

        if (diff_object.strain > max_strain) {
            max_strain = diff_object.strain;
        }

        last = diff_object;
    }

    float difficulty = 0;
    float weight = 1;

    std::sort(highest_strains.begin(), highest_strains.end(), std::greater<float>());

    for (float& strain : highest_strains) {
        difficulty += weight * strain;
        weight *= constants::DECAY_WEIGHT;
    }

    return difficulty;
}

void shiro::pp::ctb::ctb_calculator::parse_file(std::string filename) {
    std::ifstream file{ filename, std::ifstream::binary };
    std::string line;
    std::string current_sector = "";

    // Parse version
    std::getline(file, line);
    line.erase(std::remove_if(line.begin(), line.end(), [](char c) { return !std::isdigit(c); }));
    this->version = shiro::utils::strings::evaluate(line);

    while (std::getline(file, line)) {
        // Get rid of '\n' symbol
        line.pop_back();

        // Skip empty lines and comments
        if (line == "" || (line[0] == '/' && line[1] == '/')) {
            continue;
        }

        // Read sector
        if (line[0] == '[' && line[line.size() - 1] == ']') {
            line.erase(0, 1);
            line.pop_back();
            current_sector = line;
            continue;
        }

        // We already done with CS, AR, OD and HP, so parse only 2 other things
        if (current_sector == "Difficulty") {
            if (line.find("SliderMultiplier") != std::string::npos) {
                shiro::utils::strings::evaluate(line.erase(0, line.find(":") + 1), this->slider_multiplier);
                continue;
            }

            if (line.find("SliderTickRate") != std::string::npos) {
                shiro::utils::strings::evaluate(line.erase(0, line.find(":") + 1), this->slider_tick_rate);
                continue;
            }
        }

        if (current_sector == "TimingPoints") {
            parse_timing_point(line);
            continue;
        }

        if (current_sector == "HitObjects") {
            parse_hit_object(line);
            continue;
        }
    }
}

void shiro::pp::ctb::ctb_calculator::parse_timing_point(std::string line) {
    using namespace shiro::utils::strings;
    std::vector<std::string> timing_point_split;
    boost::split(timing_point_split, line, boost::is_any_of(","));

    int32_t timing_point_time = 0;
    evaluate(timing_point_split[0], timing_point_time);

    std::string timing_point_focus = timing_point_split[1];

    int32_t timing_point_type = 0;
    if (timing_point_split.size() >= 7) {
        evaluate(timing_point_split[6], timing_point_type);
    }

    if (timing_point_type == 0 && timing_point_focus[0] != '-') {
        timing_point_focus = "-100";
    }

    if (timing_point_focus[0] == '-') {
        float temp = -1;
        evaluate(timing_point_focus, temp);
        this->timing_points.push_back(timing_point{ static_cast<double>(timing_point_time), -100 / temp, temp, 100, 600 });
    }
    else {
        if (this->timing_points.size() == 0) {
            timing_point_time = 0;
        }

        float temp = 1;
        evaluate(timing_point_focus, temp);
        this->timing_points.push_back(timing_point{ static_cast<double>(timing_point_time), 1, -100, 60000 / temp, temp });
    }
}

void shiro::pp::ctb::ctb_calculator::parse_hit_object(std::string line) {
    using namespace shiro::utils::strings;
    std::vector<std::string> split_object;
    boost::split(split_object, line, boost::is_any_of(","));

    float time = evaluate<float>(split_object[2]);
    int32_t object_type = evaluate(split_object[3]);

    if (!((1 & object_type) || (2 & object_type))) {
        return;
    }

    fruit hit_object;
    if (2 & object_type) {
        int32_t repeat = 0;
        float pixel_length = 0;

        timing_point time_point = get_point_by_time_all(time);

        float tick_distance = (100 * this->slider_multiplier) / this->slider_tick_rate;
        if (this->version >= 8) {
            tick_distance /= (std::clamp(-time_point.raw_spm, 10.0, 1000.0) / 100);
        }

        std::vector<std::string> curve_split;
        std::deque<point> curve_points;
        boost::split(curve_split, split_object[5], boost::is_any_of("|"));

        for (size_t i = 0; i < curve_split.size(); i++) {
            std::vector<std::string> vector_split;
            boost::split(vector_split, curve_split[i], boost::is_any_of(":"));
            curve_points.push_back({ evaluate<float>(vector_split[0]), evaluate<float>(vector_split[1]) });
        }

        slider_t slider_type = slider_t::type::linear;

        if (this->version <= 6 && curve_points.size() >= 2) {
            if (curve_split[0] == "L") {
                slider_type = slider_t::type::bezier;
            }

            if (curve_points.size() == 2) {
                if ((evaluate(split_object[0]) == curve_points[0].x && evaluate(split_object[1]) == curve_points[0].y) || curve_points[0] == curve_points[1]) {
                    curve_points.pop_front();
                    slider_type = slider_t::type::linear;
                }
            }
        }

        hit_object = (curve_points.size() == 0)
            ? fruit { evaluate<float>(split_object[0]), evaluate<float>(split_object[1]), time, 1 }
            : fruit { evaluate<float>(split_object[0]), evaluate<float>(split_object[1]), time, object_type, slider_type, curve_points, repeat, pixel_length, time_point, this->slider_multiplier, tick_distance };
    }
    else {
        hit_object = { evaluate<float>(split_object[0]), evaluate<float>(split_object[1]), time, object_type };
    }

    this->hit_objects.push_back(hit_object);
    this->max_combo += hit_object.get_combo();
}

shiro::pp::ctb::timing_point shiro::pp::ctb::ctb_calculator::get_point_by_time_all(float time) {
    for (size_t i = 0; i < this->timing_points.size(); i++) {
        if (this->timing_points[i].timestamp == time) {
            return this->timing_points[i];
        }
    }

    return timing_point();
}

int32_t shiro::pp::ctb::fruit::get_combo() {
    int32_t result = 1;
    if (2 & this->type) {
        result += this->ticks.size();
        result += this->repeat;
    }

    return result;
}

shiro::pp::ctb::slider_tick shiro::pp::ctb::fruit::to_tick() noexcept {
    return { this->x, this->y, this->time };
}

void shiro::pp::ctb::fruit::calculate_slider() {
    using namespace shiro::pp::ctb;
    std::unique_ptr<abstract_curve> curve = nullptr;

    if (slider_type == slider_t::type::perfect && curve_points.size() > 3) {
        slider_type = slider_t::type::bezier;
    }
    else if (curve_points.size() == 2) {
        slider_type = slider_t::type::linear;
    }

    if (slider_type == slider_t::type::perfect) {
        try {
            curve = std::make_unique<perfect>(this->curve_points);
        }
        catch (const std::exception& /* ex */) {
            curve = std::make_unique<bezier>(this->curve_points);
            slider_type = slider_t::type::bezier;
        }
    }
    else if (slider_type == slider_t::type::bezier) {
        curve = std::make_unique<bezier>(this->curve_points);
    }
    else if (slider_type == slider_t::type::catmull) {
        curve = std::make_unique<catmull>(this->curve_points);
    }

    float current_distance = this->tick_distance;
    double time_add = this->duration * (this->tick_distance / (this->pixel_length * this->repeat));

    while (current_distance < this->pixel_length - this->tick_distance / 8) {
        point ptr = (slider_type.is_linear())
            ? math::point_on_line(this->curve_points[0], this->curve_points[1], current_distance)
            : curve->point_at_distance(current_distance);

        this->ticks.push_back({ ptr.x, ptr.y, this->time + time_add * (this->ticks.size() + 1) });
        current_distance += this->tick_distance;
    }

    int32_t repeat_id = 1;
    std::vector<slider_tick> repeat_bonus_ticks;
    while (repeat_id < this->repeat) {
        int32_t dist = (1 & repeat_id) * this->pixel_length;
        float time_offset = (this->duration / this->repeat) * repeat_id;

        point ptr = (slider_type.is_linear())
            ? math::point_on_line(this->curve_points[0], this->curve_points[1], current_distance)
            : curve->point_at_distance(current_distance);

        this->end_ticks.push_back({ ptr.x, ptr.y, this->time + time_offset });

        auto repeat_ticks = this->ticks;
        float normalize_time_value = 0;

        if (1 & repeat_id) {
            std::reverse(repeat_ticks.begin(), repeat_ticks.end());
            normalize_time_value = this->time + (this->duration / this->repeat);
        }
        else {
            normalize_time_value = this->time;
        }

        for (slider_tick& tick : repeat_ticks) {
            tick.time = this->time + time_offset + std::abs(tick.time - normalize_time_value);
        }

        repeat_bonus_ticks.insert(repeat_bonus_ticks.end(), repeat_ticks.begin(), repeat_ticks.end());
        repeat_id += 1;
    }

    this->ticks.insert(this->ticks.end(), repeat_bonus_ticks.begin(), repeat_bonus_ticks.end());

    float dist_end = (1 & this->repeat) * this->pixel_length;
    point ptr = (slider_type.is_linear())
        ? math::point_on_line(this->curve_points[0], this->curve_points[1], current_distance)
        : curve->point_at_distance(current_distance);

    this->end_ticks.push_back({ ptr.x, ptr.y, this->time + this->duration });
}

void shiro::pp::ctb::difficulty_object::calculate_strain(difficulty_object last, float time_rate) {
    float time = (this->object.time - last.object.time) / time_rate;
    float decay = std::pow(constants::DECAY_BASE, time / 1000);

    this->offset = std::clamp(last.scaled_position + last.offset,
        this->scaled_position - (constants::NORMALIZED_HITOBJECT_RADIUS - this->error_margin),
        this->scaled_position + (constants::NORMALIZED_HITOBJECT_RADIUS - this->error_margin)
    ) - this->scaled_position;

    this->last_movement = std::abs(this->scaled_position - last.scaled_position + this->offset - last.offset);
    float addition = std::pow(this->last_movement, 1.3) / 500;
    if (this->scaled_position < last.scaled_position) {
        this->last_movement *= -1;
    }

    float addition_bonus = 0;
    float sqrt_time = std::sqrt(std::max(time, 25.0f));

    if (std::abs(this->last_movement) > 0.1) {
        if (std::abs(last.last_movement) > 0.1 && ctb::math::sign(this->last_movement) != ctb::math::sign(last.last_movement)) {
            float bonus = constants::DIRECTION_CHANGE_BONUS / sqrt_time;
            float bonus_factor = std::min(this->error_margin, std::abs(this->last_movement)) / this->error_margin;

            addition += bonus * bonus_factor;

            if (last.hyperdash_distance <= 10) {
                addition_bonus += 0.3 * bonus_factor;
            }
        }

        addition += 7.5 * std::min(std::abs(this->last_movement), constants::NORMALIZED_HITOBJECT_RADIUS * 2.0) / (constants::NORMALIZED_HITOBJECT_RADIUS * 6.0) / sqrt_time;
    }

    if (last.hyperdash_distance <= 10) {
        if (!last.hyperdash) {
            addition_bonus += 1;
        }
        else {
            this->offset = 0;
        }

        addition *= 1 + addition_bonus * ((10 - last.hyperdash_distance) / 10);
    }

    addition *= 850 / std::max(time, 25.0f);
    this->strain = last.strain * decay + addition;
}

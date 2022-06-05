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

#include "../../utils/string_utils.hh"
#include "../../beatmaps/beatmap_helper.hh"
#include "mania_calculator.hh"

// Based on https://github.com/toxicpie/ompp-web

// Still not equal to official one, but we will use this (at least Yukime)

static std::regex note_regex("(\\d+),\\d+,(\\d+),\\d+,\\d+,(\\d+)");

shiro::pp::mania::mania_calculator::mania_calculator(shiro::beatmaps::beatmap beatmap, shiro::scores::score score) {
    std::optional<std::string> beatmap_file = beatmaps::helper::get_location(beatmap.beatmap_id);

    if (!beatmap_file.has_value()) {
        return;
    }

    this->keys = beatmap.cs;
    this->od = beatmap.od;
    this->score = score.total_score;
    this->mods = score.mods;

    parse_file(*beatmap_file);
    calculate_stars();
}

float shiro::pp::mania::mania_calculator::calculate() {
    if (this->stars == 0) {
        return 0;
    }

    double score_rate = 1;

    adjust_modifier(shiro::utils::mods::easy, score_rate, 0.5);
    adjust_modifier(shiro::utils::mods::no_fail, score_rate, 0.5);
    adjust_modifier(shiro::utils::mods::half_time, score_rate, 0.5);

    double real_score = score / score_rate;
    if (real_score > 1000000) {
        return 0;
    }

    double hit300_window = 34 + 3.0 * (std::min(10.0f, std::max(0.0f, 10 - od)));

    double strain_value = std::pow((5 * std::max(1.0, this->stars / 0.2) - 4), 2.2) / 135 * (1 + 0.1 * std::min(std::vector<shiro::pp::mania::note>::size_type(1), this->notes.size() / 1500));

    if (real_score <= 500000) {
        strain_value = 0;
    }
    else if (real_score <= 600000) {
        strain_value *= ((real_score - 500000) / 100000 * 0.3);
    }
    else if (real_score <= 700000) {
        strain_value *= (0.3 + (real_score - 600000) / 100000 * 0.25);
    }
    else if (real_score <= 800000) {
        strain_value *= (0.55 + (real_score - 700000) / 100000 * 0.20);
    }
    else if (real_score <= 900000) {
        strain_value *= (0.75 + (real_score - 800000) / 100000 * 0.15);
    }
    else {
        strain_value *= (0.9 + (real_score - 900000) / 100000 * 0.1);
    }

    double acc_value = std::max(0.0, 0.2 - ((hit300_window - 34) * 0.006667)) * strain_value * std::pow((std::max(0.0, real_score - 960000) / 40000), 1.1);

    double pp_multiplier = 0.8;
    adjust_modifier(shiro::utils::mods::no_fail, pp_multiplier, 0.9);
    adjust_modifier(shiro::utils::mods::easy, pp_multiplier, 0.5);

    return std::pow(std::pow(strain_value, 1.32) + std::pow(acc_value, 1.1), (1 / 1.1) * pp_multiplier);
}

constexpr void shiro::pp::mania::mania_calculator::adjust_modifier(shiro::utils::mods mod, double& modifier, double multiplier) {
    if (this->mods & mod) {
        modifier *= multiplier;
    }
}

void shiro::pp::mania::mania_calculator::parse_file(std::string filename) {
    std::ifstream file(filename, std::ifstream::binary);
    std::string line;
    std::string current_sector = "";
    while (std::getline(file, line)) {
        // Get rid of '\r' symbol
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

        if (current_sector == "HitObjects") {
            parse_note(line, this->keys);
            continue;
        }
    }
}

void shiro::pp::mania::mania_calculator::parse_note(std::string line, int32_t keys) {
    std::smatch matches;

    if (std::regex_search(line, matches, note_regex)) {
        size_t x = utils::strings::evaluate<size_t>(matches[1]);
        size_t start_t = utils::strings::evaluate<size_t>(matches[2]);
        size_t end_t = -1;
        double key = std::floor(x * keys / 512);

        if (matches[3] != "") {
            end_t = shiro::utils::strings::evaluate<size_t>(matches[3]);
        }

        end_t = (end_t == static_cast<size_t>(-1) ? start_t : end_t);

        this->notes.push_back({ key, start_t, end_t, 0 });
    }
}

void shiro::pp::mania::mania_calculator::calculate_stars() {
    double time_scale = 1;

    adjust_modifier(shiro::utils::mods::double_time, time_scale, 1.5);
    adjust_modifier(shiro::utils::mods::half_time, time_scale, 0.75);

    double strain_step = 400 * time_scale;
    double weight_decay_base = 0.9;
    double individual_decay_base = 0.125;
    double overall_decay_base = 0.3;
    double star_scaling_factor = 0.018;

    size_t notes_size = this->notes.size();
    std::vector<double> held_until {};
    held_until.reserve(notes_size);

    for (size_t i = 0; i < notes_size; i++) {
        held_until[i] = this->notes[i].end_t;
    }

    note previous_note;

    for (note& _note : this->notes) {

        if (!previous_note) {
            previous_note = _note;
            continue;
        }

        double time_elapsed = (_note.start_t - previous_note.start_t) / time_scale / 1000;
        double individual_decay = std::pow(individual_decay_base, time_elapsed);
        double overall_decay = std::pow(overall_decay_base, time_elapsed);
        double hold_factor = 1;
        int32_t hold_addition = 0;

        for (int32_t i = 0; i < this->keys; i++) {
            if (_note.start_t < held_until[i] && _note.end_t > held_until[i]) {
                hold_addition = 1;
            }
            else if (_note.end_t == held_until[i]) {
                hold_addition = 0;
            }
            else if (_note.end_t < held_until[i]) {
                hold_factor = 1.25;
            }

            _note.individual_strain = previous_note.individual_strain * individual_decay;
        }

        held_until[_note.key] = _note.end_t;

        _note.individual_strain += 2 * hold_factor;
        _note.overall_strain = previous_note.overall_strain * overall_decay + (1.0 + hold_addition) * hold_factor;

        previous_note = _note;
    }

    std::vector<double> strain_table;
    double max_strain = 0;
    double interval_end_time = strain_step;
    previous_note = nullptr;

    for (note& _note : this->notes) {
        while (_note.start_t > interval_end_time) {
            strain_table.push_back(max_strain);

            if (!previous_note) {
                interval_end_time = std::ceil(_note.start_t / strain_step) * strain_step;
            }
            else {
                double individual_decay = std::pow(individual_decay_base, ((interval_end_time - previous_note.start_t) / 1000));
                double overall_decay = std::pow(overall_decay_base, ((interval_end_time - previous_note.start_t) / 1000));
                max_strain = previous_note.individual_strain * individual_decay + previous_note.overall_strain * overall_decay;
            }

            interval_end_time += strain_step;
        }

        double strain = _note.individual_strain + _note.overall_strain;
        if (strain > max_strain) {
            max_strain = strain;
        }

        previous_note = _note;
    }

    double difficulty = 0;
    double weight = 1;
    std::sort(strain_table.begin(), strain_table.end(), std::greater<double>());

    for (double value : strain_table) {
        difficulty += value * weight;
        weight *= weight_decay_base;
    }

    this->stars = difficulty * star_scaling_factor;
}

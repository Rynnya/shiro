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

#include <array>

#include "../../../beatmaps/beatmap_helper.hh"
#include "../../../thirdparty/fmt/format.hh"
#include "pass.hh"

 // This value must be added to ID's
constexpr size_t achievement_offset = 32;

namespace keys {

    const std::array<std::string, 4> mode = { "osu", "taiko", "fruits", "mania" };

    const std::array<std::string, 40> names = {
        // osu!std
        "Rising Star",
        "Constellation Prize",
        "Building Confidence",
        "Insanity Approaches",
        "These Clarion Skies",
        "Above and Beyond",
        "Supremacy",
        "Absolution",
        "Event Horizon",
        "Phantasm",
        // osu!taiko
        "My First Don",
        "Katsu Katsu Katsu",
        "Not Even Trying",
        "Face Your Demons",
        "The Demon Within",
        "Drumbreaker",
        "The Godfather",
        "Rhythm Incarnate",
        "Unstoppable Beat",
        "Beat Itself",
        // osu!ctb
        "A Slice Of Life",
        "Dashing Ever Forward",
        "Zesty Disposition",
        "Hyperdash ON!",
        "It's Raining Fruit",
        "Fruit Ninja",
        "Dreamcatcher",
        "Lord of the Catch",
        "Fruit Salad",
        "Insanity",
        // osu!mania
        "First Steps",
        "No Normal Player",
        "Impulse Drive",
        "Hyperspeed",
        "Ever Onwards",
        "Another Surpassed",
        "Extra Credit",
        "Maniac",
        "Abnormal Gravity",
        "Piano"
    };

    const std::array<std::string, 40> descriptions = {
        // osu!std
        "Can't go forward without the first steps.",
        "Definitely not a consolation prize. Now things start getting hard!",
        "Oh, you've SO got this.",
        "You're not twitching, you're just ready.",
        "Everything seems so clear now.",
        "A cut above the rest.",
        "All marvel before your prowess.",
        "My god, you're full of stars!",
        "No force dares to pull you under.",
        "Fevered is your passion, extraordinary is your skill."
        // osu!taiko
        "Marching to the beat of your own drum. Literally.",
        "Hora! Ikuzo!",
        "Muzukashii? Not even.",
        "The first trials are now behind you, but are you a match for the Oni?",
        "No rest for the wicked.",
        "Too strong.",
        "You are the Don of Dons.",
        "Feel the beat. Become the beat.",
        "One with the beat.",
        "The limit of perfection.",
        // osu!ctb
        "Hey, this fruit catching business isn't bad.",
        "Fast is how you do it.",
        "No scurvy for you, not with that much fruit.",
        "Time and distance is no obstacle to you.",
        "And you can catch them all.",
        "Legendary techniques.",
        "No fruit, only dreams now.",
        "Your kingdom kneels before you.",
        "Fruits, fruits, only fruits.",
        "Nothing is real, but fruits.",
        // osu!mania
        "It isn't 9-to-5, but 1-to-9. Keys, that is.",
        "Not anymore, at least.",
        "Not quite hyperspeed, but getting close.",
        "Woah.",
        "Another challenge is just around the corner.",
        "Is there no limit to your skills?",
        "See me after class.",
        "There's just no stopping you.",
        "Did you remember this?",
        "Now you can play on piano!"
    };

    // TODO: If peppy will finally add 9 and 10* achievements for taiko, ctb and mania - replace current ones
}

void shiro::achievements::pass::verify(
    std::vector<std::pair<int64_t, std::string>>& output,
    const std::shared_ptr<shiro::users::user>& user,
    const shiro::beatmaps::beatmap& beatmap,
    const shiro::scores::score& score
) {
    const uint8_t play_mode = beatmap.play_mode;
    if (
        !score.passed ||
        shiro::beatmaps::helper::has_leaderboard(beatmap.ranked_status) || 
        play_mode > 3 || 
        score.play_mode != beatmap.play_mode || 
        user->stats.play_mode != play_mode
    ) {
        return;
    }

    const size_t difficulty = std::floor(std::clamp(get_difficulty(beatmap), 0.0f, 11.0f));

    for (size_t required = 0; required < difficulty; required++) {
        output.emplace_back(
            achievement_offset + (play_mode * 4 + required),
            fmt::format(
                "{}-skill-pass-{}+{}+{}",
                keys::mode[play_mode], required, keys::names[required * play_mode], keys::descriptions[required * play_mode]
            )
        );
    }
}

float shiro::achievements::pass::get_difficulty(const shiro::beatmaps::beatmap& beatmap) {
    switch (beatmap.play_mode) {
        case 0:
        default: {
            return beatmap.difficulty_std;
        }
        case 1: {
            return beatmap.difficulty_taiko;
        }
        case 2: {
            return beatmap.difficulty_ctb;
        }
        case 3: {
            return beatmap.difficulty_mania;
        }
    }
}

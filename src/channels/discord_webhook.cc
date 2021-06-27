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

#include <locale>

#include "../beatmaps/beatmap_helper.hh"
#include "discord_webhook.hh"
#include "../config/bot_file.hh"
#include "../config/discord_webhook_file.hh"
#include "../config/ipc_file.hh"
#include "../scores/score_helper.hh"
#include "../thirdparty/loguru.hh"
#include "../utils/curler.hh"
#include "../utils/play_mode.hh"

void shiro::channels::discord_webhook::init()
{
	if (shiro::config::discord_webhook::url == "")
	{
		LOG_F(WARNING, "Discord Webhook URL is nothing, disabling...");
		shiro::config::discord_webhook::enabled = false;
		return;
	}

	nlohmann::json msg = create_basis();
	msg["embeds"].push_back(create_embed(shiro::config::discord_webhook::name + " is now running!", "", (uint32_t)colors::Blurple));

	if (shiro::utils::curl::post_message(shiro::config::discord_webhook::url, msg))
	{
		LOG_F(INFO, "Successfully connected to Discord Webhook.");
		return;
	}

	LOG_F(WARNING, "Discord Webhook returns error, disabling...");
	shiro::config::discord_webhook::enabled = false;
}

void shiro::channels::discord_webhook::send_message(const std::string& message)
{
	if (!shiro::config::discord_webhook::enabled)
		return;

	nlohmann::json msg = create_basis();
	msg["content"] = message;

	std::thread(shiro::utils::curl::post_message, shiro::config::discord_webhook::url, msg).detach();
}

void shiro::channels::discord_webhook::send_message(const nlohmann::json& message)
{
	if (!shiro::config::discord_webhook::enabled)
		return;

	std::thread(shiro::utils::curl::post_message, shiro::config::discord_webhook::url, message).detach();
}

nlohmann::json shiro::channels::discord_webhook::create_basis()
{
	nlohmann::json msg;

	if (shiro::config::discord_webhook::override_user)
	{
		msg["username"] = shiro::config::discord_webhook::name;
		msg["avatar_url"] = shiro::config::ipc::avatar_url + "1";
	}

	return msg;
}

nlohmann::json shiro::channels::discord_webhook::create_embed(std::string title, std::string description, uint32_t color)
{
	nlohmann::json embed;
	embed["title"] = title;

	if (description != "")
		embed["description"] = description;

	if (color != 0)
		embed["color"] = color;

	return embed;
}

std::string shiro::channels::discord_webhook::get_rank_emote(std::string rank)
{
	static std::unordered_map<std::string, std::string> ranks =
	{
		{ "XH", "<:XH:749286824471429160>" },
		{ "SH", "<:X:749286850509930526>"  },
		{ "X",  "<:SH:749286835682934794>" },
		{ "S",  "<:S:749286860093784165>"  },
		{ "A",  "<:A:749286872663982191>"  },
		{ "B",  "<:B:749286881954627644>"  },
		{ "C",  "<:C:749286890351362113>"  },
		{ "D",  "<:D:749286899105005568>"  }
	};

	auto it = ranks.find(rank);
	return it != ranks.end() ? (*it).second : "<:F:753668674220457984>";
}

void shiro::channels::discord_webhook::send_restrict_message(std::string username, std::string origin_username, std::string reason)
{
	if (!shiro::config::discord_webhook::enabled)
		return;

	nlohmann::json message = create_basis();
	nlohmann::json embed = create_embed(
		"User " + username + " has been restricted",
		"Reason: " + reason,
		(uint32_t)colors::Error
	);
	embed["footer"]["text"] = "Restricted by " + origin_username;
	message["embeds"].push_back(embed);
	send_message(message);
}

void shiro::channels::discord_webhook::send_ban_message(std::string username, std::string origin_username, std::string reason)
{
	if (!shiro::config::discord_webhook::enabled)
		return;

	nlohmann::json message = create_basis();
	nlohmann::json embed = create_embed(
		"User " + username + " has been banned",
		"Reason: " + reason,
		(uint32_t)colors::Error
	);
	embed["footer"]["text"] = "Banned by " + origin_username;
	message["embeds"].push_back(embed);
	send_message(message);
}

void shiro::channels::discord_webhook::send_silence_message(std::string username, std::string origin_username, std::string reason, uint32_t duration)
{
	if (!shiro::config::discord_webhook::enabled)
		return;

	nlohmann::json message = create_basis();
	nlohmann::json embed = create_embed(
		"User " + username + " has been silenced for " + std::to_string(duration) + " seconds",
		"Reason: " + reason,
		(uint32_t)colors::Error
	);
	embed["footer"]["text"] = "Silenced by " + origin_username;
	message["embeds"].push_back(embed);
	send_message(message);
}

void shiro::channels::discord_webhook::send_top1_message(std::shared_ptr<shiro::users::user> user, shiro::beatmaps::beatmap beatmap, scores::score score)
{
	if (!shiro::config::discord_webhook::enabled)
		return;

	nlohmann::json message = create_basis();
	char buffer[128];
	char buffer_title[256];
	char buffer_stats[1024];

	utils::play_mode mode = (utils::play_mode)score.play_mode;
	std::snprintf(buffer, sizeof(buffer), "x%d/%d | [%d/%d/%d/%d]", 
		score.max_combo, beatmap.max_combo, score.count_300, score.count_100, score.count_50, score.count_misses);

	std::snprintf(
		buffer_stats, sizeof(buffer_stats),
		u8"\u25B8 Achieved by [**__%s__**](%s) | Map by %s\n" \
		u8"\u25B8 **%.2f** :star: | **%d bpm** | **%d:%02d**\n" \
		u8"\u25B8 %s | **%s**\n" \
		u8"\u25B8 %s\n" \
		u8"\u25B8 %s | **%.2f%%** | **%.2fpp**",
		user->presence.username.c_str(), user->get_url().c_str(), beatmap.creator.c_str(), 
		beatmaps::helper::score_to_difficulty(beatmap, mode), beatmap.bpm, beatmap.hit_length / 60, beatmap.hit_length % 60, 
		beatmaps::helper::build_difficulty_header(beatmap, mode).c_str(), scores::helper::build_mods_list(score.mods).c_str(),
		buffer,
		get_rank_emote(score.rank).c_str(), score.accuracy, score.pp
	);
	std::snprintf(
		buffer_title, sizeof(buffer_title),
		u8"New #1 on %s (%s) in %s 🎉",
		beatmap.title.c_str(), beatmap.difficulty_name.c_str(), utils::play_mode_to_string(mode).c_str()
	);

	nlohmann::json embed;
	embed["author"]["name"] = buffer_title;
	embed["author"]["url"] = beatmap.get_url();
	embed["author"]["icon_url"] = user->get_avatar_url();
	embed["description"] = buffer_stats;
	embed["color"] = (uint32_t)colors::Ellysa;

	message["embeds"].push_back(embed);

	std::ignore = shiro::utils::curl::post_message(shiro::config::discord_webhook::url, message);
}

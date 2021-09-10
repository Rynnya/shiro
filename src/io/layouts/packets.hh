/*
 * shiro - High performance, high quality osu!Bancho C++ re-implementation
 * Copyright (C) 2018-2020 Marc3842h, czapek
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

#ifndef SHIRO_PACKETS_HH
#define SHIRO_PACKETS_HH

#include <cstdint>

namespace shiro::io {

    [[maybe_unused]]
    static const int cho_protocol = 19;

    enum class packet_id : uint16_t {
        in_send_user_status = 0,
        in_send_irc_message = 1,
        in_exit = 2,
        in_request_status_update = 3,
        in_pong = 4,
        out_login_reply = 5,
        out_command_error = 6,
        out_send_message = 7,
        out_ping = 8,
        out_handle_irc_change_username = 9,
        out_handle_irc_quit = 10,
        out_handle_osu_update = 11,
        out_handle_user_quit = 12,
        out_spectator_joined = 13,
        out_spectator_left = 14,
        out_spectate_frames = 15,
        in_start_spectating = 16,
        in_stop_spectating = 17,
        in_spectate_frames = 18,
        out_version_update = 19,
        in_error_report = 20,
        in_cant_spectate = 21,
        out_spectator_cant_spectate = 22,
        out_get_attention = 23,
        out_announce = 24,
        in_send_irc_message_private = 25,
        out_match_update = 26,
        out_match_new = 27,
        out_match_disband = 28,
        in_lobby_part = 29,
        in_lobby_join = 30,
        in_match_create = 31,
        in_match_join = 32,
        in_match_part = 33,
        out_match_join_success = 36,
        out_match_join_fail = 37,
        in_match_change_slot = 38,
        in_match_ready = 39,
        in_match_lock = 40,
        in_match_change_settings = 41,
        out_fellow_spectator_joined = 42,
        out_fellow_spectator_left = 43,
        in_match_start = 44,
        out_match_start = 46,
        in_match_score_update = 47,
        out_match_score_update = 48,
        in_match_complete = 49,
        out_match_transfer_host = 50,
        in_match_change_mods = 51,
        in_match_load_complete = 52,
        out_match_all_players_loaded = 53,
        in_match_no_beatmap = 54,
        in_match_not_ready = 55,
        in_match_failed = 56,
        out_match_player_failed = 57,
        out_match_complete = 58,
        in_match_has_beatmap = 59,
        in_match_skip_request = 60,
        out_match_skip = 61,
        out_unauthorised = 62,
        in_channel_join = 63,
        out_channel_join_success = 64,
        out_channel_available = 65,
        out_channel_revoked = 66,
        out_channel_available_auto_join = 67,
        in_beatmap_info_request = 68,
        out_beatmap_info_reply = 69,
        in_match_transfer_host = 70,
        out_login_permissions = 71,
        out_friends_list = 72,
        in_friend_add = 73,
        in_friend_remove = 74,
        out_protocol_negotiation = 75,
        out_title_update = 76,
        in_match_change_team = 77,
        in_channel_leave = 78,
        in_receive_updates = 79,
        out_monitor = 80,
        out_match_player_skipped = 81,
        in_set_irc_away_message = 82,
        out_user_presence = 83,
        in_user_stats_request = 85,
        out_restart = 86,
        in_invite = 87,
        out_invite = 88,
        out_channel_listing_complete = 89,
        in_match_change_password = 90,
        out_match_change_password = 91,
        out_ban_info = 92,
        in_special_match_info_request = 93,
        out_user_silenced = 94,
        out_user_presence_single = 95,
        out_user_presence_bundle = 96,
        in_user_presence_request = 97,
        in_user_presence_request_all = 98,
        in_user_toggle_block_non_friend_pm = 99,
        out_user_pm_blocked = 100,
        out_target_is_silenced = 101,
        out_version_update_forced = 102,
        out_switch_server = 103,
        out_account_restricted = 104,
        out_rtx = 105,
        out_match_abort = 106,
        out_switch_tourney_server = 107,
        in_special_join_match_channel = 108,
        in_special_leave_match_channel = 109
    };

}

#endif //SHIRO_PACKETS_HH

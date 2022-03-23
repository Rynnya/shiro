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

#include "../handlers/multiplayer/lobby/lobby_leave_handler.hh"
#include "../handlers/multiplayer/lobby/lobby_join_handler.hh"
#include "../handlers/multiplayer/match/match_complete_handler.hh"
#include "../handlers/multiplayer/match/match_load_handler.hh"
#include "../handlers/multiplayer/match/match_player_failed_handler.hh"
#include "../handlers/multiplayer/match/match_score_update_handler.hh"
#include "../handlers/multiplayer/match/match_skip_request_handler.hh"
#include "../handlers/multiplayer/match/match_start_handler.hh"
#include "../handlers/multiplayer/room/room_beatmap_handler.hh"
#include "../handlers/multiplayer/room/room_change_host_handler.hh"
#include "../handlers/multiplayer/room/room_change_mods_handler.hh"
#include "../handlers/multiplayer/room/room_change_password_handler.hh"
#include "../handlers/multiplayer/room/room_change_settings_handler.hh"
#include "../handlers/multiplayer/room/room_change_slot_handler.hh"
#include "../handlers/multiplayer/room/room_change_team_handler.hh"
#include "../handlers/multiplayer/room/room_create_handler.hh"
#include "../handlers/multiplayer/room/room_join_handler.hh"
#include "../handlers/multiplayer/room/room_leave_handler.hh"
#include "../handlers/multiplayer/room/room_lock_slot_handler.hh"
#include "../handlers/multiplayer/room/room_ready_handler.hh"
#include "../handlers/chat/leave_channel_handler.hh"
#include "../handlers/chat/join_channel_handler.hh"
#include "../handlers/chat/private_chat_handler.hh"
#include "../handlers/chat/public_chat_handler.hh"
#include "../handlers/friends/friend_add_handler.hh"
#include "../handlers/friends/friend_remove_handler.hh"
#include "../handlers/presence/user_presence_request_all_handler.hh"
#include "../handlers/presence/user_presence_request_handler.hh"
#include "../handlers/presence/user_stats_request_handler.hh"
#include "../handlers/spectating/start_spectating_handler.hh"
#include "../handlers/spectating/stop_spectating_handler.hh"
#include "../handlers/spectating/spectator_frames_handler.hh"
#include "../handlers/spectating/cant_spectate_handler.hh"
#include "../handlers/invite_handler.hh"
#include "../handlers/logout_handler.hh"
#include "../handlers/ping_handler.hh"
#include "../handlers/request_status_update_handler.hh"
#include "../handlers/user_status_handler.hh"
#include "../thirdparty/naga.hh"
#include "../utils/time_utils.hh"
#include "packet_router.hh"

void shiro::routes::route(shiro::io::packet_id packet_id, shiro::io::osu_packet &in, shiro::io::osu_writer &out, std::shared_ptr<users::user> user) {
    switch (packet_id) {
        case io::packet_id::in_send_user_status: {
            handler::user_status::handle(in, out, user);
            break;
        }
        case io::packet_id::in_send_irc_message: {
            handler::chat::handle_public(in, out, user);
            break;
        }
        case io::packet_id::in_exit: {
            handler::logout::handle(in, out, user);
            break;
        }
        case io::packet_id::in_request_status_update: {
            handler::request_status_update::handle(in, out, user);
            break;
        }
        case io::packet_id::in_pong: {
            handler::ping::handle(in, out, user);
            break;
        }
        case io::packet_id::in_start_spectating: {
            handler::spectating::start::handle(in, out, user);
            break;
        }
        case io::packet_id::in_stop_spectating: {
            handler::spectating::stop::handle(in, out, user);
            break;
        }
        case io::packet_id::in_spectate_frames: {
            handler::spectating::frames::handle(in, out, user);
            break;
        }
        case io::packet_id::in_error_report: {
            // Shiro don't support this, because osu! client sends cfg file with this request
            // This also sends crash log, but its fully obfustated, so useless
            break;
        }
        case io::packet_id::in_cant_spectate: {
            handler::spectating::cant_spectate::handle(in, out, user);
            break;
        }
        case io::packet_id::in_send_irc_message_private: {
            handler::chat::handle_private(in, out, user);
            break;
        }
        case io::packet_id::in_lobby_part: {
            handler::multiplayer::lobby::leave::handle(in, out, user);
            break;
        }
        case io::packet_id::in_lobby_join: {
            handler::multiplayer::lobby::join::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_create: {
            handler::multiplayer::room::create::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_join: {
            handler::multiplayer::room::join::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_part: {
            handler::multiplayer::room::leave::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_change_slot: {
            handler::multiplayer::room::change_slot::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_ready: {
            handler::multiplayer::room::ready::handle_ready(in, out, user);
            break;
        }
        case io::packet_id::in_match_lock: {
            handler::multiplayer::room::lock_slot::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_change_settings: {
            handler::multiplayer::room::change_settings::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_start: {
            handler::multiplayer::match::start::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_score_update: {
            handler::multiplayer::match::score_update::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_complete: {
            handler::multiplayer::match::complete::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_change_mods: {
            handler::multiplayer::room::change_mods::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_load_complete: {
            handler::multiplayer::match::load::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_no_beatmap: {
            handler::multiplayer::room::beatmap::handle_no_beatmap(in, out, user);
            break;
        }
        case io::packet_id::in_match_not_ready: {
            handler::multiplayer::room::ready::handle_unready(in, out, user);
            break;
        }
        case io::packet_id::in_match_failed: {
            // TODO: Team should lose when everyone in party failed
            handler::multiplayer::match::player_failed::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_has_beatmap: {
            handler::multiplayer::room::beatmap::handle_has_beatmap(in, out, user);
            break;
        }
        case io::packet_id::in_match_skip_request: {
            handler::multiplayer::match::skip_request::handle(in, out, user);
            break;
        }
        case io::packet_id::in_channel_join: {
            handler::chat::join::handle(in, out, user);
            break;
        }
        case io::packet_id::in_beatmap_info_request: {
            // Not used in newer clients
            break;
        }
        case io::packet_id::in_match_transfer_host: {
            handler::multiplayer::room::change_host::handle(in, out, user);
            break;
        }
        case io::packet_id::in_friend_add: {
            handler::friends::add::handle(in, out, user);
            break;
        }
        case io::packet_id::in_friend_remove: {
            handler::friends::remove::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_change_team: {
            handler::multiplayer::room::change_team::handle(in, out, user);
            break;
        }
        case io::packet_id::in_channel_leave: {
            handler::chat::leave::handle(in, out, user);
            break;
        }
        case io::packet_id::in_receive_updates: {
            // Presence updates are not used now?
            break;
        }
        case io::packet_id::in_set_irc_away_message: {
            // https://github.com/osuripple/pep.py/blob/master/events/setAwayMessageEvent.py
            // https://github.com/osuripple/pep.py/blob/master/constants/clientPackets.py#L43
            break;
        }
        case io::packet_id::in_user_stats_request: {
            handler::stats::request_all::handle(in, out, user);
            break;
        }
        case io::packet_id::in_invite: {
            handler::invite::handle(in, out, user);
            break;
        }
        case io::packet_id::in_match_change_password: {
            handler::multiplayer::room::change_password::handle(in, out, user);
            break;
        }
        case io::packet_id::in_special_match_info_request: {
            // https://github.com/osuripple/pep.py/blob/master/events/tournamentMatchInfoRequestEvent.py
            // https://github.com/osuripple/pep.py/blob/master/constants/clientPackets.py#L166
            break;
        }
        case io::packet_id::in_user_presence_request: {
            handler::presence::request::handle(in, out, user);
            break;
        }
        case io::packet_id::in_user_presence_request_all: {
            handler::presence::request_all::handle(in, out, user);
            break;
        }
        case io::packet_id::in_user_toggle_block_non_friend_pm: {
            // ???
            break;
        }
        case io::packet_id::in_special_join_match_channel: {
            // https://github.com/osuripple/pep.py/blob/master/events/tournamentJoinMatchChannelEvent.py
            // https://github.com/osuripple/pep.py/blob/master/constants/clientPackets.py#L169
            break;
        }
        case io::packet_id::in_special_leave_match_channel: {
            // https://github.com/osuripple/pep.py/blob/master/events/tournamentLeaveMatchChannelEvent.py
            // https://github.com/osuripple/pep.py/blob/master/constants/clientPackets.py#L172
            break;
        }
        default: {
            LOG_F(WARNING, "Packet {} was sent for incoming packet handling while being outbound.", static_cast<uint16_t>(packet_id));
            return;
        }
    }

    user->last_ping = utils::time::current_time();
}

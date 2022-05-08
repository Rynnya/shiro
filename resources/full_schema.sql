-- MariaDB dump 10.19  Distrib 10.5.15-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: yukime
-- ------------------------------------------------------
-- Server version	10.5.15-MariaDB-0+deb11u1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `yukime`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `yukime` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;

USE `yukime`;

-- TODO: Implement 2fa system
DROP TABLE IF EXISTS `2fa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `2fa` (
  `user_id` int(11) NOT NULL,
  `ip` varchar(39) NOT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `2fa_totp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `2fa_totp` (
  `enabled` tinyint(1) NOT NULL DEFAULT 0,
  `user_id` int(11) NOT NULL,
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement achievements
DROP TABLE IF EXISTS `achievements`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `achievements` (
  `id` int(11) NOT NULL,
  `name` mediumtext NOT NULL,
  `description` mediumtext NOT NULL,
  `icon` mediumtext NOT NULL,
  `version` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement anti-cheat system, i'm mean, FULL ANTICHEAT SYSTEM, not something that we have rn
DROP TABLE IF EXISTS `anticheat_reports`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `anticheat_reports` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `anticheat_id` int(11) NOT NULL,
  `score_id` int(11) NOT NULL,
  `severity` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Is we really will log every single user message?
-- I'm don't like the idea of logging messages, especially private
DROP TABLE IF EXISTS `bancho_messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_messages` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `msg_from_user_id` int(11) NOT NULL,
  `msg_from_username` varchar(32) NOT NULL,
  `msg_to` varchar(32) NOT NULL,
  `msg` varchar(127) NOT NULL,
  `time` bigint(21) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Look previous table
DROP TABLE IF EXISTS `bancho_private_messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_private_messages` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `msg_from_user_id` int(11) NOT NULL,
  `msg_from_username` varchar(32) NOT NULL,
  `msg_to` varchar(32) NOT NULL,
  `msg` varchar(127) NOT NULL,
  `time` bigint(21) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Currently most of parameters can be adjusted by using config files
-- I'm don't think that i'm will change something in this struct
-- So in future i'm should implement config reloading, so i'm can delete this table
DROP TABLE IF EXISTS `bancho_settings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_settings` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `value_int` int(11) NOT NULL DEFAULT 0,
  `value_string` varchar(512) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Is this ever used anywhere?
-- I'm mean, bancho tokens are still valid only until user logout or leave a game
-- I'm might need to check this on official server, and if osu! saves old tokens - then I'm should implement this part too
DROP TABLE IF EXISTS `bancho_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_tokens` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `token` varchar(16) NOT NULL,
  `osu_id` int(11) NOT NULL,
  `latest_message_id` int(11) NOT NULL,
  `latest_private_message_id` int(11) NOT NULL,
  `latest_packet_time` int(11) NOT NULL,
  `latest_heavy_packet_time` int(11) NOT NULL,
  `joined_channels` varchar(512) NOT NULL,
  `game_mode` tinyint(4) NOT NULL,
  `action` int(11) NOT NULL,
  `action_text` varchar(128) NOT NULL,
  `kicked` tinyint(4) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;


DROP TABLE IF EXISTS `beatmaps`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `beatmaps` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `beatmap_id` int(11) NOT NULL DEFAULT 0,
  `beatmapset_id` int(11) NOT NULL DEFAULT 0,
  `beatmap_md5` varchar(32) NOT NULL DEFAULT '',
  `artist` text NOT NULL,
  `title` text NOT NULL,
  `difficulty_name` text NOT NULL,
  `creator` text NOT NULL,
  `cs` float DEFAULT NULL,
  `ar` float NOT NULL DEFAULT 0,
  `od` float NOT NULL DEFAULT 0,
  `hp` float DEFAULT NULL,
  `mode` int(11) NOT NULL DEFAULT 0,
  `rating` int(11) NOT NULL DEFAULT 10,
  `difficulty_std` float NOT NULL DEFAULT 0,
  `difficulty_taiko` float NOT NULL DEFAULT 0,
  `difficulty_ctb` float NOT NULL DEFAULT 0,
  `difficulty_mania` float NOT NULL DEFAULT 0,
  `max_combo` int(11) NOT NULL DEFAULT 0,
  `hit_length` int(11) NOT NULL DEFAULT 0,
  `bpm` bigint(21) NOT NULL DEFAULT 0,
  `count_normal` int(11) NOT NULL DEFAULT 0,
  `count_slider` int(11) NOT NULL DEFAULT 0,
  `count_spinner` int(11) NOT NULL DEFAULT 0,
  `play_count` int(11) NOT NULL DEFAULT 0,
  `pass_count` int(11) NOT NULL DEFAULT 0,
  `ranked_status` tinyint(4) NOT NULL DEFAULT 0,
  `latest_update` bigint(21) NOT NULL DEFAULT 0,
  `ranked_status_freezed` tinyint(4) NOT NULL DEFAULT 0,
  `creating_date` bigint(21) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=9051 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `beatmaps_names`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `beatmaps_names` (
  `id` int(11)  NOT NULL,
  `name` text NOT NULL,
  UNIQUE KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement beatmap rating system
DROP TABLE IF EXISTS `beatmaps_rating`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `beatmaps_rating` (
  `id` int(11)  NOT NULL AUTO_INCREMENT,
  `user_id` int(11)  NOT NULL,
  `beatmap_md5` varchar(32) NOT NULL,
  `rating` int(11)  NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `channels`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `channels` (
  `id` int(11)  NOT NULL AUTO_INCREMENT,
  `name` varchar(32) COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `auto_join` tinyint(1) NOT NULL,
  `hidden` tinyint(1) NOT NULL,
  `read_only` tinyint(1) NOT NULL,
  `permission` bigint(21) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

LOCK TABLES `channels` WRITE;
/*!40000 ALTER TABLE `channels` DISABLE KEYS */;
INSERT INTO `channels` VALUES (4,'#announce','',1,0,1,0),(5,'#lobby','',0,1,0,0),(6,'#console','',1,0,1,2147483648);
/*!40000 ALTER TABLE `channels` ENABLE KEYS */;
UNLOCK TABLES;

-- TODO: Implement clans
DROP TABLE IF EXISTS `clans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clans` (
  `id` int(11)  NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `description` text NOT NULL,
  `icon` text NOT NULL,
  `tag` varchar(6) NOT NULL,
  `mlimit` int(11) NOT NULL DEFAULT 16,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `clans_invites`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clans_invites` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `clan` int(11)  NOT NULL,
  `invite` varchar(8) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Where this was used?
-- Might be some old ripple code database, but need to check
DROP TABLE IF EXISTS `comments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `comments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `beatmap_id` int(11) NOT NULL DEFAULT 0,
  `beatmapset_id` int(11) NOT NULL DEFAULT 0,
  `score_id` int(11) NOT NULL DEFAULT 0,
  `mode` int(11) NOT NULL,
  `comment` varchar(128) NOT NULL,
  `time` int(11) NOT NULL,
  `who` varchar(11) NOT NULL,
  `special_format` varchar(2556) DEFAULT 'FFFFFF',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: I'm don't think that this is ever needed
-- Currently new doc's system will depends on templates and folders, so I'm might remove this in future
DROP TABLE IF EXISTS `docs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `docs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `doc_name` varchar(255) NOT NULL DEFAULT 'New Documentation File',
  `doc_contents` longtext NOT NULL,
  `public` tinyint(1) NOT NULL DEFAULT 0,
  `old_name` varchar(200) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Must recheck if we ever save user hardware data for anticheat
DROP TABLE IF EXISTS `hw_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hw_user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `mac` varchar(32) NOT NULL,
  `unique_id` varchar(32) NOT NULL,
  `disk_id` varchar(32) NOT NULL,
  `occurencies` int(11) NOT NULL DEFAULT 0,
  `activated` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: What is this for?
DROP TABLE IF EXISTS `identity_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `identity_tokens` (
  `user_id` int(11) NOT NULL,
  `token` varchar(64) NOT NULL,
  UNIQUE KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Add timestamps and catch every single ip change?
-- This might be useful to recover hacked users from ban, but we also must verify hardware then
-- I'm think this will be very useful for admin panel
DROP TABLE IF EXISTS `ip_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ip_user` (
  `user_id` int(11) NOT NULL,
  `ip` varchar(39) NOT NULL,
  `occurencies` int(11) NOT NULL,
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement IRC system
DROP TABLE IF EXISTS `irc_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `irc_tokens` (
  `user_id` int(11) NOT NULL DEFAULT 0,
  `token` varchar(32) NOT NULL DEFAULT '',
  UNIQUE KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Find something to use it and fill up
DROP TABLE IF EXISTS `main_menu_icons`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `main_menu_icons` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `is_current` int(11) NOT NULL,
  `file_id` varchar(128) NOT NULL,
  `name` varchar(256) NOT NULL,
  `url` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: What is this?
DROP TABLE IF EXISTS `osin_access`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `osin_access` (
  `scope` int(11) NOT NULL DEFAULT 0,
  `created_at` int(11) NOT NULL DEFAULT 0,
  `client` int(11) NOT NULL DEFAULT 0,
  `extra` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Look previous table
DROP TABLE IF EXISTS `osin_client`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `osin_client` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `secret` varchar(64) NOT NULL DEFAULT '',
  `extra` varchar(127) NOT NULL DEFAULT '',
  `redirect_uri` varchar(127) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question: Look previous table
DROP TABLE IF EXISTS `osin_client_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `osin_client_user` (
  `client_id` int(11) NOT NULL DEFAULT 0,
  `user` int(11) NOT NULL DEFAULT 0
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement password recovery system
DROP TABLE IF EXISTS `password_recovery`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `password_recovery` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `secret_key` varchar(80) NOT NULL,
  `user_id` int(11) NOT NULL,
  `time` timestamp NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `punishments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `punishments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `origin_id` int(11) NOT NULL,
  `type` tinyint(3) NOT NULL,
  `time` bigint(21) NOT NULL,
  `duration` int(11) DEFAULT NULL,
  `active` tinyint(1) NOT NULL DEFAULT 1,
  `reason` varchar(128) COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement ranking requests
DROP TABLE IF EXISTS `rank_requests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rank_requests` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `beatmap_id` int(11) NOT NULL,
  `type` varchar(8) NOT NULL,
  `time` int(11) NOT NULL,
  `blacklisted` tinyint(4) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `beatmap_id` (`beatmap_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement better admin ranking system, might be through admin panel?
DROP TABLE IF EXISTS `ranking_logs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ranking_logs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `map_id` int(11) DEFAULT NULL,
  `modified_by` int(11) DEFAULT NULL,
  `ranked` tinyint(4) DEFAULT NULL,
  `map_type` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `relationships`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `relationships` (
  `origin` int(11) NOT NULL,
  `target` int(11) NOT NULL,
  `blocked` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Handle reports
DROP TABLE IF EXISTS `reports`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `reports` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `from_uid` int(11) NOT NULL,
  `to_uid` int(11) NOT NULL,
  `reason` text NOT NULL,
  `chatlog` text NOT NULL,
  `time` int(11) NOT NULL,
  `assigned` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `scores`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `scores` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `hash` varchar(35) NOT NULL,
  `beatmap_md5` varchar(35) NOT NULL DEFAULT '',
  `user_id` int(11) NOT NULL,
  `ranking` varchar(2) NOT NULL,
  `score` bigint(21) NOT NULL DEFAULT 0,
  `max_combo` int(11) NOT NULL DEFAULT 0,
  `full_combo` tinyint(1) NOT NULL DEFAULT 0,
  `mods` bigint(21) NOT NULL DEFAULT 0,
  `count_300` int(11) NOT NULL,
  `count_100` int(11) NOT NULL,
  `count_50` int(11) NOT NULL,
  `count_katus` int(11) NOT NULL,
  `count_gekis` int(11) NOT NULL,
  `count_misses` int(11) NOT NULL,
  `time` bigint(21) NOT NULL,
  `play_mode` tinyint(4) NOT NULL DEFAULT 0,
  `completed` tinyint(4) NOT NULL DEFAULT 3,
  `accuracy` float NOT NULL DEFAULT 0,
  `pp` double NOT NULL DEFAULT 0,
  `play_time` int(11) NOT NULL DEFAULT 0,
  `is_relax` tinyint(1) NOT NULL DEFAULT 0,
  `times_watched` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `scores_first`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `scores_first` (
  `score_id` int(11) NOT NULL,
  `user_id` int(11) NOT NULL,
  `beatmap_md5` varchar(35) COLLATE utf8mb4_unicode_ci NOT NULL,
  `play_mode` tinyint(2) NOT NULL,
  `is_relax` tinyint(1) NOT NULL DEFAULT 0,
  UNIQUE KEY `score_id` (`score_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tokens` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `permissions` bigint(21) NOT NULL,
  `token` varchar(129) NOT NULL,
  `private` tinyint(1) DEFAULT 0,
  `last_updated` bigint(21) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- Question/TODO: I'm might need to talk to my frontend
DROP TABLE IF EXISTS `user_badges`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_badges` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `badge` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement clan system
DROP TABLE IF EXISTS `user_clans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_clans` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `clan_id` int(11) NOT NULL,
  `permissions` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `safe_username` varchar(32) NOT NULL,
  `country` varchar(2) NOT NULL DEFAULT 'XX',
  `password_hash` varchar(129) NOT NULL,
  `salt` varchar(64) NOT NULL,
  `email` varchar(256) NOT NULL,
  `ip` varchar(39) NOT NULL,
  `registration_date` bigint(21) NOT NULL,
  `latest_activity` bigint(21) NOT NULL DEFAULT 0,
  `followers` bigint(21) NOT NULL DEFAULT 0,
  `permissions` bigint(21) NOT NULL DEFAULT 0,
  `userpage` text DEFAULT NULL,
  `background` text DEFAULT NULL,
  `status` text DEFAULT NULL,
  `notes` text DEFAULT NULL,
  `last_session` text NOT NULL,
  `is_public` tinyint(1) NOT NULL DEFAULT 1,
  `is_relax` tinyint(1) NOT NULL DEFAULT 0,
  `favourite_mode` tinyint(2) NOT NULL DEFAULT 0,
  `favourite_relax` tinyint(1) NOT NULL DEFAULT 0,
  `play_style` tinyint(4) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

-- TODO: Implement
DROP TABLE IF EXISTS `users_beatmap_playcount`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_beatmap_playcount` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `beatmap_id` int(11) DEFAULT NULL,
  `game_mode` int(11) DEFAULT NULL,
  `play_count` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `play_count_index` (`user_id`,`beatmap_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `users_preferences`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_preferences` (
  `id` int(11) NOT NULL,
  `scoreboard_display_classic` tinyint(1) DEFAULT 0,
  `scoreboard_display_relax` tinyint(1) DEFAULT 0,
  `auto_last_classic` tinyint(2) DEFAULT 0,
  `auto_last_relax` tinyint(2) DEFAULT 0,
  `score_overwrite_std` tinyint(1) DEFAULT 0,
  `score_overwrite_taiko` tinyint(1) DEFAULT 0,
  `score_overwrite_ctb` tinyint(1) DEFAULT 0,
  `score_overwrite_mania` tinyint(1) DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `users_stats`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_stats` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `rank_std` int(11) NOT NULL DEFAULT 0,
  `rank_taiko` int(11) NOT NULL DEFAULT 0,
  `rank_ctb` int(11) NOT NULL DEFAULT 0,
  `rank_mania` int(11) NOT NULL DEFAULT 0,
  `ranked_score_std` bigint(21) NOT NULL DEFAULT 0,
  `ranked_score_taiko` bigint(21) NOT NULL DEFAULT 0,
  `ranked_score_ctb` bigint(21) NOT NULL DEFAULT 0,
  `ranked_score_mania` bigint(21) NOT NULL DEFAULT 0,
  `total_score_std` bigint(21) NOT NULL DEFAULT 0,
  `total_score_taiko` bigint(21) NOT NULL DEFAULT 0,
  `total_score_ctb` bigint(21) NOT NULL DEFAULT 0,
  `total_score_mania` bigint(21) NOT NULL DEFAULT 0,
  `play_count_std` int(11) NOT NULL DEFAULT 0,
  `play_count_taiko` int(11) NOT NULL DEFAULT 0,
  `play_count_ctb` int(11) NOT NULL DEFAULT 0,
  `play_count_mania` int(11) NOT NULL DEFAULT 0,
  `replays_watched_std` int(11) NOT NULL DEFAULT 0,
  `replays_watched_taiko` int(11) NOT NULL DEFAULT 0,
  `replays_watched_ctb` int(11) NOT NULL DEFAULT 0,
  `replays_watched_mania` int(11) NOT NULL DEFAULT 0,
  `total_hits_std` int(11) NOT NULL DEFAULT 0,
  `total_hits_taiko` int(11) NOT NULL DEFAULT 0,
  `total_hits_ctb` int(11) NOT NULL DEFAULT 0,
  `total_hits_mania` int(11) NOT NULL DEFAULT 0,
  `max_combo_std` int(11) NOT NULL DEFAULT 0,
  `max_combo_taiko` int(11) NOT NULL DEFAULT 0,
  `max_combo_ctb` int(11) NOT NULL DEFAULT 0,
  `max_combo_mania` int(11) NOT NULL DEFAULT 0,
  `play_time_std` int(11) NOT NULL DEFAULT 0,
  `play_time_taiko` int(11) NOT NULL DEFAULT 0,
  `play_time_ctb` int(11) NOT NULL DEFAULT 0,
  `play_time_mania` int(11) NOT NULL DEFAULT 0,
  `avg_accuracy_std` float NOT NULL DEFAULT 0,
  `avg_accuracy_taiko` float NOT NULL DEFAULT 0,
  `avg_accuracy_ctb` float NOT NULL DEFAULT 0,
  `avg_accuracy_mania` float NOT NULL DEFAULT 0,
  `pp_std` int(11) NOT NULL DEFAULT 0,
  `pp_taiko` int(11) NOT NULL DEFAULT 0,
  `pp_ctb` int(11) NOT NULL DEFAULT 0,
  `pp_mania` int(11) NOT NULL DEFAULT 0,
  `count_A_std` int(11) NOT NULL DEFAULT 0,
  `count_S_std` int(11) NOT NULL DEFAULT 0,
  `count_X_std` int(11) NOT NULL DEFAULT 0,
  `count_SH_std` int(11) NOT NULL DEFAULT 0,
  `count_XH_std` int(11) NOT NULL DEFAULT 0,
  `count_A_taiko` int(11) NOT NULL DEFAULT 0,
  `count_S_taiko` int(11) NOT NULL DEFAULT 0,
  `count_X_taiko` int(11) NOT NULL DEFAULT 0,
  `count_SH_taiko` int(11) NOT NULL DEFAULT 0,
  `count_XH_taiko` int(11) NOT NULL DEFAULT 0,
  `count_A_ctb` int(11) NOT NULL DEFAULT 0,
  `count_S_ctb` int(11) NOT NULL DEFAULT 0,
  `count_X_ctb` int(11) NOT NULL DEFAULT 0,
  `count_SH_ctb` int(11) NOT NULL DEFAULT 0,
  `count_XH_ctb` int(11) NOT NULL DEFAULT 0,
  `count_A_mania` int(11) NOT NULL DEFAULT 0,
  `count_S_mania` int(11) NOT NULL DEFAULT 0,
  `count_X_mania` int(11) NOT NULL DEFAULT 0,
  `count_SH_mania` int(11) NOT NULL DEFAULT 0,
  `count_XH_mania` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1026 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

DROP TABLE IF EXISTS `users_stats_relax`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_stats_relax` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `rank_std` int(11) NOT NULL DEFAULT 0,
  `rank_taiko` int(11) NOT NULL DEFAULT 0,
  `rank_ctb` int(11) NOT NULL DEFAULT 0,
  `rank_mania` int(11) NOT NULL DEFAULT 0,
  `ranked_score_std` bigint(21) DEFAULT 0,
  `ranked_score_taiko` bigint(21) DEFAULT 0,
  `ranked_score_ctb` bigint(21) DEFAULT 0,
  `ranked_score_mania` bigint(21) DEFAULT 0,
  `total_score_std` bigint(21) DEFAULT 0,
  `total_score_taiko` bigint(21) DEFAULT 0,
  `total_score_ctb` bigint(21) DEFAULT 0,
  `total_score_mania` bigint(21) DEFAULT 0,
  `play_count_std` int(11) NOT NULL DEFAULT 0,
  `play_count_taiko` int(11) NOT NULL DEFAULT 0,
  `play_count_ctb` int(11) NOT NULL DEFAULT 0,
  `play_count_mania` int(11) NOT NULL DEFAULT 0,
  `replays_watched_std` int(11) NOT NULL DEFAULT 0,
  `replays_watched_taiko` int(11) NOT NULL DEFAULT 0,
  `replays_watched_ctb` int(11) NOT NULL DEFAULT 0,
  `replays_watched_mania` int(11) NOT NULL DEFAULT 0,
  `total_hits_std` int(11) NOT NULL DEFAULT 0,
  `total_hits_taiko` int(11) NOT NULL DEFAULT 0,
  `total_hits_ctb` int(11) NOT NULL DEFAULT 0,
  `total_hits_mania` int(11) NOT NULL DEFAULT 0,
  `max_combo_std` int(11) NOT NULL DEFAULT 0,
  `max_combo_taiko` int(11) NOT NULL DEFAULT 0,
  `max_combo_ctb` int(11) NOT NULL DEFAULT 0,
  `max_combo_mania` int(11) NOT NULL DEFAULT 0,
  `play_time_std` int(11) NOT NULL DEFAULT 0,
  `play_time_taiko` int(11) NOT NULL DEFAULT 0,
  `play_time_ctb` int(11) NOT NULL DEFAULT 0,
  `play_time_mania` int(11) NOT NULL DEFAULT 0,
  `avg_accuracy_std` float NOT NULL DEFAULT 0,
  `avg_accuracy_taiko` float NOT NULL DEFAULT 0,
  `avg_accuracy_ctb` float NOT NULL DEFAULT 0,
  `avg_accuracy_mania` float NOT NULL DEFAULT 0,
  `pp_std` int(11) NOT NULL DEFAULT 0,
  `pp_taiko` int(11) NOT NULL DEFAULT 0,
  `pp_ctb` int(11) NOT NULL DEFAULT 0,
  `pp_mania` int(11) NOT NULL DEFAULT 0,
  `count_A_std` int(11) NOT NULL DEFAULT 0,
  `count_S_std` int(11) NOT NULL DEFAULT 0,
  `count_X_std` int(11) NOT NULL DEFAULT 0,
  `count_SH_std` int(11) NOT NULL DEFAULT 0,
  `count_XH_std` int(11) NOT NULL DEFAULT 0,
  `count_A_taiko` int(11) NOT NULL DEFAULT 0,
  `count_S_taiko` int(11) NOT NULL DEFAULT 0,
  `count_X_taiko` int(11) NOT NULL DEFAULT 0,
  `count_SH_taiko` int(11) NOT NULL DEFAULT 0,
  `count_XH_taiko` int(11) NOT NULL DEFAULT 0,
  `count_A_ctb` int(11) NOT NULL DEFAULT 0,
  `count_S_ctb` int(11) NOT NULL DEFAULT 0,
  `count_X_ctb` int(11) NOT NULL DEFAULT 0,
  `count_SH_ctb` int(11) NOT NULL DEFAULT 0,
  `count_XH_ctb` int(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2022-04-18 13:04:40

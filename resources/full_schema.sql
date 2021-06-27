-- MariaDB dump 10.19  Distrib 10.5.9-MariaDB, for Win64 (AMD64)
--
-- Host: localhost    Database: himitsu
-- ------------------------------------------------------
-- Server version	8.0.22

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
-- Table structure for table `2fa`
--

DROP TABLE IF EXISTS `2fa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `2fa` (
  `userid` int NOT NULL,
  `ip` int NOT NULL,
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `2fa_totp`
--

DROP TABLE IF EXISTS `2fa_totp`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `2fa_totp` (
  `enabled` tinyint NOT NULL DEFAULT '0',
  `userid` int NOT NULL,
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `achievements`
--

DROP TABLE IF EXISTS `achievements`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `achievements` (
  `id` int NOT NULL,
  `name` mediumtext NOT NULL,
  `description` mediumtext NOT NULL,
  `icon` mediumtext NOT NULL,
  `version` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `anticheat_reports`
--

DROP TABLE IF EXISTS `anticheat_reports`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `anticheat_reports` (
  `id` int NOT NULL AUTO_INCREMENT,
  `anticheat_id` int NOT NULL,
  `score_id` int NOT NULL,
  `severity` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bancho_messages`
--

DROP TABLE IF EXISTS `bancho_messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_messages` (
  `id` int NOT NULL AUTO_INCREMENT,
  `msg_from_userid` int NOT NULL,
  `msg_from_username` varchar(30) NOT NULL,
  `msg_to` varchar(32) NOT NULL,
  `msg` varchar(127) NOT NULL,
  `time` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bancho_private_messages`
--

DROP TABLE IF EXISTS `bancho_private_messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_private_messages` (
  `id` int NOT NULL AUTO_INCREMENT,
  `msg_from_userid` int NOT NULL,
  `msg_from_username` varchar(30) NOT NULL,
  `msg_to` varchar(32) NOT NULL,
  `msg` varchar(127) NOT NULL,
  `time` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bancho_settings`
--

DROP TABLE IF EXISTS `bancho_settings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_settings` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `value_int` int NOT NULL DEFAULT '0',
  `value_string` varchar(512) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `bancho_tokens`
--

DROP TABLE IF EXISTS `bancho_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `bancho_tokens` (
  `id` int NOT NULL AUTO_INCREMENT,
  `token` varchar(16) NOT NULL,
  `osu_id` int NOT NULL,
  `latest_message_id` int NOT NULL,
  `latest_private_message_id` int NOT NULL,
  `latest_packet_time` int NOT NULL,
  `latest_heavy_packet_time` int NOT NULL,
  `joined_channels` varchar(512) NOT NULL,
  `game_mode` tinyint NOT NULL,
  `action` int NOT NULL,
  `action_text` varchar(128) NOT NULL,
  `kicked` tinyint NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `beatmaps`
--

DROP TABLE IF EXISTS `beatmaps`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `beatmaps` (
  `id` int NOT NULL AUTO_INCREMENT,
  `beatmap_id` int NOT NULL DEFAULT '0',
  `beatmapset_id` int NOT NULL DEFAULT '0',
  `beatmap_md5` varchar(32) NOT NULL DEFAULT '',
  `artist` text NOT NULL,
  `title` text NOT NULL,
  `difficulty_name` longtext NOT NULL,
  `creator` text NOT NULL,
  `cs` float DEFAULT NULL,
  `ar` float NOT NULL DEFAULT '0',
  `od` float NOT NULL DEFAULT '0',
  `hp` float DEFAULT NULL,
  `mode` int NOT NULL DEFAULT '0',
  `rating` int NOT NULL DEFAULT '10',
  `difficulty_std` float NOT NULL DEFAULT '0',
  `difficulty_taiko` float NOT NULL DEFAULT '0',
  `difficulty_ctb` float NOT NULL DEFAULT '0',
  `difficulty_mania` float NOT NULL DEFAULT '0',
  `max_combo` int NOT NULL DEFAULT '0',
  `hit_length` int NOT NULL DEFAULT '0',
  `bpm` bigint NOT NULL DEFAULT '0',
  `count_normal` int NOT NULL DEFAULT '0',
  `count_slider` int NOT NULL DEFAULT '0',
  `count_spinner` int NOT NULL DEFAULT '0',
  `play_count` int NOT NULL DEFAULT '0',
  `pass_count` int NOT NULL DEFAULT '0',
  `ranked_status` tinyint NOT NULL DEFAULT '0',
  `latest_update` int NOT NULL DEFAULT '0',
  `ranked_status_freezed` tinyint NOT NULL DEFAULT '0',
  `creating_date` bigint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `id` (`id`),
  KEY `id_2` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=134 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `beatmaps_names`
--

DROP TABLE IF EXISTS `beatmaps_names`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `beatmaps_names` (
  `id` int NOT NULL AUTO_INCREMENT,
  `beatmap_md5` varchar(32) NOT NULL DEFAULT '',
  `beatmap_name` varchar(256) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `beatmaps_rating`
--

DROP TABLE IF EXISTS `beatmaps_rating`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `beatmaps_rating` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `beatmap_md5` varchar(32) NOT NULL,
  `rating` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `channels`
--

DROP TABLE IF EXISTS `channels`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `channels` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `description` varchar(64) NOT NULL,
  `auto_join` tinyint(1) NOT NULL,
  `hidden` tinyint(1) NOT NULL,
  `read_only` tinyint(1) NOT NULL,
  `permission` bigint unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `clans`
--

DROP TABLE IF EXISTS `clans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clans` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `description` text NOT NULL,
  `icon` text NOT NULL,
  `tag` varchar(6) NOT NULL,
  `mlimit` int NOT NULL DEFAULT '16',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `clans_invites`
--

DROP TABLE IF EXISTS `clans_invites`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `clans_invites` (
  `id` int NOT NULL AUTO_INCREMENT,
  `clan` int NOT NULL,
  `invite` varchar(8) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `comments`
--

DROP TABLE IF EXISTS `comments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `comments` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `beatmap_id` int NOT NULL DEFAULT '0',
  `beatmapset_id` int NOT NULL DEFAULT '0',
  `score_id` int NOT NULL DEFAULT '0',
  `mode` int NOT NULL,
  `comment` varchar(128) NOT NULL,
  `time` int NOT NULL,
  `who` varchar(11) NOT NULL,
  `special_format` varchar(2556) DEFAULT 'FFFFFF',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `docs`
--

DROP TABLE IF EXISTS `docs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `docs` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `doc_name` varchar(255) NOT NULL DEFAULT 'New Documentation File',
  `doc_contents` longtext NOT NULL,
  `public` tinyint unsigned NOT NULL DEFAULT '0',
  `old_name` varchar(200) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `hw_user`
--

DROP TABLE IF EXISTS `hw_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `hw_user` (
  `id` int NOT NULL AUTO_INCREMENT,
  `userid` int NOT NULL,
  `mac` varchar(32) NOT NULL,
  `unique_id` varchar(32) NOT NULL,
  `disk_id` varchar(32) NOT NULL,
  `occurencies` int NOT NULL DEFAULT '0',
  `activated` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `identity_tokens`
--

DROP TABLE IF EXISTS `identity_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `identity_tokens` (
  `userid` int NOT NULL,
  `token` varchar(64) NOT NULL,
  UNIQUE KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ip_user`
--

DROP TABLE IF EXISTS `ip_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ip_user` (
  `userid` int NOT NULL,
  `ip` mediumtext NOT NULL,
  `occurencies` int NOT NULL,
  PRIMARY KEY (`userid`),
  UNIQUE KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `irc_tokens`
--

DROP TABLE IF EXISTS `irc_tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `irc_tokens` (
  `userid` int NOT NULL DEFAULT '0',
  `token` varchar(32) NOT NULL DEFAULT '',
  UNIQUE KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `main_menu_icons`
--

DROP TABLE IF EXISTS `main_menu_icons`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `main_menu_icons` (
  `id` int NOT NULL AUTO_INCREMENT,
  `is_current` int NOT NULL,
  `file_id` varchar(128) NOT NULL,
  `name` varchar(256) NOT NULL,
  `url` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `osin_access`
--

DROP TABLE IF EXISTS `osin_access`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `osin_access` (
  `scope` int NOT NULL DEFAULT '0',
  `created_at` int NOT NULL DEFAULT '0',
  `client` int NOT NULL DEFAULT '0',
  `extra` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `osin_client`
--

DROP TABLE IF EXISTS `osin_client`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `osin_client` (
  `id` int NOT NULL AUTO_INCREMENT,
  `secret` varchar(64) NOT NULL DEFAULT '',
  `extra` varchar(127) NOT NULL DEFAULT '',
  `redirect_uri` varchar(127) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `osin_client_user`
--

DROP TABLE IF EXISTS `osin_client_user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `osin_client_user` (
  `client_id` int NOT NULL DEFAULT '0',
  `user` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `password_recovery`
--

DROP TABLE IF EXISTS `password_recovery`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `password_recovery` (
  `id` int NOT NULL AUTO_INCREMENT,
  `k` varchar(80) NOT NULL,
  `u` varchar(30) NOT NULL,
  `t` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `privileges_groups`
--

DROP TABLE IF EXISTS `privileges_groups`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `privileges_groups` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `privileges` int NOT NULL,
  `color` varchar(32) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `profile_backgrounds`
--

DROP TABLE IF EXISTS `profile_backgrounds`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `profile_backgrounds` (
  `uid` int NOT NULL,
  `time` int NOT NULL,
  `type` int NOT NULL,
  `value` mediumtext NOT NULL,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `punishments`
--

DROP TABLE IF EXISTS `punishments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `punishments` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `origin_id` int NOT NULL,
  `type` tinyint unsigned NOT NULL,
  `time` int NOT NULL,
  `duration` int DEFAULT NULL,
  `active` tinyint(1) NOT NULL,
  `reason` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `rank_requests`
--

DROP TABLE IF EXISTS `rank_requests`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `rank_requests` (
  `id` int NOT NULL AUTO_INCREMENT,
  `userid` int NOT NULL,
  `bid` int NOT NULL,
  `type` varchar(8) NOT NULL,
  `time` int NOT NULL,
  `blacklisted` tinyint NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `bid` (`bid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ranking_logs`
--

DROP TABLE IF EXISTS `ranking_logs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ranking_logs` (
  `id` int NOT NULL AUTO_INCREMENT,
  `map_id` int DEFAULT NULL,
  `modified_by` int DEFAULT NULL,
  `ranked` tinyint DEFAULT NULL,
  `map_type` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `relationships`
--

DROP TABLE IF EXISTS `relationships`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `relationships` (
  `origin` int NOT NULL,
  `target` int NOT NULL,
  `blocked` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `reports`
--

DROP TABLE IF EXISTS `reports`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `reports` (
  `id` int NOT NULL AUTO_INCREMENT,
  `from_uid` int NOT NULL,
  `to_uid` int NOT NULL,
  `reason` text NOT NULL,
  `chatlog` text NOT NULL,
  `time` int NOT NULL,
  `assigned` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `roles`
--

DROP TABLE IF EXISTS `roles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `roles` (
  `id` int unsigned NOT NULL,
  `name` varchar(32) NOT NULL,
  `permissions` bigint unsigned NOT NULL,
  `color` tinyint unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `scores`
--

DROP TABLE IF EXISTS `scores`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `scores` (
  `id` int NOT NULL AUTO_INCREMENT,
  `hash` varchar(35) NOT NULL,
  `beatmap_md5` varchar(35) NOT NULL DEFAULT '',
  `user_id` int NOT NULL,
  `ranking` varchar(2) NOT NULL,
  `score` bigint NOT NULL DEFAULT '0',
  `max_combo` int NOT NULL DEFAULT '0',
  `full_combo` tinyint NOT NULL DEFAULT '0',
  `mods` int NOT NULL DEFAULT '0',
  `count_300` int NOT NULL,
  `count_100` int NOT NULL,
  `count_50` int NOT NULL,
  `count_katus` int NOT NULL,
  `count_gekis` int NOT NULL,
  `count_misses` int NOT NULL,
  `time` bigint NOT NULL,
  `play_mode` tinyint NOT NULL DEFAULT '0',
  `completed` tinyint NOT NULL DEFAULT '3',
  `accuracy` float NOT NULL DEFAULT '0',
  `pp` double NOT NULL DEFAULT '0',
  `play_time` int NOT NULL DEFAULT '0',
  `is_relax` tinyint NOT NULL DEFAULT '0',
  `times_watched` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `scores_first`
--

DROP TABLE IF EXISTS `scores_first`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `scores_first` (
  `score_id` int NOT NULL,
  `beatmap_md5` varchar(35) NOT NULL,
  `user_id` int NOT NULL,
  `play_mode` int NOT NULL,
  `is_relax` tinyint NOT NULL DEFAULT '0',
  UNIQUE KEY `scoreid_UNIQUE` (`score_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `system_settings`
--

DROP TABLE IF EXISTS `system_settings`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `system_settings` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `value_int` int NOT NULL DEFAULT '0',
  `value_string` varchar(512) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `tokens`
--

DROP TABLE IF EXISTS `tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tokens` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user` varchar(31) NOT NULL,
  `privileges` int NOT NULL,
  `token` varchar(127) NOT NULL,
  `private` tinyint DEFAULT '0',
  `last_updated` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=24 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `user_badges`
--

DROP TABLE IF EXISTS `user_badges`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_badges` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user` int NOT NULL,
  `badge` mediumtext NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `user_clans`
--

DROP TABLE IF EXISTS `user_clans`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `user_clans` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user` int NOT NULL,
  `clan` int NOT NULL,
  `perms` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users` (
  `id` int NOT NULL AUTO_INCREMENT,
  `username` varchar(32) NOT NULL,
  `safe_username` varchar(30) NOT NULL,
  `country` varchar(2) NOT NULL DEFAULT 'XX',
  `password_md5` varchar(128) NOT NULL,
  `salt` varchar(64) NOT NULL,
  `email` varchar(254) NOT NULL,
  `ip` varchar(39) NOT NULL,
  `registration_date` int NOT NULL,
  `latest_activity` int NOT NULL DEFAULT '0',
  `followers` int NOT NULL DEFAULT '0',
  `roles` int unsigned NOT NULL DEFAULT '0',
  `userpage` longtext,
  `background` longtext,
  `status` mediumtext,
  `notes` mediumtext,
  `last_session` varchar(1024) NOT NULL DEFAULT 'check',
  `is_public` tinyint NOT NULL DEFAULT '1',
  `is_relax` tinyint NOT NULL DEFAULT '0',
  `favourite_mode` tinyint NOT NULL DEFAULT '0',
  `favourite_relax` tinyint NOT NULL DEFAULT '0',
  `play_style` smallint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1017 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `users_beatmap_playcount`
--

DROP TABLE IF EXISTS `users_beatmap_playcount`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_beatmap_playcount` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `beatmap_id` int DEFAULT NULL,
  `game_mode` int DEFAULT NULL,
  `playcount` int DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `playcount_index` (`user_id`,`beatmap_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `users_preferences`
--

DROP TABLE IF EXISTS `users_preferences`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_preferences` (
  `id` int NOT NULL,
  `scoreboard_display_classic` tinyint DEFAULT '0',
  `scoreboard_display_relax` tinyint DEFAULT '0',
  `auto_last_classic` tinyint DEFAULT '0',
  `auto_last_relax` tinyint DEFAULT '0',
  `score_overwrite_std` tinyint DEFAULT '0',
  `score_overwrite_taiko` tinyint DEFAULT '0',
  `score_overwrite_ctb` tinyint DEFAULT '0',
  `score_overwrite_mania` tinyint DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `users_stats`
--

DROP TABLE IF EXISTS `users_stats`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_stats` (
  `id` int NOT NULL AUTO_INCREMENT,
  `rank_std` int NOT NULL DEFAULT '0',
  `rank_taiko` int NOT NULL DEFAULT '0',
  `rank_ctb` int NOT NULL DEFAULT '0',
  `rank_mania` int NOT NULL DEFAULT '0',
  `ranked_score_std` bigint NOT NULL DEFAULT '0',
  `ranked_score_taiko` bigint NOT NULL DEFAULT '0',
  `ranked_score_ctb` bigint NOT NULL DEFAULT '0',
  `ranked_score_mania` bigint NOT NULL DEFAULT '0',
  `total_score_std` bigint NOT NULL DEFAULT '0',
  `total_score_taiko` bigint NOT NULL DEFAULT '0',
  `total_score_ctb` bigint NOT NULL DEFAULT '0',
  `total_score_mania` bigint NOT NULL DEFAULT '0',
  `play_count_std` int NOT NULL DEFAULT '0',
  `play_count_taiko` int NOT NULL DEFAULT '0',
  `play_count_ctb` int NOT NULL DEFAULT '0',
  `play_count_mania` int NOT NULL DEFAULT '0',
  `replays_watched_std` int unsigned NOT NULL DEFAULT '0',
  `replays_watched_taiko` int NOT NULL DEFAULT '0',
  `replays_watched_ctb` int NOT NULL DEFAULT '0',
  `replays_watched_mania` int unsigned NOT NULL DEFAULT '0',
  `total_hits_std` int NOT NULL DEFAULT '0',
  `total_hits_taiko` int NOT NULL DEFAULT '0',
  `total_hits_ctb` int NOT NULL DEFAULT '0',
  `total_hits_mania` int NOT NULL DEFAULT '0',
  `max_combo_std` int NOT NULL DEFAULT '0',
  `max_combo_taiko` int NOT NULL DEFAULT '0',
  `max_combo_ctb` int NOT NULL DEFAULT '0',
  `max_combo_mania` int NOT NULL DEFAULT '0',
  `play_time_std` int NOT NULL DEFAULT '0',
  `play_time_taiko` int NOT NULL DEFAULT '0',
  `play_time_ctb` int NOT NULL DEFAULT '0',
  `play_time_mania` int NOT NULL DEFAULT '0',
  `avg_accuracy_std` float NOT NULL DEFAULT '0',
  `avg_accuracy_taiko` float NOT NULL DEFAULT '0',
  `avg_accuracy_ctb` float NOT NULL DEFAULT '0',
  `avg_accuracy_mania` float NOT NULL DEFAULT '0',
  `pp_std` int NOT NULL DEFAULT '0',
  `pp_taiko` int NOT NULL DEFAULT '0',
  `pp_ctb` int NOT NULL DEFAULT '0',
  `pp_mania` int NOT NULL DEFAULT '0',
  `count_A_std` int NOT NULL DEFAULT '0',
  `count_S_std` int NOT NULL DEFAULT '0',
  `count_X_std` int NOT NULL DEFAULT '0',
  `count_SH_std` int NOT NULL DEFAULT '0',
  `count_XH_std` int NOT NULL DEFAULT '0',
  `count_A_taiko` int NOT NULL DEFAULT '0',
  `count_S_taiko` int NOT NULL DEFAULT '0',
  `count_X_taiko` int NOT NULL DEFAULT '0',
  `count_SH_taiko` int NOT NULL DEFAULT '0',
  `count_XH_taiko` int NOT NULL DEFAULT '0',
  `count_A_ctb` int NOT NULL DEFAULT '0',
  `count_S_ctb` int NOT NULL DEFAULT '0',
  `count_X_ctb` int NOT NULL DEFAULT '0',
  `count_SH_ctb` int NOT NULL DEFAULT '0',
  `count_XH_ctb` int NOT NULL DEFAULT '0',
  `count_A_mania` int NOT NULL DEFAULT '0',
  `count_S_mania` int NOT NULL DEFAULT '0',
  `count_X_mania` int NOT NULL DEFAULT '0',
  `count_SH_mania` int NOT NULL DEFAULT '0',
  `count_XH_mania` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1017 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `users_stats_relax`
--

DROP TABLE IF EXISTS `users_stats_relax`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users_stats_relax` (
  `id` int NOT NULL AUTO_INCREMENT,
  `rank_std` int NOT NULL DEFAULT '0',
  `rank_taiko` int NOT NULL DEFAULT '0',
  `rank_ctb` int NOT NULL DEFAULT '0',
  `rank_mania` int NOT NULL DEFAULT '0',
  `ranked_score_std` bigint DEFAULT '0',
  `ranked_score_taiko` bigint DEFAULT '0',
  `ranked_score_ctb` bigint DEFAULT '0',
  `ranked_score_mania` bigint DEFAULT '0',
  `total_score_std` bigint DEFAULT '0',
  `total_score_taiko` bigint DEFAULT '0',
  `total_score_ctb` bigint DEFAULT '0',
  `total_score_mania` bigint DEFAULT '0',
  `play_count_std` int NOT NULL DEFAULT '0',
  `play_count_taiko` int NOT NULL DEFAULT '0',
  `play_count_ctb` int NOT NULL DEFAULT '0',
  `play_count_mania` int NOT NULL DEFAULT '0',
  `replays_watched_std` int unsigned NOT NULL DEFAULT '0',
  `replays_watched_taiko` int NOT NULL DEFAULT '0',
  `replays_watched_ctb` int NOT NULL DEFAULT '0',
  `replays_watched_mania` int unsigned NOT NULL DEFAULT '0',
  `total_hits_std` int NOT NULL DEFAULT '0',
  `total_hits_taiko` int NOT NULL DEFAULT '0',
  `total_hits_ctb` int NOT NULL DEFAULT '0',
  `total_hits_mania` int NOT NULL DEFAULT '0',
  `max_combo_std` int NOT NULL DEFAULT '0',
  `max_combo_taiko` int NOT NULL DEFAULT '0',
  `max_combo_ctb` int NOT NULL DEFAULT '0',
  `max_combo_mania` int NOT NULL DEFAULT '0',
  `play_time_std` int NOT NULL DEFAULT '0',
  `play_time_taiko` int NOT NULL DEFAULT '0',
  `play_time_ctb` int NOT NULL DEFAULT '0',
  `play_time_mania` int NOT NULL DEFAULT '0',
  `avg_accuracy_std` float NOT NULL DEFAULT '0',
  `avg_accuracy_taiko` float NOT NULL DEFAULT '0',
  `avg_accuracy_ctb` float NOT NULL DEFAULT '0',
  `avg_accuracy_mania` float NOT NULL DEFAULT '0',
  `pp_std` int NOT NULL DEFAULT '0',
  `pp_taiko` int NOT NULL DEFAULT '0',
  `pp_ctb` int NOT NULL DEFAULT '0',
  `pp_mania` int NOT NULL DEFAULT '0',
  `count_A_std` int NOT NULL DEFAULT '0',
  `count_S_std` int NOT NULL DEFAULT '0',
  `count_X_std` int NOT NULL DEFAULT '0',
  `count_SH_std` int NOT NULL DEFAULT '0',
  `count_XH_std` int NOT NULL DEFAULT '0',
  `count_A_taiko` int NOT NULL DEFAULT '0',
  `count_S_taiko` int NOT NULL DEFAULT '0',
  `count_X_taiko` int NOT NULL DEFAULT '0',
  `count_SH_taiko` int NOT NULL DEFAULT '0',
  `count_XH_taiko` int NOT NULL DEFAULT '0',
  `count_A_ctb` int NOT NULL DEFAULT '0',
  `count_S_ctb` int NOT NULL DEFAULT '0',
  `count_X_ctb` int NOT NULL DEFAULT '0',
  `count_SH_ctb` int NOT NULL DEFAULT '0',
  `count_XH_ctb` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1017 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2021-05-24 23:55:02

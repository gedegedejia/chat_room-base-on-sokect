SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";
CREATE DATABASE IF NOT EXISTS `chat_room` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;
USE `chat_room`;
CREATE TABLE `account` (
  `uid` int(11) NOT NULL,
  `name` varchar(10) NOT NULL,
  `is_vip` int(11) NOT NULL,
  `is_online` int(11) NOT NULL,
  `password` varchar(33) NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8;
CREATE TABLE `friends` (
  `uid` int(11) NOT NULL,
  `fuid` int(11) NOT NULL,
  `is_follow` int(11) NOT NULL,
  `state` int(11) NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8;
CREATE TABLE `groups` (
  `gid` int(11) NOT NULL,
  `name` varchar(20) NOT NULL,
  `owner` int(11) NOT NULL COMMENT '群主uid',
  `num` int(11) NOT NULL COMMENT '群成员数量'
) ENGINE = InnoDB DEFAULT CHARSET = utf8;
CREATE TABLE `group_member` (
  `gid` int(11) NOT NULL,
  `uid` int(11) NOT NULL,
  `permission` int(11) NOT NULL,
  `noname` int(11) NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8;
CREATE TABLE `group_rec` (
  `gid` int(11) NOT NULL,
  `uid` int(11) NOT NULL,
  `msg` varchar(300) NOT NULL,
  `offlist` varchar(20) NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8;
CREATE TABLE `private_rec` (
  `from_uid` int(11) NOT NULL,
  `to_uid` int(11) NOT NULL,
  `msg` varchar(300) NOT NULL,
  `is_offline` int(11) NOT NULL
) ENGINE = InnoDB DEFAULT CHARSET = utf8;

CREATE TABLE `user_notes` (
  `uid` int(11) NOT NULL,         -- 备注创建者的用户ID
  `target_uid` int(11) NOT NULL,  -- 被备注的用户ID
  `note` varchar(100) NOT NULL,   -- 备注内容
  PRIMARY KEY (`uid`, `target_uid`),  -- 确保每个用户对每个目标用户的备注唯一
  FOREIGN KEY (`uid`) REFERENCES `account` (`uid`) ON DELETE CASCADE,
  FOREIGN KEY (`target_uid`) REFERENCES `account` (`uid`) ON DELETE CASCADE
) ENGINE = InnoDB DEFAULT CHARSET = utf8;

ALTER TABLE `account`
ADD PRIMARY KEY (`uid`);
ALTER TABLE `friends`
ADD PRIMARY KEY (`uid`, `fuid`);
ALTER TABLE `groups`
ADD PRIMARY KEY (`gid`);
ALTER TABLE `group_member`
ADD PRIMARY KEY (`gid`, `uid`);
ALTER TABLE `account`
MODIFY `uid` int(11) NOT NULL AUTO_INCREMENT,
  AUTO_INCREMENT = 3;
ALTER TABLE `groups`
MODIFY `gid` int(11) NOT NULL AUTO_INCREMENT,
  AUTO_INCREMENT = 2;
COMMIT;

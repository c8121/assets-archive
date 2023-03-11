/*
 Archive meta data database DDL
*/

#CREATE DATABASE assetsarchive;
#USE assetsarchive;

/* 
 Example how to create a user:
 CREATE USER 'archive'@'localhost' IDENTIFIED BY '<insert-password-here>';
 GRANT ALL PRIVILEGES ON assetsarchive.* to 'archive'@'localhost';
*/

CREATE TABLE `HASH`
(
    `ID`   int(11) NOT NULL AUTO_INCREMENT,
    `HASH` varchar(254) DEFAULT NULL,
    PRIMARY KEY (`ID`)
) ENGINE MyISAM
  DEFAULT CHARSET = utf8mb4;

CREATE TABLE `PERSON`
(
    `ID`   int(11) NOT NULL AUTO_INCREMENT,
    `NAME` varchar(254) DEFAULT NULL,
    PRIMARY KEY (`ID`)
) ENGINE MyISAM
  DEFAULT CHARSET = utf8mb4;

CREATE TABLE `CATEGORY`
(
    `ID`     int(11) NOT NULL AUTO_INCREMENT,
    `PARENT` int(11)      DEFAULT NULL,
    `NAME`   varchar(254) DEFAULT NULL,
    PRIMARY KEY (`ID`)
) ENGINE MyISAM
  DEFAULT CHARSET = utf8mb4;

CREATE TABLE `ORIGIN`
(
    `ID`       int(11) NOT NULL AUTO_INCREMENT,
    `HASH`     int(11)      DEFAULT NULL,
    `NAME`     varchar(254) DEFAULT NULL,
    `OWNER`    int(11)      DEFAULT NULL,
    `CATEGORY` int(11)      DEFAULT NULL,
    `CREATED`  datetime     DEFAULT NULL,
    `CHANGED`  datetime     DEFAULT NULL,
    PRIMARY KEY (`ID`)
) ENGINE MyISAM
  DEFAULT CHARSET = utf8mb4;

CREATE TABLE `ORIGIN_PARTICIPANT`
(
    `ID`     int(11) NOT NULL AUTO_INCREMENT,
    `ORIGIN` int(11) DEFAULT NULL,
    `PERSON` int(11) DEFAULT NULL,
    PRIMARY KEY (`ID`)
) ENGINE MyISAM
  DEFAULT CHARSET = utf8mb4;


CREATE TABLE `TAG`
(
    `ID`  int(11) NOT NULL AUTO_INCREMENT,
    `TAG` varchar(254) DEFAULT NULL,
    PRIMARY KEY (`ID`)
) ENGINE MyISAM
  DEFAULT CHARSET = utf8mb4;

CREATE TABLE `ORIGIN_TAG`
(
    `ID`     int(11) NOT NULL AUTO_INCREMENT,
    `ORIGIN` int(11) DEFAULT NULL,
    `TAG`    int(11) DEFAULT NULL,
    PRIMARY KEY (`ID`)
) ENGINE MyISAM
  DEFAULT CHARSET = utf8mb4;
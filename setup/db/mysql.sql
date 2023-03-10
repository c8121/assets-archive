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
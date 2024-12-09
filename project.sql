-- MySQL dump 10.13  Distrib 8.0.40, for Linux (x86_64)
--
-- Host: localhost    Database: sw_pj
-- ------------------------------------------------------
-- Server version	8.0.40-0ubuntu0.24.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `comments`
--

DROP TABLE IF EXISTS `comments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `comments` (
  `comment_id` int NOT NULL AUTO_INCREMENT,
  `sender_uid` int DEFAULT NULL,
  `comment_text` varchar(200) DEFAULT NULL,
  `comment_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `video_id` int DEFAULT NULL,
  PRIMARY KEY (`comment_id`)
) ENGINE=InnoDB AUTO_INCREMENT=37 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `comments`
--

LOCK TABLES `comments` WRITE;
/*!40000 ALTER TABLE `comments` DISABLE KEYS */;
INSERT INTO `comments` VALUES (36,4,'好耶','2024-12-09 13:52:42',52);
/*!40000 ALTER TABLE `comments` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `tag`
--

DROP TABLE IF EXISTS `tag`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `tag` (
  `tag_id` int NOT NULL AUTO_INCREMENT,
  `tag_name` varchar(15) NOT NULL,
  `tag_count` int DEFAULT '0',
  PRIMARY KEY (`tag_id`),
  UNIQUE KEY `tag_name` (`tag_name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `tag`
--

LOCK TABLES `tag` WRITE;
/*!40000 ALTER TABLE `tag` DISABLE KEYS */;
INSERT INTO `tag` VALUES (1,'非常好看',0),(2,'比博燃',0),(3,'快乐风男',0);
/*!40000 ALTER TABLE `tag` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `type`
--

DROP TABLE IF EXISTS `type`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `type` (
  `type_id` int NOT NULL AUTO_INCREMENT,
  `type_name` varchar(30) NOT NULL,
  `type_count` int DEFAULT '0',
  PRIMARY KEY (`type_id`),
  UNIQUE KEY `type_name` (`type_name`),
  UNIQUE KEY `type_name_2` (`type_name`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `type`
--

LOCK TABLES `type` WRITE;
/*!40000 ALTER TABLE `type` DISABLE KEYS */;
INSERT INTO `type` VALUES (1,'生活',0),(2,'搞笑',0),(3,'动漫',13),(4,'新闻',0),(5,'音乐',0),(6,'教育',0),(7,'科技',0);
/*!40000 ALTER TABLE `type` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user` (
  `uid` int NOT NULL AUTO_INCREMENT,
  `name` varchar(15) DEFAULT '用户',
  `username` varchar(15) NOT NULL,
  `password` varchar(15) NOT NULL,
  `user_head_portrait_path` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`uid`),
  UNIQUE KEY `username` (`username`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user`
--

LOCK TABLES `user` WRITE;
/*!40000 ALTER TABLE `user` DISABLE KEYS */;
INSERT INTO `user` VALUES (4,'kexisorry','root','root','/image/user_head/4b78c7e331ad1c3117375c91bc4055b.jpg'),(6,'黄浩','1','1','/image/user_head/0d09378f269d8a292244b5ffcfc12e8f_t.gif'),(7,'kexisorry','hh','hh','/image/user_head/0d28639f12deb46b21c7f54e1174e35.jpg');
/*!40000 ALTER TABLE `user` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_video_relation`
--

DROP TABLE IF EXISTS `user_video_relation`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user_video_relation` (
  `id` int NOT NULL AUTO_INCREMENT,
  `uid` int NOT NULL,
  `video_id` int NOT NULL,
  `is_liked` tinyint(1) DEFAULT '0',
  `dislike` tinyint(1) DEFAULT '0',
  `favorite` tinyint(1) DEFAULT '0',
  `grade` decimal(2,1) DEFAULT '0.0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=31 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_video_relation`
--

LOCK TABLES `user_video_relation` WRITE;
/*!40000 ALTER TABLE `user_video_relation` DISABLE KEYS */;
INSERT INTO `user_video_relation` VALUES (23,4,41,1,0,1,0.0),(24,4,42,1,0,1,0.0),(25,4,53,1,0,1,0.0),(26,4,48,1,0,1,0.0),(27,4,44,0,0,1,0.0),(28,4,45,0,0,0,0.0),(29,4,51,0,0,0,0.0),(30,4,52,0,0,0,0.0);
/*!40000 ALTER TABLE `user_video_relation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `video`
--

DROP TABLE IF EXISTS `video`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `video` (
  `video_id` int NOT NULL AUTO_INCREMENT,
  `uid` int NOT NULL,
  `video_name` varchar(20) NOT NULL,
  `video_info` text,
  `likes` int DEFAULT '0',
  `dislikes` int DEFAULT '0',
  `favorites` int DEFAULT '0',
  `views` int DEFAULT '0',
  `heat` bigint DEFAULT '0',
  `grade` decimal(2,1) DEFAULT '0.0',
  `comments` int DEFAULT '0',
  `release_time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `video_type_id` int DEFAULT '0',
  `video_path` varchar(100) NOT NULL,
  `image_path` varchar(100) NOT NULL,
  `duration` varchar(8) NOT NULL DEFAULT '0:0:0',
  PRIMARY KEY (`video_id`)
) ENGINE=InnoDB AUTO_INCREMENT=54 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `video`
--

LOCK TABLES `video` WRITE;
/*!40000 ALTER TABLE `video` DISABLE KEYS */;
INSERT INTO `video` VALUES (41,4,'青猪1','青春猪头系列第1集',1,0,1,29,61,0.0,0,'2024-12-09 12:09:47',3,'/video/青猪1[SumiSora][Ao_Buta][01][GB][720p].mp4','/image/video_img/青猪1青猪1.jpg','0:0:0'),(42,4,'青猪2','青春猪头系列第2集',1,0,1,7,17,0.0,0,'2024-12-09 12:12:24',3,'/video/青猪2[SumiSora][Ao_Buta][02][GB][720p].mp4','/image/video_img/青猪2青猪1.jpg','0:0:0'),(43,4,'青猪3','青春猪头系列第3集',0,0,0,0,0,0.0,0,'2024-12-09 12:15:09',3,'/video/青猪3[SumiSora][Ao_Buta][03][GB][720p].mp4','/image/video_img/青猪3青猪1.jpg','0:0:0'),(44,4,'青猪4','青春猪头系列第4集',0,0,1,4,10,0.0,0,'2024-12-09 12:16:10',3,'/video/青猪4[SumiSora][Ao_Buta][04][GB][720p].mp4','/image/video_img/青猪4青猪1.jpg','0:0:0'),(45,4,'青猪5','青春猪头系列第5集',0,0,0,1,0,0.0,0,'2024-12-09 12:16:33',3,'/video/青猪5[SumiSora][Ao_Buta][05][GB][720p].mp4','/image/video_img/青猪5青猪1.jpg','0:0:0'),(46,4,'青猪6','青春猪头系列第6集',0,0,0,0,0,0.0,0,'2024-12-09 12:16:52',3,'/video/青猪6[SumiSora][Ao_Buta][06][GB][720p].mp4','/image/video_img/青猪6青猪1.jpg','0:0:0'),(47,4,'青猪7','青春猪头系列第7集',0,0,0,0,0,0.0,0,'2024-12-09 12:17:08',3,'/video/青猪7[SumiSora][Ao_Buta][07][GB][720p].mp4','/image/video_img/青猪7青猪1.jpg','0:0:0'),(48,4,'青猪8','青春猪头系列第8集',1,0,1,9,21,0.0,0,'2024-12-09 12:17:25',3,'/video/青猪8[SumiSora][Ao_Buta][08][GB][720p].mp4','/image/video_img/青猪8青猪1.jpg','0:0:0'),(49,4,'青猪9','青春猪头系列第9集',0,0,0,0,0,0.0,0,'2024-12-09 12:17:46',3,'/video/青猪9[SumiSora][Ao_Buta][09][GB][720p].mp4','/image/video_img/青猪9青猪1.jpg','0:0:0'),(50,4,'青猪10','青春猪头系列第10集',0,0,0,0,0,0.0,0,'2024-12-09 12:18:56',3,'/video/青猪10[SumiSora][Ao_Buta][10][GB][720p].mp4','/image/video_img/青猪10青猪1.jpg','0:0:0'),(51,4,'青猪11','青春猪头系列第11集',0,0,0,1,0,0.0,0,'2024-12-09 12:19:16',3,'/video/青猪11[SumiSora][Ao_Buta][11][GB][720p].mp4','/image/video_img/青猪11青猪1.jpg','0:0:0'),(52,4,'青猪12','青春猪头系列第12集',0,0,0,2,2,0.0,0,'2024-12-09 12:19:32',3,'/video/青猪12[SumiSora][Ao_Buta][12][GB][720p].mp4','/image/video_img/青猪12青猪1.jpg','0:0:0'),(53,4,'青猪13','青春猪头系列第13集',1,0,1,6,0,0.0,0,'2024-12-09 12:19:46',3,'/video/青猪13[SumiSora][Ao_Buta][13][GB][720p].mp4','/image/video_img/青猪13青猪1.jpg','0:0:0');
/*!40000 ALTER TABLE `video` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `video_of_tag`
--

DROP TABLE IF EXISTS `video_of_tag`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `video_of_tag` (
  `id` int NOT NULL AUTO_INCREMENT,
  `video_id` int NOT NULL,
  `tag_id` int NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `video_of_tag`
--

LOCK TABLES `video_of_tag` WRITE;
/*!40000 ALTER TABLE `video_of_tag` DISABLE KEYS */;
INSERT INTO `video_of_tag` VALUES (1,5,1),(2,5,2),(3,17,1);
/*!40000 ALTER TABLE `video_of_tag` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2024-12-09 21:56:54

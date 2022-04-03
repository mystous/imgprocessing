-- MySQL dump 10.13  Distrib 8.0.28, for Win64 (x86_64)
--
-- Host: localhost    Database: lunit
-- ------------------------------------------------------
-- Server version	8.0.28-0ubuntu0.20.04.3

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `imageMeta`
--

DROP TABLE IF EXISTS `imageMeta`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `imageMeta` (
  `imageId` varchar(20) NOT NULL,
  `desc` varchar(45) DEFAULT NULL,
  `uploadedTime` char(19) DEFAULT '0000-00-00T00:00:00',
  `patchedTime` char(19) DEFAULT '0000-00-00T00:00:00',
  `processedTime` char(19) DEFAULT '0000-00-00T00:00:00',
  `status` int NOT NULL DEFAULT '0',
  `uploadedLocation` varchar(2048) DEFAULT NULL,
  `maxXIndex` int NOT NULL DEFAULT '-1',
  `maxYIndex` int NOT NULL DEFAULT '-1',
  PRIMARY KEY (`imageId`),
  UNIQUE KEY `image_id_UNIQUE` (`imageId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `imageMeta`
--

LOCK TABLES `imageMeta` WRITE;
/*!40000 ALTER TABLE `imageMeta` DISABLE KEYS */;
INSERT INTO `imageMeta` VALUES ('1b390f03acbc','slide_thumbnail_6862x6616.jpg','2022-04-03 01:19:54','0000-00-00T00:00:00','0000-00-00T00:00:00',2,'http://121.160.75.246:8080/files/images/1b390f03acbc.jpg',27,26),('60cb6a3c8953','w.jpg','2022-04-03 11:29:21','0000-00-00T00:00:00','0000-00-00T00:00:00',2,'http://121.160.75.246:8080/files/images/60cb6a3c8953.jpg',5,4),('f7f5c298f3ca','story1-01.png','2022-04-03 11:29:53','0000-00-00T00:00:00','0000-00-00T00:00:00',2,'http://121.160.75.246:8080/files/images/f7f5c298f3ca.jpg',4,3);
/*!40000 ALTER TABLE `imageMeta` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2022-04-03 11:37:54

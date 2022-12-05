#!/bin/sh

sudo docker kill mysql
sudo docker rm mysql 

#sudo docker run --name=mysql -d mysql/mysql-server:latest
docker run \
--detach \
--name=mysql \
--env="MYSQL_DATABASE=IB" \
--env="MYSQL_USER=tombleron" \
--env="MYSQL_PASSWORD=test" \
--publish 6603:3306 \
--volume=/home/tombleron/Desktop/NSTU/IB/conf.d:/etc/mysql/conf.d \
mysql/mysql-server:latest

until [[ $(docker ps | grep healthy | wc -l) == 1 ]] ; do
	echo "Waiting..."
	sleep 5
done

echo "Db booted up"

sudo docker logs mysql 2>&1 | grep "GENERATED ROOT PASSWORD"

FROM mariadb:latest

# Set environment variables for MariaDB configuration
ENV MYSQL_ROOT_PASSWORD="joda"
ENV MYSQL_USER="user"
ENV MYSQL_PASSWORD="password"
ENV MYSQL_DATABASE="mydb"

COPY ./start.sh /
COPY ./init.sql /


# Declare the volume for the MariaDB data directory
VOLUME /var/lib/mysql

# Expose the default MariaDB port
EXPOSE 3306

RUN chmod +x /start.sh

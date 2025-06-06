CREATE DATABASE IF NOT EXISTS exordium;
CREATE USER IF NOT EXISTS 'exordium'@'%' IDENTIFIED BY 'exordium';
GRANT ALL PRIVILEGES ON exordium.* TO 'exordium'@'%';
FLUSH PRIVILEGES;

CREATE DATABASE IF NOT EXISTS exordium;
USE exordium;
CREATE TABLE IF NOT EXISTS users (
    id uuid DEFAULT UUID() PRIMARY KEY,
    username VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL,
    hmac_key BINARY(32),
    chacha_key BINARY(32),
    chacha_nonce BINARY(8),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    confirmed BOOLEAN DEFAULT FALSE
);
INSERT INTO users (username, password, email) VALUES ('admin', 'admin', 'admin');

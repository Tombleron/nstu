# Output the date and time the script was run.
select "Output from script, run at: " as 'Script Information',
    NOW() as 'Date and Time Executed';

#######################################################
# Create table: Supply
#######################################################

CREATE TABLE Supply (
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	vendor_id CHAR(32) NOT NULL,
	part_id CHAR(32) NOT NULL,
	product_id CHAR(32) NOT NULL,
	count INT UNSIGNED NOT NULL,
	PRIMARY KEY (id)
);

# Add some item types to table
INSERT INTO Supply(vendor_id, part_id, product_id, count)
	VALUES
	('S1','P1','J1',200),
	('S1','P1','J4',700),
	('S2','P3','J1',400),
	('S2','P3','J2',200),
	('S2','P3','J3',200),
	('S2','P3','J4',500),
	('S2','P3','J5',600),
	('S2','P3','J6',400),
	('S2','P3','J7',800),
	('S2','P5','J2',100),
	('S3','P3','J1',200),
	('S3','P4','J2',500),
	('S4','P6','J3',300),
	('S4','P6','J7',300),
	('S5','P2','J2',200),
	('S5','P2','J4',100),
	('S5','P5','J5',500),
	('S5','P5','J7',100),
	('S5','P6','J2',200),
	('S5','P1','J4',100),
	('S5','P3','J4',200),
	('S5','P4','J4',800),
	('S5','P5','J4',400),
	('S5','P6','J4',500)
;


SELECT * FROM Supply;

#######################################################
# Create table: Parts
#######################################################

CREATE TABLE Parts (
	part_id CHAR(32) NOT NULL,
	part_name CHAR(32) NOT NULL,
	color CHAR(32) NOT NULL,
	weight INT UNSIGNED NOT NULL,
	city CHAR(32) NOT NULL,
	PRIMARY KEY (part_id)
);

# Add some item types to table
INSERT INTO Parts(part_id, part_name, color, weight, city)
	VALUES
	('P1','Гайка','Красный',12,'Лондон'),
	('P2','Болт','Зеленый',17,'Париж'),
	('P3','Винт','Голубой',17,'Рим'),
	('P4','Винт','Красный',14,'Лондон'),
	('P5','Кулачок','Голубой',12,'Париж'),
	('P6','Блюм','Красный',19,'Лондон')
;


SELECT * FROM Parts;

#######################################################
# Create table: Products
#######################################################

CREATE TABLE Products (
	product_id CHAR(32) NOT NULL,
	product_name CHAR(32) NOT NULL,
	city CHAR(32) NOT NULL,
	PRIMARY KEY (product_id)
);

# Add some item types to table
INSERT INTO Products(product_id, product_name, city)
	VALUES
	('J1','Жесткий диск','Париж'),
	('J2','Перфоратор','Рим'),
	('J3','Считыватель','Афины'),
	('J4','Принтер','Афины'),
	('J5','Флоппи-диск','Лондон'),
	('J6','Терминал','Осло'),
	('J7','Лента','Лондон')
;


SELECT * FROM Products;

#######################################################
# Create table: Vendors
#######################################################

CREATE TABLE Vendors (
	vendor_id CHAR(32) NOT NULL,
	surname CHAR(32) NOT NULL,
	rating INT UNSIGNED NOT NULL,
	city CHAR(32) NOT NULL,
	PRIMARY KEY (vendor_id)
);

# Add some item types to table
INSERT INTO Vendors(vendor_id, surname, rating, city)
	VALUES
	('S1','Смит',20,'Лондон'),
	('S2','Джонс',10,'Париж'),
	('S3','Блейк',30,'Париж'),
	('S4','Кларк',20,'Лондон'),
	('S5','Адамс',30,'Афины')
;


SELECT * FROM Vendors;

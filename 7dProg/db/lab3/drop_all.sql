# Output the date and time the script was run.
select "Output from script, run at: " as 'Script Information',
    NOW() as 'Date and Time Executed';

#######################################################
# Drop table: Supply
#######################################################

DROP TABLE Supply;

#######################################################
# Drop table: Parts
#######################################################

DROP TABLE Parts;

#######################################################
# Drop table: Products
#######################################################

DROP TABLE Products;

#######################################################
# Drop table: Vendors 
#######################################################

DROP TABLE Vendors;

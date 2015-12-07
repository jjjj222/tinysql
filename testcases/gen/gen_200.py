#! /usr/bin/python
import sys
#CREATE TABLE example (attr0 INT, attr1 INT, attr2 INT, attr3 INT, attr4 INT)
#INSERT INTO example (attr0, attr1, attr2, attr3, attr4) VALUES (0, 0, 0, 0, 0)

table_name = "example";
num_of_tuples = 200;

#print "CREATE TABLE",  table_name, "(attr0 INT, attr1 INT, attr2 INT, attr3 INT)"
print "CREATE TABLE",  table_name, "(attr0 INT, attr1 INT, attr2 INT, attr3 INT, attr4 INT)"

for i in range(1, num_of_tuples + 1):
    #print "INSERT INTO", table_name, "(attr0, attr1, attr2, attr3) VALUES (", i, ",", i, ",", i, ",", i, ")"
    print "INSERT INTO", table_name, "(attr0, attr1, attr2, attr3, attr4) VALUES (", i, ",", i, ",", i, ",", i, ",", i, ")"
    #print "INSERT INTO", table_name, "(attr0, attr1, attr2, attr3) VALUES (", i, ",", i, ",", i, ",", i, ")"
    #print i;
    #sys.stdout.write(str(i))

print "SELECT * FROM", table_name
print "SELECT * FROM", table_name, "WHERE attr0=0"
print "SELECT * FROM", table_name, "ORDER BY attr0"
print "SELECT DISTINCT * FROM", table_name

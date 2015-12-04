#CREATE TABLE example (attr0 INT, attr1 STR20)
#INSERT INTO example (attr0, attr1) VALUES (0, "A")
#INSERT INTO example (attr0, attr1) VALUES (0, "A")
#INSERT INTO example (attr0, attr1) VALUES (2, "C")
#INSERT INTO example (attr1, attr0) VALUES ("B", 1)
#SELECT DISTINCT * FROM example
#CREATE TABLE example2 (attr0 INT, attr1 STR20)
#INSERT INTO example2 (attr0, attr1) VALUES (1, "A")
#INSERT INTO example2 (attr0, attr1) VALUES (3, "D")
#INSERT INTO example2 (attr0, attr1) VALUES (3, "A")
#SELECT example.attr0, example.attr1, example2.attr0 FROM example, example2 WHERE example.attr0 < example2.attr0 ORDER BY example.attr0
#source testcases/all_tmp.in
#source testcases/example.in.tmp
#source testcases/example3.in.tmp
#source testcases/report
#source testcases/debug.in.tmp
#source testcases/all_4.in
#source testcases/example2.in.tmp
#source testcases/example2.in
#CREATE TABLE course (sid INT, homework INT, project INT, exam INT, grade STR20)
CREATE TABLE example (attr0 INT, attr1 INT, attr2 INT, attr3 INT, attr4 INT)
INSERT INTO example (attr0, attr1, attr2, attr3, attr4) VALUES (0, 0, 0, 0, 0)
SELECT * FROM example ORDER BY attr0
show tables
dump time
#dump relation course
#dump memory
#dump relation course
#dump relation course2

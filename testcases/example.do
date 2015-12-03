#CREATE TABLE example (attr0 INT, attr1 STR20)
#INSERT INTO example (attr0, attr1) VALUES (0, "A")
#INSERT INTO example (attr1, attr2) VALUES ("B", 1)
#source testcases/all_tmp.in
#source testcases/example.in.tmp
source testcases/example3.in.tmp
#source testcases/all_4.in
#source testcases/example2.in.tmp
#source testcases/example2.in
#CREATE TABLE course (sid INT, homework INT, project INT, exam INT, grade STR20)
show tables
dump time
#dump relation course
#dump memory
#dump relation course
#dump relation course2

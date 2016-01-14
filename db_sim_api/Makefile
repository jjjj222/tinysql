all: TestStorageManager

TestStorageManager: StorageManager.o 
	g++ -std=c++11 -g -o TestStorageManager StorageManager.o TestStorageManager.cpp

StorageManager.o: Block.h Disk.h Field.h MainMemory.h Relation.h Schema.h SchemaManager.h Tuple.h Config.h
	g++ -std=c++11 -g -c StorageManager.cpp

clean:
	rm -f *.o TestStorageManager

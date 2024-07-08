all: upload/starter/with-bonus/bin/lib_simpleloader.so

upload/starter/with-bonus/bin/lib_simpleloader.so: upload/starter/with-bonus/loader/loader.c
	gcc -m32 -shared -o "$@" "$<"

clean:
	-rm -f C:/Programming/OS\ GitHub/Group-5.zip/2022071-089_OS_Assignment_1/ASSIGNMENT-1-MODIFIVE/upload/starter/with-bonus/bin/lib_simpleloader.so

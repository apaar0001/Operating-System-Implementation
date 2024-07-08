all: C:/Programming/OS GitHub/Group-5.zip/2022071-089_OS_Assignment_1/ASSIGNMENT-1-MODIFIVE/upload/starter/with-bonus/bin/launch

C:/Programming/OS GitHub/Group-5.zip/2022071-089_OS_Assignment_1/ASSIGNMENT-1-MODIFIVE/upload/starter/with-bonus/bin/launch: launch.c C:/Programming/OS GitHub/Group-5.zip/2022071-089_OS_Assignment_1/ASSIGNMENT-1-MODIFIVE/upload/starter/with-bonus/bin/lib_simpleloader.so
	gcc -m32 -o $@ $< -I../loader -L/C:/Programming/OS GitHub/Group-5.zip/2022071-089_OS_Assignment_1/ASSIGNMENT-1-MODIFIVE/upload/starter/with-bonus/bin -Wl,-rpath,'C:/Programming/OS GitHub/Group-5.zip/2022071-089_OS_Assignment_1/ASSIGNMENT-1-MODIFIVE/upload/starter/with-bonus/bin'lib_simpleloader.so

clean:
	-rm -f C:/Programming/OS GitHub/Group-5.zip/2022071-089_OS_Assignment_1/ASSIGNMENT-1-MODIFIVE/upload/starter/with-bonus/bin/launch


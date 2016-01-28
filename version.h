/*
 * version.h
 */

#ifndef VERSION_H_
#define VERSION_H_

#define MAJOR_VERSION_NUMBER 0

#define MINOR_VERSION_NUMBER 1

#define SUBVERSION_NUMBER 0

#define __STR(x) #x
#define __STR2(x) __STR(x)

#define VERSION_STRING (__STR2(MAJOR_VERSION_NUMBER) "." __STR2(MINOR_VERSION_NUMBER) "." __STR2(SUBVERSION_NUMBER))


#endif /* VERSION_H_ */

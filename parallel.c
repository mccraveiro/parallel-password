#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#if defined(__linux__)
#include <crypt.h>
#endif

#define HASH_SIZE 256

/*
 * @brief Generate a Hash from a password string
 * @param password password string
 * @param hash generated Hash
 */
char *generatePasswordHash(const char *password);

/*
 * @brief Set 'password' to the next value to be tested
 * @param password password string
 * @return -1 if there are no more possible values
 */
int getNextPassword(char *password);

/*
 * @brief compare password with target_hash
 * @param target_hash string
 * @param password string
 * @return 1 if equal, otherwise 0.
 */
int ComparePassword(const char *target_hash, const char *password);

/*
 * @brief number of chars used on password
 */
int passwordLength = 0;

/*
 * @brief string used to generate password
 */
char *password;

int main(int argc, char *argv[]) {
	int success = 0;

	// time spent running the program
	clock_t start = 0;
	clock_t finish = 0;

	// ensure the program got the right arguments
	if (argc < 3) {
		printf("Usage: %s <password length> <target hash>", argv[0]);
		return 1;
	}

	passwordLength = atoi(argv[1]);
	password = malloc(sizeof(char) * (passwordLength + 1));

  // set initial password to '0' chars
	memset(password, '0', passwordLength);
	password[passwordLength] = '\0';

	// start timer
	start = clock();

	// test all possile passwords
	while ((success = ComparePassword(argv[2], password)) == 0 && getNextPassword(password) != -1);

	// stop timer
	finish = clock();

	if (success) {
		printf("%f\n", (float)(finish - start) / CLOCKS_PER_SEC);
	}

	return 0;
}

int ComparePassword(const char *hashAlvo, const char *password) {
	// char generatedHash[HASH_SIZE + 1];
	// strcpy(generatedHash, generatePasswordHash(password));

	// compare generated hash with target hash
	if (!strcmp(hashAlvo, generatePasswordHash(password))) {
		return 1;
	}

	return 0;
}

int getNextPassword(char *password) {
  // Increment password chars from right to left Ex.: 0000, 0001, 0002 ... 0010...
	int index = passwordLength - 1;

	while (index >= 0) {
		if (password[index] != '9') {
			password[index]++;
			break;
		}

    password[index] = '0';
		index--;
	}

	return index;
}

char *generatePasswordHash(const char *password) {
	return crypt(password, "aa");
}

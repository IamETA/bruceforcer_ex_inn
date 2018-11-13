#define _GNU_SOURCE

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <crypt.h>
#include "include/crypto.h"
#include "include/functions.h"

extern char *strdup(const char *src);

//Bruceforce operations
void load_dictionary_item(const char *dictionaryfile, char ***dictionary, int *count);
char *password_for_hash(clyps *cdata,const char *salt, const char *hash, const char* password);
/* 
  Check password to hash function to see if the
  hashed result is equal to the input of <hash>

  returns NULL if the result does not match the hash
    or POINTER to the maching password if it
    successfully matches the input string.

  Improvements: We can probably optimize the string compare 
  function to increase the speed
*/
char *password_for_hash(clyps *cdata,const char *salt, const char *hash,const char* password) {
  char* hashcompare = crypt_r(password,salt,cdata);
  if (hashcompare == NULL) {
    printf("Critical error Cannot compute hash!!\n");
    printf("hash: %s, salt: %s, password: %s\n",hash,salt,password);
    exit(EXIT_FAILURE);
  }
  else if (strcmp(hashcompare,hash) == 0) return strdup(password);
  return NULL;
}
/* This is the function that will enumerate all
  possible characters in a given c_tablesize.
  Like smoking out hiding places, no characters can hide
  from this function :)

  This was the hardest function to make, and theres plenty of 
  optimizations to make
*/
char* bytesmoker(bruteforce_args *bargs,char* sz_word, int wordsize,int workingposition,clyps *cdata) {
  for (int i = 0;i<bargs->c_tablesize+1;i++) {
    //Go deeper if possible
    if (workingposition < wordsize && (sz_word[workingposition+1] != bargs->c_table[bargs->c_tablesize-1])) {
      char* password = bytesmoker(bargs,sz_word,wordsize,workingposition+1,cdata);
      if (password != NULL) return password;
      sz_word[workingposition+1] = bargs->c_table[0];
    }

    if (bargs->stop) return NULL;
    //Change the character
    sz_word[workingposition] = bargs->c_table[i];
  
    //Test the password
    char *s;
    if ((s = password_for_hash(cdata,bargs->salt,bargs->hash,sz_word)) != NULL) {
      free(s);
      return sz_word;
    }
    bargs->p_processed++; //Show number of processed words
  }
  return NULL;
}


char *bf_hack(bruteforce_args *args)
{
  // Crypto Init
  clyps *cdata = malloc(sizeof(clyps));
  cdata->initialized = 0;
  char *return_password;
  char *sz_word = malloc(sizeof(char*)); //Allocate buffer
  //Incease bitsize incrementally as we try passwords

  for (int wordsize=1+(args->startNum-2);wordsize<args->wordsize;wordsize++) {
    args->p_processed = 0;
    sz_word = realloc(sz_word,sizeof(char*) * (wordsize+1));
    for(int i=0;i<wordsize;i++)  {
      sz_word[i] = args->c_table[0];
    }
    sz_word[wordsize+1] = '\0';
    for (int segment = args->segment_from;segment<args->segment_from + args->segment_count;segment++) {
      //Start with the first character in c_table
      //"statically" assign first bit with segment (each thread will have different segments)
      sz_word[0] = args->c_table[segment]; //As multithread, we only process a segment of the total c_tablesize
      if (args->stop) {
        free(cdata);
	free(sz_word);
	return NULL;
      }
      sz_word[wordsize+1]='\0';
      //Process each column
      int charpos = 1;

      args->depth = wordsize;
      return_password = bytesmoker(args, sz_word, wordsize, charpos, cdata);
      //Remember to free up memory if needed here
     
      if(return_password!=NULL) {
	free(cdata);
	return return_password;
      }
    }
  }
  free(cdata);
  free(sz_word);
  return NULL;
}
//Dictionary
const char *bf_dictionary(char **dictionary, int startfrom, int count, const char *p_type_salt, const char *hashedvalue, float *p_status, bool *abort)
{
  // Crypto Init
  clyps *cdata = malloc(sizeof(clyps));
  //struct crypt_data cdata;
  cdata->initialized = 0;
  for (int i = startfrom; i < startfrom + count; i++)
  {
    if (*abort == true)
      break;

    char * s;
    if ((s = password_for_hash(cdata,p_type_salt,hashedvalue,dictionary[i])) != NULL) {
      free(cdata);
      free(s);
      return dictionary[i];
    }
    *p_status = ((float)(i - startfrom) / count) * 100;
  }
  free(cdata);
  return '\0';
}

void load_dictionary(const char *dictionarypath, char ***dictionary, int *count, int *dictfilecount)
{
  (*dictfilecount) = 0;
  DIR *dpdf;
  struct dirent *epdf;
  dpdf = opendir(dictionarypath);
  if (dpdf == NULL)
  {
    printf("Dictionaries folder not found: %s\n", dictionarypath);
    return;
  }
  while ((epdf = readdir(dpdf)))
  {
    if (epdf->d_name[0] != '.') {
      char *dictionaryfilepath = concat(dictionarypath, epdf->d_name);
      load_dictionary_item(dictionaryfilepath, dictionary, count);
      (*dictfilecount)++;
      free(dictionaryfilepath);
    }
  }
  closedir(dpdf);
}

void load_dictionary_item(const char *dictionaryfile, char ***dictionary, int *count)
{
  char **dictref = (*dictionary);
  FILE *fp = fopen(dictionaryfile, "r");
  //size_t len = 0;
  char *buffer = malloc(sizeof(char) * 100);

  if (fp == NULL)
    exit(EXIT_FAILURE);

  int position = 0;

  //fill buffer until we have a line
  char chard = fgetc(fp);
  int words = (*count);
  int wordsize = 0;
  while (chard != EOF)
  {
    //Add to buffer;
    if (chard == '\n' && wordsize > 0)
    {
      if (words % 1000 == 0) {
        dictref = realloc(dictref, sizeof(char *) * (words+1000)); //add a couple of extra to increase speed
      }
      //null terminate buffer
      buffer[wordsize] = '\0';
      //realoc size of dictionary
      if (dictref == NULL) exit(EXIT_FAILURE);
      dictref[words] = strdup(buffer);//strncpy(dictref[words], buffer, wordsize-1);
      words++;
      wordsize = 0;
    }
    else
    {
      buffer[wordsize] = chard;
      wordsize++;
    }
    position++;
    chard = fgetc(fp);
  }
  (*count) = words;
  (*dictionary) = dictref;
  free(buffer);
  fclose(fp);
}

//String operations
//Combine strings, in a somewhat sloppy way, but hey
char *concat(const char *s1, const char *s2)
{
  size_t stringsize = strlen(s1) + strlen(s2) + 1;
  char *result = malloc(stringsize); // +1 for the null-terminator
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}
